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

    // Extracts the currently checked out branch name.
    // In GitHub Actions, this is the branch created in the step "Create a branch for the splitsh"
    $current_ref = trim(shell_exec('git rev-parse --symbolic-full-name --abbrev-ref HEAD'));

    // If the Actions run was triggered by a push, the branch will be named "heads/refs/tags/TAG".
    $is_tag = strpos($current_ref, 'heads/refs/tags') === 0;

    // If is a tag, current_ref contains the string "refs/tags" already.
    if ($is_tag) {
      $bare_tag = str_replace('heads/refs/tags', '', $current_ref);
      $target_ref = $current_ref;
      $target_ref_devmaster = "refs/tags/7.x-$bare_tag";
    }
    else {
      $target_ref = "refs/heads/$current_ref";
      $target_ref_devmaster = "refs/heads/7.x-$current_ref";
    }

    foreach (self::REPOS as $folder => $remote) {
      echo "\n\n- Splitting $folder for git reference $current_ref ... \n";

      // Use a different local target branch so we dont break local installs by reassigning the current branch to the new commit.
      $target = "refs/splits/$folder";

      // Split the commits into a different branch.
      // @TODO: When this becomes a composer plugin, pass -v to --progress.
      $progress = '';// '--progress';
      if (self::exec("splitsh-lite {$progress} --prefix={$folder}/ --target=$target") != 0) {
        exit(1);
      }

      // Push the current_ref to the remote.
      if (self::exec("git push --force $remote $target:$target_ref") != 0) {
        exit(1);
      }

      // Handle special case for devmaster
      // Push an additional "7.x-1.x" or "7.x-2.x" branch to remote if splitting 1.x or 2.x
      if ($folder == 'devmaster') {
        echo "\n\n- Pushing devmaster to $target_ref_devmaster ... \n";
        if (self::exec("git push --force $remote $target:$target_ref_devmaster") != 0) {
          exit(1);
        }
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
