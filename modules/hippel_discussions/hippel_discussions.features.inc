<?php

/**
 * Implementation of hook_content_default_fields().
 */
function hippel_discussions_content_default_fields() {
  module_load_include('inc', 'hippel_discussions', 'hippel_discussions.defaults');
  $args = func_get_args();
  return call_user_func_array('_hippel_discussions_content_default_fields', $args);
}

/**
 * Implementation of hook_ctools_plugin_api().
 */
function hippel_discussions_ctools_plugin_api() {
  module_load_include('inc', 'hippel_discussions', 'hippel_discussions.defaults');
  $args = func_get_args();
  return call_user_func_array('_hippel_discussions_ctools_plugin_api', $args);
}

/**
 * Implementation of hook_node_info().
 */
function hippel_discussions_node_info() {
  module_load_include('inc', 'hippel_discussions', 'hippel_discussions.features.node');
  $args = func_get_args();
  return call_user_func_array('_hippel_discussions_node_info', $args);
}

/**
 * Implementation of hook_user_default_permissions().
 */
function hippel_discussions_user_default_permissions() {
  module_load_include('inc', 'hippel_discussions', 'hippel_discussions.defaults');
  $args = func_get_args();
  return call_user_func_array('_hippel_discussions_user_default_permissions', $args);
}

/**
 * Implementation of hook_strongarm().
 */
function hippel_discussions_strongarm() {
  module_load_include('inc', 'hippel_discussions', 'hippel_discussions.defaults');
  $args = func_get_args();
  return call_user_func_array('_hippel_discussions_strongarm', $args);
}

/**
 * Implementation of hook_views_default_views().
 */
function hippel_discussions_views_default_views() {
  module_load_include('inc', 'hippel_discussions', 'hippel_discussions.features.views');
  $args = func_get_args();
  return call_user_func_array('_hippel_discussions_views_default_views', $args);
}
