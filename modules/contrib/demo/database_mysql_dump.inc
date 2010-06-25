<?php
// $Id: database_mysql_dump.inc,v 1.5.2.11 2009/11/19 17:11:21 smk Exp $

// Some older mysql client libs are missing this constant.
if (!defined('MYSQLI_BINARY_FLAG')) {
  define('MYSQLI_BINARY_FLAG', 128);
}

/**
 * Dump active database.
 *
 * @param $filename
 *   The filename including path to write the dump to.
 * @param $options
 *   An associative array of snapshot options, as described in demo_dump().
 */
function demo_dump_db($filename, $options = array()) {
  // Make sure we have permission to save our backup file.
  $directory = dirname($filename);
  if (!file_check_directory($directory, FILE_CREATE_DIRECTORY)) {
    return FALSE;
  }

  if ($fp = fopen($filename, 'wb')) {
    $header = "-- Demo.module database dump (version " . DEMO_DUMP_VERSION . ")\n";
    $header .= "-- http://drupal.org/project/demo\n";
    $header .= "--\n";
    $header .= "-- Database: " . _demo_get_database() . "\n";
    $header .= "-- Date: " . format_date(time(), 'large') . "\n\n";
    // Avoid auto value for zero values (required for user id 0).
    $header .= "SET SQL_MODE=\"NO_AUTO_VALUE_ON_ZERO\";\n";
    // Temporarily disable foreign key checks for the time of import.
    $header .= "SET FOREIGN_KEY_CHECKS = 0;\n";
    fwrite($fp, $header);

    foreach ($options['tables'] as $table => $dump_options) {
      if (!_demo_table_is_view($table)) {
        if ($dump_options['schema']) {
          _demo_dump_table_schema($fp, $table);
        }
        if ($dump_options['data']) {
          _demo_dump_table_data($fp, $table);
        }
      }
    }

    // Re-enable foreign key checks.
    fwrite($fp, "\nSET FOREIGN_KEY_CHECKS = 1;\n");

    fclose($fp);
    return TRUE;
  }

  return FALSE;
}

/**
 * Returns the name of the active database.
 */
function _demo_get_database() {
  $database = array_keys(db_fetch_array(db_query('SHOW TABLES')));
  $database = preg_replace('/^Tables_in_/', '', $database[0]);
  return $database;
}

/**
 * Enumerate database tables.
 */
function _demo_enum_tables() {
  return db_query('SHOW TABLES');
}

/**
 * Dump table schema.
 *
 * @param $fp
 *   The file handle of the output file.
 * @param $table
 *   A table name to export the schema for.
 */
function _demo_dump_table_schema($fp, $table) {
  $output = "\n";
  $output .= "--\n";
  $output .= "-- Table structure for table '$table'\n";
  $output .= "--\n\n";

  $data = db_fetch_array(db_query("SHOW CREATE TABLE %s", $table));
  $output .= preg_replace('/^CREATE TABLE/', 'CREATE TABLE IF NOT EXISTS', $data['Create Table']);
  $status = db_fetch_array(db_query("SHOW TABLE STATUS LIKE '%s'", $table));
  // PDO is missing the table type.
  if (!strpos($output, ' ENGINE=')) {
    $output .= ' ENGINE=' . $status['engine'];
  }
  // @todo Wrong value; contains 'utf8_general_ci' (collation), not 'utf8' (charset).
  //$output .= ' DEFAULT CHARSET=' . $status['collation'];
  if (!empty($status['comment'])) {
    $output .= " COMMENT='" . $status['comment'] . "'";
  }
  // @todo Depends on whether we dump data and table existence on import.
//  if (!empty($status['auto_increment'])) {
//    $output .= ' AUTO_INCREMENT=' . $status['auto_increment'];
//  }
  $output .= ";\n";

  fwrite($fp, $output);
}

/**
 * Dump table data.
 *
 * This code has largely been stolen from the phpMyAdmin project.
 *
 * @param $fp
 *   The file handle of the output file.
 * @param $table
 *   A table name to export the data for.
 */
function _demo_dump_table_data($fp, $table) {
  $output = "\n";
  $output .= "--\n";
  $output .= "-- Dumping data for table '$table'\n";
  $output .= "--\n\n";

  // Dump table data.
  $result = db_query("SELECT * FROM %s", $table);

  // Get table fields.
  if ($fields = _demo_get_fields($result)) {
    // Disable indices to speed up import.
    $output .= "/*!40000 ALTER TABLE $table DISABLE KEYS */;\n";

    // Escape backslashes, PHP code, special chars
    $search = array('\\', "'", "\x00", "\x0a", "\x0d", "\x1a");
    $replace = array('\\\\', "''", '\0', '\n', '\r', '\Z');

    $insert_cmd = "INSERT INTO `$table` VALUES\n";
    $insert_buffer = '';
    $current_row = 0;
    $query_size = 0;

    while ($row = db_fetch_array($result)) {
      $current_row++;
      $values = array();
      $field = 0;
      foreach ($row as $value) {
        // NULL
        if (!isset($value) || is_null($value)) {
          $values[] = 'NULL';
        }
        // A number
        // timestamp is numeric on some MySQL 4.1, BLOBs are sometimes numeric
        else if ($fields[$field]->numeric && !$fields[$field]->timestamp && !$fields[$field]->blob) {
          $values[] = $value;
        }
        // A true BLOB
        // - mysqldump only generates hex data when the --hex-blob
        //   option is used, for fields having the binary attribute
        //   no hex is generated
        // - a TEXT field returns type blob but a real blob
        //   returns also the 'binary' flag
        else if ($fields[$field]->binary && $fields[$field]->blob) {
          // Empty blobs need to be different, but '0' is also empty :-(
          if (empty($value) && $value != '0') {
            $values[] = "''";
          }
          else {
            $values[] = '0x' . bin2hex($value);
          }
        }
        // Something else -> treat as a string
        else {
          $values[] = "'" . str_replace($search, $replace, $value) . "'";
        }
        $field++;
      }

      if ($current_row == 1) {
        $insert_buffer = $insert_cmd . '(' . implode(', ', $values) . ')';
      }
      else {
        $insert_buffer = '(' . implode(', ', $values) . ')';

        // Check if buffer size exceeds 50KB.
        if ($query_size + strlen($insert_buffer) > 50000) {
          // Flush to disc and start new buffer.
          fwrite($fp, $output . ";\n");
          $output = '';
          $current_row = 1;
          $query_size = 0;
          $insert_buffer = $insert_cmd . $insert_buffer;
        }
      }
      $query_size += strlen($insert_buffer);

      $output .= ($current_row == 1 ? '' : ",\n") . $insert_buffer;
    }

    if ($current_row > 0) {
      $output .= ";\n";
    }

    // Enable indices again.
    $output .= "/*!40000 ALTER TABLE $table ENABLE KEYS */;\n";
  }

  fwrite($fp, $output);
}

/**
 * Return table fields and their properties.
 */
function _demo_get_fields($result) {
  $fields = array();

  switch ($GLOBALS['db_type']) {
    case 'mysql':
      $num_fields = mysql_num_fields($result);
      for ($i = 0; $i < $num_fields; $i++) {
        $meta = mysql_fetch_field($result, $i);
        // Enhance field definition with custom properties.
        $meta->timestamp = (int)($meta->type == 'timestamp');
        $flags = mysql_field_flags($result, $i);
        $meta->binary = (int)(stristr($flags, 'binary') !== FALSE);
        $fields[] = $meta;
      }
      break;

    case 'mysqli':
      while ($meta = mysqli_fetch_field($result)) {
        // Enhance the field definition for mysql-extension compatibilty.
        $meta->numeric = (int)(bool)($meta->flags & MYSQLI_NUM_FLAG);
        $meta->blob = (int)(bool)($meta->flags & MYSQLI_BLOB_FLAG);
        // Add custom properties.
        $meta->timestamp = (int)($meta->type == MYSQLI_TYPE_TIMESTAMP);
        $meta->binary = (int)(bool)($meta->flags & MYSQLI_BINARY_FLAG);
        $fields[] = $meta;
      }
      break;
  }

  return $fields;
}

/**
 * Determine whether the given table is a VIEW.
 */
function _demo_table_is_view($table) {
  static $tables = array();
  if (!isset($tables[$table])) {
    $status = db_fetch_array(db_query("SHOW TABLE STATUS LIKE '%s'", $table));
    $tables[$table] = (strtoupper(substr($status['Comment'], 0, 4)) == 'VIEW');
  }
  return $tables[$table];
}

