<?php
// $Id: purl_subdomain.inc,v 1.1.2.4 2010/01/04 23:03:49 yhahn Exp $

/**
 * Subdomain prefixing.
 */
class purl_subdomain implements purl_processor {

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
    $parts = explode('.', str_replace('http://','',$_SERVER['HTTP_HOST']));
    return array_shift($parts);
  }

  public function method() {
    return 'subdomain';
  }

  public function description() {
    return t('Enter a sub-domain for this context, such as "mygroup".  Do not include http://');
  }

  public function parse($valid_values, $q) {
    $parsed = array();
    if (isset($valid_values[$q])) {
      $parsed[$q] = $valid_values[$q];
    }
    return purl_path_elements($this, $parsed);
  }

  public function adjust(&$value, $item, &$q) { return; }

  public function rewrite(&$path, &$options, $element) {
    $options['absolute'] = TRUE;
    if (!_purl_skip($element, $options)) {
      // Check to see if the link has already been treated.
      $parts = explode('.', str_replace('http://','', $options['base_url']));
      $possible = array_shift($parts);
      $matches = purl_parse($this, $possible);

      // If not add our subdomain.
      if (!count($matches)) {
        // ...but replace what we checked first.
        array_unshift($parts, $possible);
        array_unshift($parts, $element->value);

        $options['absolute'] = TRUE;
        $options['base_url'] = "http://". implode('.', $parts);
      }
    }
    else {
      $options['base_url'] = variable_get('purl_base_domain', $base_url);
    }
  }
}
