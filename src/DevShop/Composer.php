<?php

namespace DevShop;

/**
 * Class Composer
 *
 * @package DevShop
 */
class Composer {

  /**
   * Run ansible-galaxy install --force to update the Ansible roles included in DevShop.
   */
  static function updateRoles() {
    $ansible_playbook_installed = `command -v ansible`;
    if ($ansible_playbook_installed) {
      putenv('ANSIBLE_FORCE_COLOR=1');
      echo "> \n";
      echo "> Updating Ansible roles ...\n";
      echo "> \n";
      self::exec('ansible-galaxy install -r roles/roles.yml -p roles --force');
    }
    else {
      echo "Ansible not found. Skipping Ansible Galaxy Role update. \n";
    }
  }

  /**
   * Run composer update on the DevShop Control sub-project.
   */
  static function updateControl() {
    echo "> \n";
    echo "> Updating src/DevShop/Control ...\n";
    echo "> \n";
    self::exec('composer update --working-dir=src/DevShop/Control --ansi');
  }

  /**
   * Prepare the codebase for development.
   *
   * 1. Checkout 7.x-4.x branches of provision and hosting.
   * 2. Enable devel and dblog.
   */
  static function prepareDevelopmentEnvironment() {
    self::exec('cd src/DevShop/Control/web/sites/all/modules/contrib/hosting && git checkout 7.x-4.x && git remote set-url origin git@git.drupal.org:project/hosting.git');
    self::exec('cd vendor/drupal/provision && git checkout 7.x-4.x && git remote set-url origin git@git.drupal.org:project/provision.git');
    self::exec('bin/robo exec "drush @hm en devel dblog -y"');
  }

  /**
   * Print the command then run it.
   * @param $command
   *
   * @return mixed
   */
  static function exec($command) {
    echo "> $command \n";
    passthru($command, $exit);
    return $exit;
  }
}
