<?php

/**
 * Detect if an environment's path is on a branch or a checkout out tag (aka detachd HEAD)
 *
 * @param $environment
 *
 * @return string
 */
function devshop_environment_get_git_ref_type($environment_repo_path) {
  if (file_exists($environment_repo_path . '/.git')) {
    return null;
  }
  $git_status = shell_exec("cd {$environment_repo_path}; git status");
  if (strpos($git_status, 'On branch ') === 0) {
    return 'branch';
  }
  else {
    return 'tag';
  }
}
