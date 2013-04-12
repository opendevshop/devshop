<?php

/**
 * Class for the platform context.
 */
class provisionContext_project extends provisionContext {
  public $parent_key = 'server';

  static function option_documentation() {
    return array(
      '--project_name' => 'Project: The codename for this project.',
      '--code_path' => 'Project: The path to the project codebases.  (NOT the Drupal root)',
      '--drupal_path' => 'Project: The path to the drupal root.',
      '--git_url' => 'Project: The Git URL for this project.',
      '--git_branches' => 'Project: The available Git branches in the remote repository for this project.',
      '--git_tags' => 'Project: The available Git tags in the remote repository for this project.',
      '--base_url' => 'Project: the base URL that the dev/test/live subdomains will be attached to.',
      '--server' => 'Project: The server hosting this project.  (Default is @server_master)',
      '--install_profile' => 'Project: The desired installation profile for all sites.',
    );
  }

  function init_project() {
    $this->setProperty('project_name');
    $this->setProperty('code_path');
    $this->setProperty('drupal_path');
    $this->setProperty('install_profile');
    $this->setProperty('git_url');
    $this->setProperty('git_branches');
    $this->setProperty('git_tags');
    $this->setProperty('base_url');
    $this->setProperty('settings');
  }

  function verify() {
    $this->type_invoke('verify');
    drush_log('[DEVSHOP] verify()', 'ok');
  }
}
