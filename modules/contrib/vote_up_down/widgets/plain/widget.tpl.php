<?php
// $Id: widget.tpl.php,v 1.1.2.14 2010/05/31 03:17:52 marvil07 Exp $
/**
 * @file
 * widget.tpl.php
 *
 * Plain widget theme for Vote Up/Down
 */
?>
<?php if ($show_links): ?>
  <div class="vud-widget vud-widget-plain" id="<?php print $id; ?>">
    <a href="<?php print $link_up; ?>" class="<?php print $link_class_up; ?>">
      <span class="<?php print $class_up; ?>" title="<?php print t('Vote up!'); ?>"></span>
    </a>
    <a href="<?php print $link_down; ?>" class="<?php print $link_class_down; ?>">
      <span class="<?php print $class_down; ?>" title="<?php print t('Vote down!'); ?>"></span>
    </a>
  </div>
<?php endif; ?>
