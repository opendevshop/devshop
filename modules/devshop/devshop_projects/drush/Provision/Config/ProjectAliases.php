<?php
/**
 * @file
 * Provides the Provision_Config_Drushrc_Alias class.
 */

/**
 * Class to write an alias records.
 */
class Provision_Config_ProjectAliases extends Provision_Config_Drushrc {
  public $template = 'project_aliases.tpl.php';

  /**
   * @param $name
   *   String '\@name' for named context.
   * @param $options
   *   Array of string option names to save.
   */
  function __construct($context, $project = array()) {

    parent::__construct($context, (array) $project);
    $this->data = array(
      'name' => strtr($context, array('@project_' => '')),
      'project' => $project,
    );
  }

  function filename() {
    return drush_server_home() . '/.drush/project_aliases/' . $this->data['name'] . '.aliases.drushrc.php';
  }
}
