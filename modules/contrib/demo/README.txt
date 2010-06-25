/* $Id: README.txt,v 1.5.2.2 2009/11/10 05:38:20 sun Exp $ */

-- SUMMARY --

Demo Site is a simple module that allows you to take a snapshot of a Drupal
demonstration site. It turns a Drupal installation into a sandbox that you can
use for testing modules or publicly demonstrating a module / extension / theme
(you name it). In short: With cron enabled, the Drupal site will be reset to
the dumped state in a definable interval. Of course you can reset the site
manually, too.

For a full description visit the project page:
  http://drupal.org/project/demo
Bug reports, feature suggestions and latest developments:
  http://drupal.org/project/issues/demo


-- REQUIREMENTS --

* Cron, or alternatively Poormanscron (http://drupal.org/project/poormanscron).


-- INSTALLATION --

* Copy the Demo module to your modules directory and enable it on the Modules
  page (admin/build/modules).

* Optionally configure who is allowed to administer Demo module, create dumps
  and reset the site on the Permissions page (admin/user/permissions).


-- CONFIGURATION --

* Configure the path where dumps will be stored at the Dump settings
  (admin/build/demo).

To configure automatic reset:

* Go to Manage snapshots (admin/build/demo/manage) and select a snapshot
  for cron.

* Enable atomatic reset from Dump settings (admin/build/demo). Make sure you
  have cron configured to run at least once within the entered time interval.


-- USAGE --

* Go to Create snapshot (admin/build/demo/dump) and create your first
  snapshot.

* After a while, reset your site (admin/build/demo/reset).


-- CONTACT --

Current maintainers:
* Daniel F. Kudwien (sun) - dev@unleashedmind.com
* Stefan M. Kudwien (smk-ka) - dev@unleashedmind.com

This project has been sponsored by:
* UNLEASHED MIND
  Specialized in consulting and planning of Drupal powered sites, UNLEASHED
  MIND offers installation, development, theming, customization, and hosting
  to get you started. Visit http://www.unleashedmind.com for more information.

