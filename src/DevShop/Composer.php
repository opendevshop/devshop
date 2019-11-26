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
   * @var array The list of folders to split into sub repos.
   */
  const REPOS = array(
      'devmaster' => 'https://$INPUT_GITHUB_TOKEN@github.com/opendevshop/devmaster.git',
      'roles/opendevshop.apache' => 'https://$INPUT_GITHUB_TOKEN@github.com/opendevshop/ansible-role-apache.git',
      'roles/opendevshop.devmaster' => 'https://$INPUT_GITHUB_TOKEN@github.com/opendevshop/ansible-role-devmaster.git',
      'roles/opendevshop.users' => 'https://$INPUT_GITHUB_TOKEN@github.com/opendevshop/ansible-role-users.git'
  );

  /**
   * Run the splitsh script on each repo.
   */
  static function splitRepos() {

    $current_branch = trim(shell_exec('git rev-parse --symbolic-full-name --abbrev-ref HEAD'));

    foreach (self::REPOS as $folder => $remote) {
      echo "\n\n- Splitting $folder for branch $current_branch ... \n";

      // Use a different local target branch so we dont break local installs by reassigning the current branch to the new commit.
      $target = "refs/splits/$folder";

      // Handle special case for devmaster
      if ($folder == 'devmaster' && $current_branch == '1.x') {
        $branch = '7.x-1.x';
        echo "\n\n- Pushing devmaster to 7.x-1.x ... \n";
      }
      else {
        $branch = $current_branch;
      }

      // Split the commits into a different branch.
      if (self::exec("splitsh-lite --progress --prefix={$folder}/ --target=$target") != 0) {
        exit(1);
      }

      // Push the branch to the remote.
      if (self::exec("git push --force $remote $target:refs/heads/$branch") != 0) {
        exit(1);
      }

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
