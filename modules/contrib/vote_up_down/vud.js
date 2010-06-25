// $Id: vud.js,v 1.1.2.1 2010/03/12 08:10:25 marvil07 Exp $
/**
 * @file
 *
 * Various customizations for all widgets.
 */

Drupal.behaviors.vudUpdownWidget = function () {
  $('.vud-widget .up-active,.vud-widget .up-inactive,.vud-widget .down-active,.vud-widget .down-inactive').click(function () {
    if ($(this).hasClass('denied')) {
      alert(Drupal.settings.vud_node.widget_message);
      return false;
    }
  });
}
