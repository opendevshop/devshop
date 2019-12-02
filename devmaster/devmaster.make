;
; devmaster.make
;
; This file is to ensure devmaster's requirements are loaded automatically when
; another makefiles requires "devmaster".
;
; This is how drush make behaves for install profiles: if a PROJECTNAME.make
; file is found, it loads it into the build.

core = 7.x
api = 2

includes[devmaster] = drupal-org.make
