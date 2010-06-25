<?php
// $Id: custom_breadcrumbs_common.inc,v 1.1.2.6 2009/11/29 03:54:11 mgn Exp $

/**
 * @file
 * Common helper functions used by custom breadcrumbs submodules.
 */

/**
 * Helper function to determine if two paths match, allowing for wildcards and aliases.
 *
 * @param $curpath
 *   The current Drupal path.
 * @param $breadcrumb_path 
 *   The path that the breadcrumb applies to.
 *
 * @return
 *    TRUE (1) if the paths match, FALSE (0) otherwise.
 */
function _custom_breadcrumbs_match_path($curpath, $breadcrumb_path) {
  $path = drupal_get_path_alias($curpath);
  // Compare with the internal and path alias (if any).
  $page_match = drupal_match_path(drupal_strtolower($path), drupal_strtolower($breadcrumb_path));
  if ($path != $curpath) {
   $page_match = $page_match || drupal_match_path(drupal_strtolower($curpath), drupal_strtolower($breadcrumb_path));
  }
  return $page_match;
}

/**
 * Helper function to construct the view path replacing wildcards with arguments.
 *
 * @param $display
 *   The view $display object.
 * @param $viewsargs
 *   The $view->args array.
 *
 * @return $viewpath
 *   The complete path to the view.
 */
function _custom_breadcrumbs_construct_view_path($display, $viewargs) {
  $bits = explode('/', $display->display_options['path']);
  foreach ($bits as $pos => $bit) {
    if ($bit == '%' && !empty($viewargs)) {
      $bits[$pos] = array_shift($viewargs);
    }
  }
  if (!empty($viewargs)) {
    // Add any additional arguments to end of path.
    $bits = array_merge($bits, $viewargs);
  }
  $viewpath = implode('/', $bits);
  return $viewpath;
}

/**
 *  Helper function to specify allowed views displays for breadcrumb placement.
 *
 * @param $display
 *   The view $display object.
 *
 * @return
 *    TRUE if the display should be considered for a custom breadcrumb, FALSE otherwise.
 */
function _custom_breadcrumbs_allowed_display($display) {

  if (drupal_substr($display->id, 0, 4)  == 'page' || drupal_substr($display->id, 0, 8)  == 'calendar') {
    if (!(isset($display->handler->view->is_attachment) && $display->handler->view->is_attachment)) {
      if (isset($display->display_options['path']) ) {
        if (module_exists('panels') && panels_get_current_page_display()) {
          return FALSE;
        }
        return TRUE;
      }
    }
  }
  return FALSE;
}

/**
 *  Helper function to obtain the appropriate type and value of each view argument.
 *
 * @param $display_argument_ids
 *   An array of ids for each argument of the view.
 * @param $viewargs
 *   The $display->handler->view->args array.
 *
 * @return $arg_values
 *   An associative array of two elements, 'types' and 'values', each an array with elements 
 *   corresponding to the views arguments.
 */
function _custom_breadcrumbs_views_parse_args($arguments, $viewargs) {
  $arg_values = array('types' => array(), 'values' => array());
  foreach ($arguments as $arg_id => $argument) {
    if (!empty($viewargs)) {
      $arg = array_shift($viewargs);
      $arg_id_3 = drupal_substr($arg_id, 0, 3);
      if (($arg_id_3 == 'tid') || (drupal_substr($arg_id, 0, 19) == 'term_node_tid_depth')) {
        $terms = taxonomy_terms_parse_string($arg);
        $arg_values['types'][] = 'tid';
        $arg_values['values'][] = empty($terms['tids']) ? NULL : $terms['tids'][0];
      }
      elseif (drupal_substr($arg_id, 0, 4) == 'name') {
        if (drupal_substr($argument['table'], 0, 5) == 'term_') {
          $terms = taxonomy_get_term_by_name($arg);
          $arg_values['types'][] = 'tid';
          $arg_values['values'][] = empty($terms) ? NULL : $terms[0]->tid;
        }
      }
      elseif (($arg_id_3 == 'vid') || ($arg_id_3 == 'uid') || ($arg_id_3 == 'nid')) {
        $arg_values['types'][] = $arg_id_3;
        $arg_values['values'][] = $arg;
      }
    }
  }
  return $arg_values;
}

/**
 *  Helper function to obtain the appropriate objects for token type replacement for this view.
 *
 * @param $display
 *   The view $display object.
 *
 * @return $objs
 *   An associate array of objects to use for token replacement.
 */
function _custom_breadcrumbs_views_token_types($display) {
  $objs = array();
  // Check to see if the current display has overriden the default arguments.
  $arguments = _custom_breadcrumbs_views_display_arguments($display);
  if (isset($arguments) && !empty($arguments)) {
    $viewargs = (isset($display->handler->view->args) && is_array($display->handler->view->args)) ? $display->handler->view->args : array();
    $arg_values = _custom_breadcrumbs_views_parse_args($arguments, $viewargs);
    foreach ($arg_values['types'] as $key => $type) {
      switch ($type) {
        case 'tid':
          $objs['taxonomy'] = taxonomy_get_term($arg_values['values'][$key]);
          break;
        case 'nid':
          $objs['node'] = node_load(array('nid' => $arg_values['values'][$key]));
          break;
        case 'uid':
          $objs['user'] = user_load(array('uid' => $arg_values['values'][$key]));
          break;
      }
    }
  }
  return $objs;
}

/**
 *  Helper function to extract the views display option arguments array from the display.
 *
 * @param $display
 *   The view $display object.
 * @return $arguments
 *   The view display option arguments array.
 */
function _custom_breadcrumbs_views_display_arguments($display) {
  $arguments = NULL;
  if (isset($display->handler->view->display[$display->id]->display_options['arguments'])) {
    $arguments = $display->handler->view->display[$display->id]->display_options['arguments'];
  }
  if (!isset($arguments) && isset($display->handler->view->display['default']->display_options['arguments'])) {
    $arguments = $display->handler->view->display['default']->display_options['arguments'];
  }
  return $arguments;
}
