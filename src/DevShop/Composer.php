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
   * Run composer install on the DevShop Control sub-project.
   */
  static function installControl() {
    echo "> \n";
    echo "> Installing src/DevShop/Control ...\n";
    echo "> \n";
    $_SERVER['argv'][] = '--working-dir=src/DevShop/Control';
    $command = implode(' ', $_SERVER['argv']);
    return self::exec($command);
  }

  static $drupalDevelopmentPaths = [
    'provision' => 'vendor/drupal/provision',
    'hosting' => 'src/DevShop/Control/web/sites/all/modules/contrib/hosting',
  ];

  /**
   * Prepare the codebase for development.
   *
   * 1. Checkout 7.x-4.x branches of provision and hosting.
   * 2. Enable devel and dblog.
   */
  static function prepareDevelopmentEnvironment() {
    foreach (self::$drupalDevelopmentPaths as $package => $path) {
      self::exec("cd {$path} && git checkout 7.x-4.x && git remote set-url origin git@git.drupal.org:project/{$package}.git");
    }

    self::exec('bin/robo exec "drush @hm en devel dblog -y"');
  }

  /**
   * Show git status in important folders.
   */
  static function gitStatus() {
    foreach (self::$drupalDevelopmentPaths as $package => $path) {
      self::exec("cd {$path} && git status --ahead-behind");
    }
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
