<?php

namespace DevShop;


use Symfony\Component\Process\Exception\ProcessFailedException;
use Symfony\Component\Process\Process;
use Symfony\Component\Process\ProcessBuilder;

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

    $process = new Process(['git', 'rev-parse', '--symbolic-full-name', '--abbrev-ref', 'HEAD']);
    $process->run();

    // executes after the command finishes
    if (!$process->isSuccessful()) {
      throw new ProcessFailedException($process);
    }

    $branch = trim($process->getOutput());
    foreach (self::REPOS as $folder => $remote) {
      passthru("splitsh-lite --progress --prefix={$folder}/ --target=HEAD", $exit);
      if ($exit != 0) {
        exit($exit);
      }

      passthru("git push $remote HEAD:$branch", $exit);
      if ($exit != 0) {
        exit($exit);
      }
    }
  }
}