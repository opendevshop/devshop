<?php

namespace DevShop\Component\Deploy;

class DeployStages {

    const STAGES = [
        'git' => 'Get the source code.',
        'build' => 'Prepare the source code and services.',
        'install' => 'Install the application data.',
        'deploy' => 'Update application data after code changes.',
        'test' => 'Run a command to verify functionality of the site.'
    ];

    const DEFAULT_DEPLOY = [
        'git',
        'build',
        'deploy',
    ];

    /**
     * Get the list of all Stages.
     * @return string[] List of stages with key being stage name, value the description.
     */
    public static function getStages(){
      return self::STAGES;
    }

    /**
     * Get the list of stages run by default.
     * @return string[] List of stages that will be run by default during a deploy.
     */
    public static function getDefaultDeploy(){
      return self::DEFAULT_DEPLOY;
    }

    /**
     * Get the list of stages that are run by default.
     *
     * @return string[] List of stages that run by default during a "deploy" command.
     */
    public static function isDefaultStage($stage_name){
      return in_array($stage_name, self::getDefaultDeploy());
    }
}