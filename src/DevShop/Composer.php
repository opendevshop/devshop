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
   * Install binary files.
   */
  static function installBins() {
    foreach (self::BIN_FILES as $name => $url) {
      $bin_path = "bin/{$name}";

      if (strpos($url, 'tar.gz') !== FALSE) {
        $filename = sys_get_temp_dir() . "/$name";
        $filename_tar = "$filename.tar";
        $filename_tar_gz = "$filename_tar.gz";

        echo "\n- Downloading to $filename_tar_gz";
        copy($url, $filename_tar_gz);

        passthru("tar zxf $filename_tar_gz");
        rename("./" . $name, $bin_path);
      }
      else {
        copy($url, $bin_path);
      }

      chmod($bin_path, 0755);
      echo "\n- Installed $url to $bin_path";
    }
  }

  /**
   * @var array The list of folders to split into sub repos.
   */
  const REPOS = array(
      'devmaster' => 'git@github.com:opendevshop/devmaster.git',
      'roles/opendevshop.apache' => 'git@github.com:opendevshop/ansible-role-apache.git',
      'roles/opendevshop.devmaster' => 'git@github.com:opendevshop/ansible-role-devmaster.git',
      'roles/opendevshop.users' => 'git@github.com:opendevshop/ansible-role-users.git'
  );

  /**
   * Run the splitsh script on each repo.
   */
  static function splitRepos() {
    foreach (self::REPOS as $folder => $remote) {
      passthru("splitsh-lite --progress --prefix={$folder}/ --target=refs/heads/split/$folder", $exit);
      if ($exit != 0) {
        exit($exit);
      }

      passthru("git push $remote refs/heads/split/$folder:refs/heads/split/$folder", $exit);
      if ($exit != 0) {
        exit($exit);
      }
    }
  }
}