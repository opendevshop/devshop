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
    'splitsh' => 'https://bitbucket.org/drupalorg-infrastructure/subtree-splitter/raw/5d81a6fafd1802659369e4b8cbcc64bb3103db8a/splitsh-lite',
  );

  /**
   * Install binary files.
   */
  static function installBins() {
    foreach (self::BIN_FILES as $name => $url) {
      $bin_path = "bin/{$name}";
      copy($url, $bin_path);
      chmod($bin_path, 0755);
      echo "Installed $url to $bin_path \n";
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