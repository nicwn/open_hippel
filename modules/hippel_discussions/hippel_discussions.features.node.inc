<?php

/**
 * Helper to implementation of hook_node_info().
 */
function _hippel_discussions_node_info() {
  $items = array(
    'discuss' => array(
      'name' => t('Discussion'),
      'module' => 'features',
      'description' => t('A new discussion topic'),
      'has_title' => '1',
      'title_label' => t('Topic'),
      'has_body' => '0',
      'body_label' => '',
      'min_word_count' => '0',
      'help' => '',
    ),
    'reply' => array(
      'name' => t('Reply'),
      'module' => 'features',
      'description' => t('Reply to a discussion topic'),
      'has_title' => '1',
      'title_label' => t('Title'),
      'has_body' => '0',
      'body_label' => '',
      'min_word_count' => '0',
      'help' => '',
    ),
  );
  return $items;
}
