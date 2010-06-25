<?php
// $Id: demo.admin.inc,v 1.1.2.19 2010/01/17 19:00:10 sun Exp $

/**
 * @file
 * Demonstration Site administrative pages
 */

/**
 * Current version of SQL dump structure.
 */
define('DEMO_DUMP_VERSION', '1.1');

/**
 * Form builder for Demo module settings.
 */
function demo_admin_settings() {
  global $base_url;

  $form['status'] = array(
    '#type' => 'fieldset',
    '#title' => t('Status'),
    '#collapsible' => FALSE,
  );
  if (variable_get('demo_reset_last', 0)) {
    $reset_date = format_date(variable_get('demo_reset_last', 0));
  }
  else {
    $reset_date = t('Never');
  }
  $form['status'][] = array(
    '#value' => t('<p><strong>Last reset:</strong> !date</p>', array('!date' => $reset_date)),
  );

  $fileconfig = demo_get_fileconfig();

  $form['dump'] = array(
    '#type' => 'fieldset',
    '#title' => t('Dump settings'),
  );


  $form['dump']['demo_dump_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Dump path'),
    '#default_value' => $fileconfig['path'],
    '#description' => t('Enter a writable directory where dump files of this demonstration site are stored, f.e. %files. The name of this site (e.g. %confpath) is automatically appended to this directory, if required.<br /><br /><strong>Note: For security reasons you should store site dumps outside of the document root of your webspace!</strong>', array('%files' => file_directory_path() . '/demo', '%confpath' => $fileconfig['site'])),
  );
  $form['#validate'][] = 'demo_admin_settings_validate';

  return system_settings_form($form);
}

/**
 * Form validation handler for demo_admin_settings().
 */
function demo_admin_settings_submit($form, &$form_state) {
  if (!file_check_directory($form_state['values']['demo_dump_path'], FILE_CREATE_DIRECTORY)) {
    form_set_error('demo_dump_path', t('The snapshot directory %directory could not be created.', array('%directory' => $form_state['values']['demo_dump_path'])));
  }
}

/**
 * Form builder to manage snapshots.
 */
function demo_manage_form() {
  $form['dump'] = array(
    '#type' => 'fieldset',
    '#title' => t('Available snapshots'),
  );
  $form['dump'] += demo_get_dumps();
  $form['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
    '#submit' => array('demo_manage_delete_submit'),
  );
  return $form;
}

/**
 * Delete button submit handler for demo_manage_form().
 */
function demo_manage_delete_submit($form, &$form_state) {
  $form_state['redirect'] = 'admin/build/demo/delete/' . $form_state['values']['filename'];
}

/**
 * Form builder to confirm deletion of a snapshot.
 */
function demo_delete_confirm(&$form_state, $filename) {
  $fileconfig = demo_get_fileconfig($filename);
  if (!file_exists($fileconfig['infofile'])) {
    return drupal_access_denied();
  }

  $form['filename'] = array(
    '#type' => 'value',
    '#value' => $filename,
  );
  return confirm_form($form, t('Are you sure you want to delete the snapshot %title?', array('%title' => $filename)), 'admin/build/demo/manage', t('This action cannot be undone.'), t('Delete'));
}

/**
 * Form submit handler for demo_delete_confirm().
 */
function demo_delete_confirm_submit($form, &$form_state) {
  $files = demo_get_fileconfig($form_state['values']['filename']);
  unlink($files['sqlfile']);
  unlink($files['infofile']);
  drupal_set_message(t('Snapshot %title has been deleted.', array('%title' => $form_state['values']['filename'])));
  $form_state['redirect'] = 'admin/build/demo/manage';
}

/**
 * Form builder to create a new snapshot.
 */
function demo_dump_form() {
  $form['dump']['filename'] = array(
    '#title' => t('File name'),
    '#type' => 'textfield',
    '#autocomplete_path' => 'demo/autocomplete',
    '#required' => TRUE,
    '#maxlength' => 128,
    '#description' => t('Enter the snapshot file name without file extension. Allowed characters are a-z, 0-9, dashes ("-"), underscores ("_") and dots.'),
  );
  $form['dump']['description'] = array(
    '#title' => t('Description'),
    '#type' => 'textarea',
    '#rows' => 2,
    '#description' => t('Optionally enter a description for this snapshot here. If no description is given and a snapshot with the same filename already exists, the previous description is used.'),
  );
  $form['dump']['tables'] = array('#type' => 'value', '#value' => demo_enum_tables());

  return confirm_form($form, t('Are you sure you want to create a new snapshot?'), 'admin/build/demo', t('If the above filename already exists, creating a new snapshot will overwrite the existing snapshot. This action cannot be undone.'), t('Create'), t('Cancel'));
}

/**
 * Form submit handler for demo_dump_form().
 */
function demo_dump_form_submit($form, &$form_state) {
  if ($fileconfig = _demo_dump($form_state['values'])) {
    drupal_set_message(t('Successfully created snapshot %filename.', array('%filename' => $fileconfig['sqlfile'])));
  }
  $form_state['redirect'] = 'admin/build/demo/manage';
}

/**
 * Create a new snapshot.
 *
 * @param $options
 *   A structured array of snapshot options:
 *   - filename: The base output filename, without extension.
 *   - default: Whether to set this dump as new default snapshot.
 *   - description: A description for the snapshot. If a snapshot with the same
 *     name already exists and this is left blank, the new snapshot will reuse
 *     the existing description.
 *   - tables: An array of tables to dump, keyed by table name (including table
 *     prefix, if any). The value is an array of dump options:
 *     - schema: Whether to dump the table schema.
 *     - data: Whether to dump the table data.
 */
function _demo_dump($options) {
  // Increase PHP's max_execution_time for large dumps.
  @set_time_limit(600);

  // Generate the info file.
  $info = demo_set_info($options);
  if (!$info) {
    return FALSE;
  }

  // Allow other modules to alter the dump options.
  $fileconfig = demo_get_fileconfig($info['filename']);
  drupal_alter('demo_dump', $options, $info, $fileconfig);

  // Perform database dump.
  if (!demo_dump_db($fileconfig['sqlfile'], $options)) {
    return FALSE;
  }

  // Adjust file permissions.
  if (function_exists('drupal_chmod')) {
    drupal_chmod($fileconfig['infofile']);
    drupal_chmod($fileconfig['sqlfile']);
  }
  else {
    chmod($fileconfig['infofile'], 0664);
    chmod($fileconfig['sqlfile'], 0664);
  }

  // Allow other modules to act on successful dumps.
  module_invoke_all('demo_dump', $options, $info, $fileconfig);

  return $fileconfig;
}

/**
 * Form builder to reset site to a snapshot.
 */
function demo_reset_confirm() {
  $form['dump'] = array(
    '#type' => 'fieldset',
    '#title' => t('Available snapshots'),
  );
  $form['dump'] += demo_get_dumps();

  return confirm_form($form, t('Are you sure you want to reset the site?'), 'admin/build/demo', t('Resetting the site will overwrite all changes that have been made to this Drupal installation since the chosen snapshot.<br /><br /><div style="color: red; font-weight: bold; font-size: 18px;"><center>THIS ACTION CANNOT BE UNDONE!</center><br /></div>'), t('Reset'), t('Cancel'));
}

/**
 * Form submit handler for demo_reset_confirm().
 */
function demo_reset_confirm_submit($form, &$form_state) {
  // Reset site to chosen snapshot.
  _demo_reset($form_state['values']['filename']);

  $form_state['redirect'] = 'admin/build/demo';
}

/**
 * Reset site using snapshot.
 *
 * @param $filename
 *   Base snapshot filename, without extension.
 * @param $verbose
 *   Whether to output status messages.
 */
function _demo_reset($filename, $verbose = TRUE) {
  // Load database specific functions.
  if (!demo_load_include()) {
    return FALSE;
  }

  // Increase PHP's max_execution_time for large dumps.
  @set_time_limit(600);

  $fileconfig = demo_get_fileconfig($filename);
  if (!file_exists($fileconfig['sqlfile']) || !($fp = fopen($fileconfig['sqlfile'], 'r'))) {
    if ($verbose) {
      drupal_set_message(t('Unable to open dump file %filename.', array('%filename' => $fileconfig['sqlfile'])), 'error');
    }
    watchdog('demo', 'Unable to open dump file %filename.', array('%filename' => $fileconfig['sqlfile']), WATCHDOG_ERROR);
    return FALSE;
  }

  // Load any database information in front of reset.
  $info = demo_get_info($fileconfig['infofile']);
  module_invoke_all('demo_reset_before', $filename, $info, $fileconfig);

  // Drop tables.
  $is_version_1_0_dump = version_compare($info['version'], '1.1', '<');
  $watchdog = db_prefix_tables('{watchdog}');
  foreach (demo_enum_tables() as $table => $dump_options) {
    // Skip watchdog, except for legacy dumps that included the watchdog table
    if ($table != $watchdog || $is_version_1_0_dump) {
      db_query("DROP TABLE $table");
    }
  }

  // Load data from snapshot.
  $success = TRUE;
  $query = '';
  $new_line = TRUE;

  while (!feof($fp)) {
    // Better performance on PHP 5.2.x when leaving out buffer size to
    // fgets().
    $data = fgets($fp);
    if ($data === FALSE) {
      break;
    }
    // Skip empty lines (including lines that start with a comment).
    if ($new_line && ($data == "\n" || !strncmp($data, '--', 2) || !strncmp($data, '#', 1))) {
      continue;
    }

    $query .= $data;
    $len = strlen($data);
    if ($data[$len - 1] == "\n") {
      if ($data[$len - 2] == ';') {
        // Reached the end of a query, now execute it.
        if (!_db_query($query, FALSE)) {
          $success = FALSE;
        }
        $query = '';
      }
      $new_line = TRUE;
    }
    else {
      // Continue adding data from the same line.
      $new_line = FALSE;
    }
  }
  fclose($fp);

  if ($success) {
    if ($verbose) {
      drupal_set_message(t('Successfully restored database from %filename.', array('%filename' => $fileconfig['sqlfile'])));
    }
    watchdog('demo', 'Successfully restored database from %filename.', array('%filename' => $fileconfig['sqlfile']), WATCHDOG_NOTICE);

    // Allow other modules to act on successful resets.
    module_invoke_all('demo_reset', $filename, $info, $fileconfig);
  }
  else {
    if ($verbose) {
      drupal_set_message(t('Failed restoring database from %filename.', array('%filename' => $fileconfig['sqlfile'])), 'error');
    }
    watchdog('demo', 'Failed restoring database from %filename.', array('%filename' => $fileconfig['sqlfile']), WATCHDOG_ERROR);
  }

  // Save request time of last reset, but not during re-installation via
  // demo_profile.
  if (!defined('MAINTENANCE_MODE') || MAINTENANCE_MODE !== 'install') {
    variable_set('demo_reset_last', $_SERVER['REQUEST_TIME']);
  }

  return $success;
}

function demo_get_fileconfig($filename = 'demo_site') {
  $fileconfig = array();

  // Build dump path.
  $fileconfig['path'] = variable_get('demo_dump_path', file_directory_path() . '/demo');
  $fileconfig['dumppath'] = $fileconfig['path'];
  // Append site name if it is not included in file_directory_path() and if not
  // storing files in sites/all/files.
  $fileconfig['site'] = str_replace('sites', '', conf_path());
  if (strpos($fileconfig['path'], conf_path()) === FALSE && strpos($fileconfig['path'], '/all/') === FALSE) {
    $fileconfig['dumppath'] .= $fileconfig['site'];
  }

  // Check if directory exists.
  file_check_directory($fileconfig['path'], FILE_CREATE_DIRECTORY, 'path');
  if (!file_check_directory($fileconfig['dumppath'], FILE_CREATE_DIRECTORY, 'path')) {
    return FALSE;
  }

  // Protect dump files.
  $htaccess = $fileconfig['path'] . "/.htaccess";
  if (!is_file($htaccess)) {
    $htaccess_lines = "# demo.module snapshots\n# Do not let the webserver serve anything under here!\n#\nDeny from all\n";
    if (($fp = fopen($htaccess, 'w')) && fputs($fp, $htaccess_lines)) {
      fclose($fp);
      if (function_exists('drupal_chmod')) {
        drupal_chmod($htaccess);
      }
      else {
        chmod($htaccess, 0664);
      }
    }
  }

  // Build SQL filename.
  $fileconfig['sql'] = $filename . '.sql';
  $fileconfig['sqlfile'] = $fileconfig['dumppath'] . '/' . $fileconfig['sql'];

  // Build info filename.
  $fileconfig['info'] = $filename . '.info';
  $fileconfig['infofile'] = $fileconfig['dumppath'] . '/' . $fileconfig['info'];

  return $fileconfig;
}

/**
 * Load database specific functions.
 */
function demo_load_include() {
  global $db_type;

  $engine = ($db_type == 'mysqli' ? 'mysql' : $db_type);
  if (module_load_include('inc', 'demo', 'database_' . $engine . '_dump') === FALSE) {
    drupal_set_message(t('@engine support not implemented yet.', array('@engine' => ucfirst($engine))), 'error');
    return FALSE;
  }
  return TRUE;
}

function demo_get_dumps() {
  $fileconfig = demo_get_fileconfig();

  // Fetch list of available info files
  $files = file_scan_directory($fileconfig['dumppath'], '.info$');

  foreach ($files as $file => $object) {
    $files[$file]->filemtime = filemtime($file);
    $files[$file]->filesize = filesize(substr($file, 0, -4) . 'sql');
  }

  // Sort snapshots by date (ascending file modification time)
  uasort($files, create_function('$a, $b', 'return ($a->filemtime < $b->filemtime);'));

  $options = array();
  // Forms API does not pass selected value of individual radio buttons,
  // so we manually insert an internal form value here.
  $options['filename'] = array(
    '#type' => 'value',
    '#required' => TRUE,
    '#title' => t('Snapshot'),
  );
  foreach ($files as $filename => $file) {
    // Build basic file info
    $files[$filename] = (array) $file;
    $info = demo_get_info($filename);

    // Convert file info for Forms API
    $option = array(
      '#type' => 'radio',
      '#name' => 'filename',
      '#title' => check_plain($info['filename']) . ' (' . format_date($file->filemtime, 'small') . ', ' . format_size($file->filesize) . ')',
      '#description' => '',
      '#return_value' => $info['filename'],
      '#file' => $file,
      '#info' => $info,
      '#attributes' => array('onclick' => "jQuery('.description', this.parentNode.parentNode).slideToggle();"),
    );
    if (!empty($info['description'])) {
      $option['#description'] .= '<p>' . $info['description'] . '</p>';
    }
    $targs = array(
      '@info-file-url' => url('demo/download/' . $file->name . '/info'),
      '@sql-file-url' => url('demo/download/' . $file->name . '/sql'),
    );
    $option['#description'] .= '<p>' . t('Download: <a href="@info-file-url">.info file</a>, <a href="@sql-file-url">.sql file</a>', $targs) . '</p>';
    if (count($info['modules']) > 1) {
      // Remove required core modules and obvious modules from module list.
      $info['modules'] = array_diff($info['modules'], array('block', 'filter', 'node', 'system', 'user', 'demo'));
      // Sort module list alphabetically.
      sort($info['modules']);
      $option['#description'] .= t('Modules: ') . implode(', ', $info['modules']);
    }

    $options[$info['filename']] = $option;
  }

  // Attach stylesheet to initially hide descriptions
  drupal_add_js("$('div.form-item div.description', $('form')).hide();", 'inline', 'footer');

  return $options;
}

function demo_get_info($filename, $field = NULL) {
  $info = array();

  if (file_exists($filename)) {
    $info = parse_ini_file($filename);

    if (isset($info['modules'])) {
      $info['modules'] = explode(" ", $info['modules']);
    }
    else {
      $info['modules'] = NULL;
    }

    if (!isset($info['version'])) {
      $info['version'] = '1.0';
    }
  }

  if (isset($field)) {
    return isset($info[$field]) ? $info[$field] : NULL;
  }
  else {
    return $info;
  }
}

function demo_set_info($values = NULL) {
  if (isset($values['filename']) && is_array($values)) {
    // Check for valid filename
    if (!preg_match('/^[-_\.a-zA-Z0-9]+$/', $values['filename'])) {
      drupal_set_message(t('Dump filename %title must contain alphanumeric characters, dots, dashes and underscores only. Other characters, including blanks (spaces), are not allowed.', array('%title' => $values['filename'])), 'error');
      return FALSE;
    }

    if (!empty($values['description'])) {
      // parse_ini_file() doesn't allow certain characters in description
      $s = array("\r\n", "\r", "\n", '"');
      $r = array(' ', ' ', ' ', "'");
      $values['description'] = str_replace($s, $r, $values['description']);
    }
    else {
      // If new description is empty, try to use previous description.
      $old_file = demo_get_fileconfig($values['filename']);
      $old_description = demo_get_info($old_file['infofile'], 'description');
      if (!empty($old_description)) {
        $values['description'] = $old_description;
      }
    }

    // Set values
    $infos = array();
    $infos['filename'] = $values['filename'];
    $infos['description'] = '"' . $values['description'] . '"';
    $infos['modules'] = implode(' ', module_list());
    $infos['version'] = DEMO_DUMP_VERSION;

    // Write information to .info file
    $fileconfig = demo_get_fileconfig($values['filename']);
    $infofile = fopen($fileconfig['infofile'], 'w');
    foreach ($infos as $key => $info) {
      fwrite($infofile, $key . ' = ' . $info . "\n");
    }
    fclose($infofile);

    return $infos;
  }
}

/**
 * Returns a list of tables in the active database.
 *
 * Only returns tables whose prefix matches the configured one (or ones, if
 * there are multiple).
 */
function demo_enum_tables() {
  global $db_prefix;

  $tables = array();

  // Load database specific functions.
  if (!demo_load_include()) {
    return FALSE;
  }

  // Create a regex that matches the table prefix(es).
  if (is_array($db_prefix)) {
    $rx = '/^' . implode('|', array_filter($db_prefix)) . '/';
  }
  else if ($db_prefix != '') {
    $rx = '/^' . $db_prefix . '/';
  }

  // Query the database engine for the table list.
  $result = _demo_enum_tables();
  while ($table = db_fetch_array($result)) {
    $table = reset($table);
    if (is_array($db_prefix)) {
      // Check if table name matches a configured prefix.
      if (preg_match($rx, $table, $matches)) {
        $table_prefix = $matches[0];
        $plain_table = substr($table, strlen($table_prefix));
        if ($db_prefix[$plain_table] == $table_prefix || $db_prefix['default'] == $table_prefix) {
          $tables[$table] = array('schema' => TRUE, 'data' => TRUE);
        }
      }
    }
    else if ($db_prefix != '') {
      if (preg_match($rx, $table)) {
        $tables[$table] = array('schema' => TRUE, 'data' => TRUE);
      }
    }
    else {
      $tables[$table] = array('schema' => TRUE, 'data' => TRUE);
    }
  }

  // Apply default exclude list.
  $excludes = array(
    // Core
    '{cache}',
    '{cache_block}',
    '{cache_content}',
    '{cache_filter}',
    '{cache_form}',
    '{cache_menu}',
    '{cache_page}',
    '{cache_update}',
    '{watchdog}',
    // CTools
    '{ctools_object_cache}',
    // Drupal Administration Menu
    '{cache_admin_menu}',
    // Panels
    '{panels_object_cache}',
    // Views
    '{cache_views}',
    '{cache_views_data}',
    '{views_object_cache}',
  );
  foreach (array_map('db_prefix_tables', $excludes) as $table) {
    if (isset($tables[$table])) {
      $tables[$table]['data'] = FALSE;
    }
  }

  return $tables;
}

/**
 * Retrieve a pipe delimited string of autocomplete suggestions for existing snapshots.
 */
function demo_autocomplete($string = '') {
  $matches = array();
  if ($string && $fileconfig = demo_get_fileconfig()) {
    $string = preg_quote($string);
    $files = file_scan_directory($fileconfig['dumppath'], $string . '.*\.info$');
    foreach ($files as $file) {
      $matches[$file->name] = check_plain($file->name);
    }
  }
  print drupal_to_js($matches);
  exit();
}

/**
 * Transfer (download) a snapshot file.
 *
 * @param $filename
 *   The snapshot filename to transfer.
 * @param $type
 *   The file type, i.e. extension to transfer.
 *
 * @todo Allow to download an bundled archive of snapshot files.
 */
function demo_download($filename, $type) {
  $fileconfig = demo_get_fileconfig($filename);
  if (!isset($fileconfig[$type . 'file']) || !file_exists($fileconfig[$type . 'file'])) {
    return MENU_NOT_FOUND;
  }
  // Force the client to re-download and trigger a file save download.
  $headers = array(
    'Cache-Control: private',
    'Content-Type: application/octet-stream',
    'Content-Length: ' . filesize($fileconfig[$type . 'file']),
    'Content-Disposition: attachment, filename=' . $fileconfig[$type],
  );
  file_transfer($fileconfig[$type . 'file'], $headers);
}

