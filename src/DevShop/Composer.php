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
   * Install binary files.
   */
  static function installBins() {
    foreach (self::BIN_FILES as $name => $url) {
      $bin_path = "bin/{$name}";

      if (strpos($url, 'tar.gz') !== FALSE) {
        $filename = sys_get_temp_dir() . "/$name";
        $filename_tar = "$filename.tar";
        $filename_tar_gz = "$filename_tar.gz";

        echo "- Downloading to $filename_tar_gz \n";
        copy($url, $filename_tar_gz);

        passthru("tar zxf $filename_tar_gz");
        rename("./" . $name, $bin_path);
      }
      else {
        copy($url, $bin_path);
      }

      chmod($bin_path, 0755);
      echo "- Installed $url to $bin_path \n";
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
