// $Id: imce.js,v 1.1 2009/04/17 00:20:14 quicksketch Exp $

/**
 * JavaScript behavior to auto-trigger the IMCE FileField source when selected.
 */
Drupal.behaviors.fileFieldIMCE = function() {
  $('a.filefield-source-imce').unbind('click.fileFieldIMCE').bind('click.fileFieldIMCE', function() {
    $(this).parents('div.filefield-element:first').find('a.filefield-sources-imce-browse').click();
    return false;
  });
};
