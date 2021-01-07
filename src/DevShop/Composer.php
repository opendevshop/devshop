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

  static $drupalDevelopmentPaths = [
    'provision' => 'vendor/drupal/provision',
    'hosting' => 'src/DevShop/Control/web/sites/all/modules/contrib/hosting',
  ];

  /**
   * Prepare the codebase for development.
   *
   * 1. Checkout 7.x-4.x branches of provision and hosting.
   * 2. Enable devel and dblog.
   * 3. Add "refs/pull" to fetched objects.
   */
  static function prepareDevelopmentEnvironment() {

    // Add pull requests to tracked remote references to make it easy to checkout.
    $output = shell_exec('git config --get-all remote.origin.fetch');
    if (strpos($output, 'pulls') !== FALSE) {
      echo "> Pull Requests are being fetched. \n";
    }
    else {
      echo "> Pull Requests are not being fetched. Adding to git config... \n";
      self::exec('git config --add remote.origin.fetch +refs/pull/*/head:refs/remotes/origin/pr/*');
      self::exec('git fetch');
    }

    // Checkout branches for provision and hosting.
    echo "> Checking out branches for drupal projects... \n";
    foreach (self::$drupalDevelopmentPaths as $package => $path) {
      self::exec("cd {$path} && git checkout 7.x-4.x && git remote set-url origin git@git.drupal.org:project/{$package}.git");
    }

    // Enable devel module and dblog.
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
    if ($exit != 0) {
      throw new \Exception('Command failed: ' . $command);
    }
  }
}
