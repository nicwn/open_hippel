<?php
// $Id: imce.inc,v 1.4 2010/03/02 20:01:27 quicksketch Exp $

/**
 * @file
 * A FileField extension to allow referencing of files from IMCE.
 *
 * The "hooks" in this file are not true hooks, they're called individually
 * from the main filefield_sources.module in the corresponding hook by the
 * same name. Any of these hooks could be broken out into a separate module.
 */

if ($_GET['q'] == 'imce' && isset($_GET['filefield']) && isset($_GET['type']) && content_fields($_GET['filefield'], $_GET['type'])) {
  $GLOBALS['conf']['imce_custom_scan'] = 'filefield_source_imce_custom_scan';
  $GLOBALS['conf']['imce_custom_process'] = array('filefield_source_imce_custom_process' => 1);
  $GLOBALS['conf']['imce_settings_absurls'] = FALSE;
}

/**
 * Implementation of hook_filefield_source_info().
 */
function filefield_source_imce_info() {
  $source = array();

  if (!module_exists('imce') || !imce_access()) {
    return $source;
  }

  $source['imce'] = array(
    'name' => t('IMCE file browser'),
    'label' => t('File browser'),
    'description' => t('Select a file to use from a file browser.'),
    'process' => 'filefield_source_imce_process',
    'value' => 'filefield_source_imce_value',
  );
  return $source;
}

/**
 * Implementation of hook_theme().
 */
function filefield_source_imce_theme() {
  return array(
    'filefield_source_imce_element' => array(
      'arguments' => array('element' => NULL),
      'file' => 'sources/imce.inc',
    ),
 );
}

/**
 * Implementation of hook_filefield_source_settings().
 */
function filefield_source_imce_settings($op, $field) {
  $return = array();

  // Add settings to the FileField widget form.

  return $return;

}

/**
 * A #process callback to extend the filefield_widget element type.
 */
function filefield_source_imce_process($element, $edit, &$form_state, $form) {
  $field = content_fields($element['#field_name'], $element['#type_name']);

  $element['filefield_imce'] = array(
    '#weight' => 100.3,
    '#access' => empty($element['fid']['#value']),
    '#theme' => 'filefield_source_imce_element',
    '#description' => filefield_sources_element_validation_help($element),
  );

  $filepath_id = $element['#id'] . '-imce-path';
  $display_id = $element['#id'] . '-imce-display';
  $select_id = $element['#id'] . '-imce-select';
  $element['filefield_imce']['file_path'] = array(
    '#type' => 'hidden',
    '#value' => '',
    '#id' => $filepath_id,
    '#attributes' => array(
      'onchange' => "if (!jQuery('#$select_id').attr('disabled')) { jQuery('#$select_id').mousedown().disable(); jQuery('#$display_id').html(this.value); }",
    ),
  );

  $imce_function = 'window.open(\''. url('imce', array('query' => 'filefield='. $element['#field_name'] .'&type='. $element['#type_name'] .'&app='. rawurlencode($field['widget']['label']) .'|url@'. $filepath_id)) .'\', \'\', \'width=760,height=560,resizable=1\'); return false;';
  $element['filefield_imce']['display_path'] = array(
    '#type' => 'markup',
    '#value' => '<span id="'. $display_id .'" class="filefield-sources-imce-display">' . t('No file selected') .'</span> (<a class="filefield-sources-imce-browse" href="#" onclick="'. $imce_function .'">' . t('browse') . '</a>)',
  );

  $element['filefield_imce']['select'] = array(
    '#type' => 'submit',
    '#value' => t('Select'),
    '#submit' => array('node_form_submit_build_node'),
    '#name' => $element['#name'] . '[filefield_imce][button]',
    '#id' => $select_id,
    '#attributes' => array('style' => 'display: none;'),
    '#ahah' => array(
       'path' => 'filefield/ahah/'. $element['#type_name'] .'/'. $element['#field_name'] .'/'. $element['#delta'],
       'wrapper' => $element['#id'] .'-ahah-wrapper',
       'method' => 'replace',
       'effect' => 'fade',
    ),
  );

  return $element;
}

/**
 * A #filefield_value_callback function.
 */
function filefield_source_imce_value($element, &$item) {
  if (isset($item['filefield_imce']['file_path']) && $item['filefield_imce']['file_path'] != '') {
    if (variable_get('file_downloads', FILE_DOWNLOADS_PUBLIC) == FILE_DOWNLOADS_PUBLIC) {
      $file_path = preg_replace('/^' . preg_quote(base_path(), '/') . '/', '', $item['filefield_imce']['file_path']);
    }
    else {
      $file_path = preg_replace('/^' . preg_quote(url('system/files'), '/') . '/', file_directory_path(), $item['filefield_imce']['file_path']);
    }

    // Resolve the file path to an FID.
    if ($fid = db_result(db_query("SELECT fid FROM {files} WHERE filepath = '%s'", rawurldecode($file_path)))) {
      $file = field_file_load($fid);
      if (filefield_sources_element_validate($element, (object) $file)) {
        $item = array_merge($item, $file);
      }
    }
    else {
      form_error($element, t('The selected file could not be used because the file does not exist in the database.'));
    }
    // No matter what happens, clear the value from the file path field.
    $item['filefield_imce']['file_path'] = '';
  }
}

/**
 * Theme the output of the autocomplete field.
 */
function theme_filefield_source_imce_element($element) {
  $output = theme('markup', $element['display_path']) . theme('hidden', $element['file_path']) . theme('submit', $element['select']);
  return '<div class="filefield-source filefield-source-imce clear-block">' . theme('form_element', $element, $output) . '</div>';
}

/**
 * Scan directory and return file list, subdirectories, and total size.
 */
function filefield_source_imce_custom_scan($dirname) {
  // Get a list of files in the database for this directory.
  $sql_dir_name = $dirname == '.' ? file_directory_path() : file_directory_path() .'/'. $dirname;

  $result = db_query("SELECT filename FROM {files} WHERE filepath LIKE '%s' AND filepath NOT LIKE '%s'", $sql_dir_name .'/%', $sql_dir_name .'/%/%');
  $db_files = array();
  while ($row = db_fetch_object($result)) {
    $db_files[$row->filename] = $row->filename;
  }

  // Get the default IMCE directory scan, then filter down to database files.
  $directory = imce_scan_directory($dirname);
  foreach ($directory['files'] as $filename => $file) {
    if (!isset($db_files[$filename])) {
      unset($directory['files'][$filename]);
      $directory['dirsize'] -= $file['size'];
    }
  }

  return $directory;
}

function filefield_source_imce_custom_process(&$imce) {
  // Disable uploading, thumbnails, delete, and other unnecessary features.
  $imce['perm']['subnav'] = 0;
  $imce['perm']['upload'] = 0;
  $imce['perm']['thumb']  = 0;
  $imce['perm']['delete'] = 0;
  $imce['perm']['resize'] = 0;
}