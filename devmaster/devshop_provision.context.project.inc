<?php

/**
 * Class for the platform context.
 */
class provisionContext_project extends provisionContext {
  public $parent_key = 'project';

  static function option_documentation() {
    return array(
      '--path' => 'Project: Path to the project root.',
      '--git-url' => 'Project: Git URL',
      '--server' => 'Project: drush backend server; default @server_master',
    );
  }

  function init_project() {
    $this->setProperty('path');
    $this->setProperty('git-url');
  }
}