<?php

/**
 * Class for the platform context.
 */
class Provision_Context_project extends Provision_Context {
  public $type = 'project';
  public $parent_key = 'server';

  function getEnvironment($name) {
    if (isset($this->project['environments'][$name])) {
      return (object) $this->project['environments'][$name];
    }
    else {
      return drush_set_error('DEVSHOP_PROJECT_ERROR', dt('Environment %name not found.', array(
        '%name' => $name,
      )));
    }
  }

  static function option_documentation() {
    return array(
      '--project_name' => 'Project: The codename for this project.',
      '--project' => 'Project: JSON encoded data about the project.',

      //@TODO: Document this!
//      '--code_path' => 'Project: The path to the project codebases.  (NOT the Drupal root)',
//      '--drupal_path' => 'Project: The path to the drupal root.',
//      '--git_url' => 'Project: The Git URL for this project.',
//      '--git_branches' => 'Project: The available Git branches in the remote repository for this project.',
//      '--git_tags' => 'Project: The available Git tags in the remote repository for this project.',
//      '--base_url' => 'Project: the base URL that the dev/test/live subdomains will be attached to.',
//      '--server' => 'Project: The server hosting this project.  (Default is @server_master)',
//      '--install_profile' => 'Project: The desired installation profile for all sites.',
    );
  }

  function init_project() {
    $this->setProperty('project_name');
    $this->setProperty('project');
  }

  function verify() {
    $this->type_invoke('verify');
    drush_log('[DEVSHOP] verify()', 'ok');
  }
}
