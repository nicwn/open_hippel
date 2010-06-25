// $Id: README.txt,v 1.14.2.2.2.2 2010/04/19 08:14:38 marvil07 Exp $

= Vote Up/Down =

== Overview ==

This module will let you make votes on some entities inside drupal with
different widgets, by using Voting API and its some submodules: vud_node,
vud_comment and vud_term.

AJAX functionality, provided by ctools, is used for voting if the browser
supports it.

=== Features ===

* Vote on the entities: nodes, comments and terms.

To submit bug reports and feature suggestions, or to track changes:
  http://drupal.org/project/issues/vote_up_down

== Requirements ==

* Drupal modules:
** voting_api: http://drupal.org/project/voting_api
** ctools: http://drupal.org/project/ctools

== Instalation ==

* Install as usual, see http://drupal.org/node/70151 for further
information.

== Configuration ==

* Configure permissions in Administer >> User management >> Permissions
>> vud* module:
*** vud module
**** access vote up/down statistics
+
Users in roles with the 'access vote up/down statistics' permission
will be able to see the votes performed by each user on its 'Votes' tab.
+
**** administer vote up/down
+
Users in roles with the 'administer vote up/down' permission will be able
to modify the Voting API tag for Vote Up/Down votes.
+
**** reset vote up/down votes
+
Users in roles with the 'reset vote up/down votes' permission will be able
to undo their own votes if it's also permitted in the configuration for
the respective module.
+
**** use vote up/down
+
Users in roles with the 'use vote up/down' permission will be able to
actually cast a vote with vote up/down(for the callback).
+
*** vud_comment
**** administer vote up/down on comments 
+
Users in roles with the 'administer vote up/down on comments' permission
will be able to modify the settings regardin voting on comments.
+
**** use vote up/down on comments
+
Users in roles with the 'use vote up/down on comments' permission will be
able to see the widget for casting votes on comments.
+
*** vud_node
**** administer vote up/down on nodes
+
Users in roles with the 'administer vote up/down on nodes' permission
will be able to modify the settings regarding voting on nodes.
+
**** see vote up/down node stats
+
Users in roles with the 'see vote up/down node stats' permission will be
able to see the votes performed by each user on the 'Voting details' node
tab.
+
**** use vote up/down on nodes
+
Users in roles with the 'use vote up/down on nodes' permission will be
able to see the widget for casting votes on nodes.
+
**** view vote up/down count on nodes
+
Users in roles with the 'view vote up/down count on nodes' permission
will be able to see the widget for casting votes on nodes.
+
*** vud_term
**** administer vote up/down on terms
+
Users in roles with the 'administer vote up/down on terms' permission
will be able to modify the settings regarding voting on terms.
+
**** use vote up/down on terms
+
Users in roles with the 'use vote up/down on terms' permission will be
able to see the widget for casting votes on terms.
+
* Customize the  settings in Administer >> Site configuration >>
Vote Up/Down, so you can
** match:
*** vud_node: what node types should be votable
*** vud_comment: what comments of node types should be votable
*** vid_term: what terms of node types should be votable
** which widget is going to be used
** if you want to display the widget in on teasers/full nodes
** if the user is going to be able to reset their own votes
* Start voting!

== Customization ==

You can write your own widget for Vote Up/Down, and you can put it in
your module or in your theme. Please take a look to WIDGETAPI.txt for more
information.

== Credits ==

Original Author - Fredrik Jonsson fredrik at combonet dot se

Orginal 2.x version - Pratul Kalia (lut4rp)

Current Maintainer: Marco Villegas (marvil07)
