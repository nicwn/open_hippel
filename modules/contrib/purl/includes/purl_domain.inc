<?php
// $Id: purl_domain.inc,v 1.1.2.4 2010/01/04 23:03:49 yhahn Exp $

/**
 *  Full domain handling.
 */
class purl_domain implements purl_processor {

  public function admin_form(&$form, $id) {
    $form['purl_location'] = array(
      '#type' => 'fieldset',
    );
    $form['purl_location']['purl_base_domain'] = array(
      '#type' => 'textfield',
      '#title' => t('Default domain'),
      '#description' => t('Enter the default domain if you are using domain modifiers.'),
      '#required' => FALSE,
      '#default_value' => variable_get('purl_base_domain', $base_url),
    );
  }

  function detect($q) {
    return str_replace('http://','',$_SERVER['HTTP_HOST']);
  }

  public function method() {
    return 'domain';
  }

  public function description() {
    return t('Enter a domain registered for this context, such as "www.example.com".  Do not include http://');
  }

  /**
   * Simply match our 'q' (aka domain) against an allowed value.
   */
  public function parse($valid_values, $q) {
    $parsed = array();
    if (isset($valid_values[$q])) {
      $parsed[$q] = $valid_values[$q];
    }
    return purl_path_elements($this, $parsed);
  }

  public function adjust(&$value, $item, &$q) { return; }

  /**
   * Either force the url, or set it back to the base.
   */
  public function rewrite(&$path, &$options, $element) {
    $options['absolute'] = TRUE;
    if (!_purl_skip($element, $options)) {
      $options['base_url'] = "http://{$element->value}";
    }
    else {
      $options['base_url'] = variable_get('purl_base_domain', $base_url);
    }
  }
}

