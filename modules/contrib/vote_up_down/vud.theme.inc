<?php
// $Id: vud.theme.inc,v 1.1.2.32 2010/05/31 03:17:52 marvil07 Exp $

/**
 * @file
 * Theme functions
 */

define('VUD_WIDGET_MESSAGE_ERROR', 0);
define('VUD_WIDGET_MESSAGE_DENIED', 1);

/**
 * Implementation of hook_ctools_plugin_*.
 *
 * Give information to CTools about the widgets plugin.
 */
function vud_ctools_plugin_widgets() {
  return array(
    'cache' => FALSE,
    'defaults' => 'vud_width_plugin_defaults',
    // Themes can offer this plugin.
    'load themes' => TRUE,
  );
}

/**
 * Provide defaults for widgets.
 */
function vud_width_plugin_defaults($info, &$plugin) {
  $defaults = array(
    'render function' => 'theme_render_template',
    'extension' => '.tpl.php',
    'css' => $plugin['name'] . '.css',
  );

  $plugin += $defaults;
}

/**
 * Load metadata for a single widget without loading all widgets.
 */
function vud_widget_get($name) {
  ctools_include('plugins');
  return ctools_get_plugins('vud', 'widgets', $name);
}

/**
 * Load metadata for all widgets
 */
function vud_widget_get_all() {
  ctools_include('plugins');
  return ctools_get_plugins('vud', 'widgets');
}

/**
 * Load the names of all widgets for use in a select.
 *
 * This can be given directly to #options when choosing a widget.
 */
function vud_widget_get_names() {
  $names = array();
  foreach (vud_widget_get_all() as $name => $plugin) {
    $names[$name] = $plugin['title'];
  }

  asort($names);
  return $names;
}

/**
 * Build a list of theme function names.
 */
function vud_theme_functions($hook, $type, $content_id) {
  switch ($type) {
    case 'node':
      $node = node_load($content_id);
      $themes[] = $hook . '_' . $type . '__' . $node->type;
      break;
    case 'comment':
      $comment = _comment_load($content_id);
      $node = node_load($comment->nid);
      $themes[] = $hook . '_' . $type . '__' . $node->type;
      break;
    case 'term':
      $term = taxonomy_get_term($content_id);
      $themes[] = $hook . '_' . $type . '__' . $term->vid;
      break;
  }
  $themes[] = $hook . '_' . $type;
  $themes[] = $hook;
  return $themes;
}

/**
 * Implementation of hook_theme().
 */
function vud_theme() {
  return array(
    'vud_widget' => array(
      'function'  => 'vud_widget_proxy',
      'arguments' => array('content_id' => NULL, 'type' => NULL, 'tag' => NULL, 'widget_theme' => NULL),
    ),
  );
}

/**
 * Proxy widget function that hook_theme() calls.
 */
function vud_widget_proxy($content_id, $type, $tag, $widget_theme, $readonly=NULL, $widget_message_code=VUD_WIDGET_MESSAGE_ERROR) {
  $plugin = vud_widget_get($widget_theme);
  if (empty($plugin) || empty($plugin['widget template'])) {
    return;
  }

  $template_file = $plugin['path'] . '/' . $plugin['widget template'] . $plugin['extension'];
  $variables = array(
    'content_id' => $content_id,
    'type' => $type,
    'widget_theme' => $widget_theme,
    'plugin' => $plugin,
    'tag' => $tag ? $tag : variable_get('vud_tag', 'vote'),
    'id' => 'widget-' . $type . '-' . $content_id,
    'link_class_up' => 'vud-link-up',
    'link_class_down' => 'vud-link-down',
  );

  global $user;
  $uid = votingapi_current_user_identifier();

  ctools_add_js('ajax-responder');
  if ($type == 'comment') {
    drupal_add_css(drupal_get_path('module', 'vud_comment') . '/vud_comment.css');
  }
  vud_add_files('css', $plugin);  // Search and add the CSS files.
  vud_add_files('js', $plugin);   // Search and add the JS files.

  $base_criteria = array(
    'content_type' => $type,
    'content_id' => $content_id,
    'tag' => $variables['tag'],
  );
  $criteria = $base_criteria + $uid;
  $user_vote = votingapi_select_single_vote_value($criteria);

  if ($user_vote > 0) {
    $variables['class_up'] = 'up-active';
    $variables['class_down'] = 'down-inactive';
  }
  elseif ($user_vote < 0) {
    $variables['class_up'] = 'up-inactive';
    $variables['class_down'] = 'down-active';
  }
  else {
    $variables['class_up'] = 'up-inactive';
    $variables['class_down'] = 'down-inactive';
  }

  $token_up = drupal_get_token("vote/$type/$content_id/1/$tag/$widget_theme");
  $token_down = drupal_get_token("vote/$type/$content_id/-1/$tag/$widget_theme");

  $result_criteria = array(
    'content_type' => $type,
    'content_id' => $content_id,
    'value_type' => 'points',
    'tag' => $tag,
    'function' => 'sum'
  );
  $vote_result = (int) votingapi_select_single_result_value($result_criteria);
  $variables['unsigned_points'] = $vote_result;

  if ($vote_result > 0) {
    $variables['class'] = 'positive';
    $variables['points'] = '+'. $vote_result;
  }
  else {
    $variables['points'] = $vote_result;
    if ($vote_result < 0) {
      $variables['class'] = 'negative';
    }
    else {
      $variables['class'] = 'neutral';
    }
  }
  $variables['vote_label'] = format_plural($vote_result, 'vote', 'votes');

  $variables['readonly'] = $readonly;
  $link_up = url("vote/$type/$content_id/1/$tag/$widget_theme/$token_up");
  $link_down = url("vote/$type/$content_id/-1/$tag/$widget_theme/$token_down");
  $message_on_deny = variable_get('vud_message_on_deny', FALSE);
  $variables['show_links'] = !$readonly || $message_on_deny;
  if ($readonly) {
    $variables['link_class_up'] .= ' denied';
    $variables['link_class_down'] .= ' denied';
    if ($message_on_deny) {
      ctools_include('modal');
      ctools_modal_add_js();
      $variables['link_class_up'] .= ' ctools-use-modal';
      $variables['link_class_down'] .= ' ctools-use-modal';
      $link_up = url(sprintf('vud/nojs/denied/%d', $widget_message_code));
      $link_down = url(sprintf('vud/nojs/denied/%d', $widget_message_code));
    }
    else {
      // no vote access, so avoid show the link at template
      $link_up = '#';
      $link_down = '#';
    }
  }
  else {
    $variables['link_class_up'] .= ' ctools-use-ajax';
    $variables['link_class_down'] .= ' ctools-use-ajax';
  }

  $variables['link_up'] = $link_up;
  $variables['link_down'] = $link_down;

  $output = $plugin['render function']($template_file, $variables);
  return $output;
}

/**
 * Proxy votes display function, that hook_theme() calls.
 */
function vud_votes_proxy($content_id, $type, $tag, $widget_theme) {
  $plugin = vud_widget_get($widget_theme);
  if (empty($plugin) || empty($plugin['votes template'])) {
    return;
  }

  $template_file = $plugin['path'] . '/' . $plugin['votes template'] . $plugin['extension'];
  $variables = array(
    'content_id' => $content_id,
    'type' => $type,
    'widget_theme' => $widget_theme,
    'plugin' => $plugin,
    'id' => 'votes-' . $type . '-' . $content_id,
  );

  $variables['tag'] = $tag;

  $criteria = array(
    'content_type' => $type,
    'content_id' => $content_id,
    'value_type' => 'points',
    'tag' => $tag,
    'function' => 'sum'
  );

  $vote_result = (int) votingapi_select_single_result_value($criteria);
  $variables['unsigned_points'] = $vote_result;

  if ($vote_result > 0) {
    $variables['class'] = 'positive';
    $variables['points'] = '+'. $vote_result;
  }
  else {
    $variables['points'] = $vote_result;
    if ($vote_result < 0) {
      $variables['class'] = 'negative';
    }
    else {
      $variables['class'] = 'neutral';
    }
  }
  $variables['vote_label'] = format_plural($vote_result, 'vote', 'votes');

  vud_add_files('css', $plugin);  // Search and add the CSS files.
  vud_add_files('js', $plugin);   // Search and add the JS files.

  if (file_exists($template_file)) {
    return $plugin['render function']($template_file, $variables);
  }
}

/**
 * Read and load all CSS or JS files in the selected widget directory.
 */
function vud_add_files($type, $plugin) {
  $function = 'drupal_add_' . $type;
  if (empty($plugin[$type])) {
    return;
  }

  if (is_string($plugin[$type])) {
    $css = array($plugin[$type]);
  }

  if (is_array($plugin[$type])) {
    $css = $plugin[$type];
  }

  if (!empty($css)) {
    foreach ($css as $file) {
      $function($plugin['path'] . '/' . $file);
    }
  }
}

/**
 * Function for the main voting handler with Ajax support.
 */
function vud_vote($type, $content_id, $value, $tag, $widget, $token) {
  if (is_numeric($value) && drupal_valid_token($token, "vote/$type/$content_id/$value/$tag/$widget")) {
    $vote = array();
    // Sanity-check the incoming values.
    if ($value > 0) {
      $vote['value'] = 1;
    }
    elseif ($value < 0) {
      $vote['value'] = -1;
    }
    else {
      $vote['value'] = 0;
    }

    $vote['value_type'] = 'points';
    $tag = $tag ? $tag : variable_get('vud_tag', 'vote');
    $vote['tag'] = $tag;

    $vote['content_id'] = $content_id;
    $vote['content_type'] = $type;
    $vote['tag'] = $tag;
    $votes = array(0 => $vote);
    drupal_alter('vud_votes', $votes);
    votingapi_set_votes($votes);
  }
  else {
    watchdog("vud", "Could not vote on $type $content_id, with value $value, tag $tag and token $token");
    drupal_set_message(t("Oops! There was an error in submitting your vote!"), 'warning');
  }

  if (isset($_SERVER['HTTP_X_REQUESTED_WITH']) && $_SERVER['HTTP_X_REQUESTED_WITH'] == 'XMLHttpRequest') {
    ctools_include('ajax');
    $plugin = vud_widget_get($widget);

    if ($function = ctools_plugin_get_function($plugin, 'ajax render')) {
      $commands = $function($type, $content_id, $value, $tag, $token, $widget);
    }
    else {
      $commands = array();
      if (!empty($plugin['widget template'])) {
        $commands[] = ctools_ajax_command_replace("#widget-$type-$content_id", theme(vud_theme_functions('vud_widget', $type, $content_id), $content_id, $type, $tag, $widget));
      }

      if (!empty($plugin['votes template'])) {
        $commands[] = ctools_ajax_command_replace("#votes-$type-$content_id", theme(vud_theme_functions('vud_votes', $type, $content_id), $content_id, $type, $tag, $widget));
      }
    }

    // This is the default set of commands. It can be overridden by an individual
    // widget if it wants to.
    ctools_ajax_render($commands);
  }
  else {
    drupal_goto($_SERVER['HTTP_REFERER']);
  }
}
