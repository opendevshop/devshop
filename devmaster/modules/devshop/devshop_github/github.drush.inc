<?php

/**
 * Detect if an environment's path is on a branch or a tag (aka detached HEAD).
 *
 * @param string $environment_repo_path
 *   Path to a Git repository.
 *
 * @return string
 */
function devshop_environment_get_git_ref_type($environment_repo_path) {
  if (!file_exists($environment_repo_path . '/.git')) {
    return NULL;
  }
  $git_status = shell_exec("cd {$environment_repo_path}; git status");

  // @TODO: Use the git library for this!!
  if (strpos($git_status, 'On branch ') === 0 || strpos($git_status, 'Your branch ') === 0) {
    return 'branch';
  }
  else {
    return 'tag';
  }
}
