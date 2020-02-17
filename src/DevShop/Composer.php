<?php

namespace DevShop;

/**
 * Class Composer
 *
 * Inpsired by https://bitbucket.org/drupalorg-infrastructure/subtree-splitter/src/master/src/SplitCommand.php
 *
 * @package DevShop
 */
class Composer {

  const BIN_FILES = array(
    'drush' => 'https://github.com/drush-ops/drush/releases/download/8.3.0/drush.phar',
    'splitsh-lite' => 'https://github.com/splitsh/lite/releases/download/v1.0.1/lite_linux_amd64.tar.gz',
  );

  /**
   * Run ansible-galaxy install --force to update the Ansible roles included in DevShop.
   */
  static function updateRoles() {
    $ansible_playbook_installed = `command -v ansible`;
    if ($ansible_playbook_installed) {
      self::exec('ansible-galaxy install -r roles/roles.yml -p roles --force');
    }
    else {
      echo "Ansible not found. Skipping Ansible Galaxy Role update. \n";
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
