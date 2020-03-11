<?php

namespace DevShop\Component\GitSplit;

/**
 * Splitter offers a command to split a git repository into multiple sub repositories using the splitsh script.
 *
 * @author Jon Pugh <jon@thinkdrop.net>
 */
class Splitter {

  const SPLITSH_NAME = 'splitsh-lite';
  const SPLITSH_URL = 'https://github.com/splitsh/lite/releases/download/v1.0.1/lite_linux_amd64.tar.gz';
  const BIN_FILES = array(
    'splitsh-lite' => 'https://github.com/splitsh/lite/releases/download/v1.0.1/lite_linux_amd64.tar.gz',
  );

  /**
   * Install splitsh-lite script.
   */
  static function install($bin_dir = 'bin') {

      $name = self::SPLITSH_NAME;
      $url = self::SPLITSH_URL;

      // @TODO: Load BIN path from composer project bin path.
      $bin_path = "{$bin_dir}/{$name}";
      if (file_exists($bin_path)) {
        echo "- $name already installed at $bin_path \n";
        return;
      }

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

  /**
   * Run the splitsh script on each repo.
   */
  static function splitRepos($repos, $show_progress = false, $bin_dir = './bin') {

    // Extracts the currently checked out branch name.
    // In GitHub Actions, this is the branch created in the step "Create a branch for the splitsh"
    // @TODO: Use branch-or-tag command.
    $current_ref = trim(shell_exec('git rev-parse --symbolic-full-name --abbrev-ref HEAD'));

    // If the Actions run was triggered by a push, the branch will be named "heads/refs/tags/TAG".
    $is_tag = strpos($current_ref, 'heads/refs/tags') === 0;

    // If is a tag, current_ref contains the string "refs/tags" already.
    if ($is_tag) {
      $bare_tag = str_replace('heads/refs/tags/', '', $current_ref);
      $target_ref = str_replace('heads/', '', $current_ref);
      $target_ref_devmaster = "refs/tags/7.x-$bare_tag";
    }
    else {
      $target_ref = "refs/heads/$current_ref";
      $target_ref_devmaster = "refs/heads/7.x-$current_ref";
    }

    foreach ($repos as $folder => $remote) {
      echo "\n\n- Splitting $folder for git reference $current_ref to $remote ... \n";


      // Use a different local target branch so we dont break local installs by reassigning the current branch to the new commit.
      $target = "refs/splits/$folder";

      // Split the commits into a different branch.
      // @TODO: When this becomes a composer plugin, pass -v to --progress.
      $progress = $show_progress? '--progress': '';

      $relative_path = "{$bin_dir}/splitsh-lite";
      $command = realpath($relative_path);
      if (!file_exists($command)) {
        throw new \Exception("The script splitsh-lite was not found in {$relative_path}.");

      }
      elseif (!is_executable($command)) {
        throw new \Exception("The script splitsh-lite file is not executable: {$relative_path}");
      }

      if (self::exec("{$command} {$progress} --prefix={$folder}/ --target=$target") != 0) {
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
