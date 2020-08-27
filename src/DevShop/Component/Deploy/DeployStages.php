<?php

namespace DevShop\Component\Deploy;

class DeployStages {

    const DEFAULT_STAGES = [
        'git' => 'Get the source code.',
        'build' => 'Prepare the source code and services.',
        'install' => 'Install the application data.',
        'deploy' => 'Update application data after code changes.',
        'test' => 'Run a command to verify functionality of the site.'
    ];

    /**
     * Get the list of default Stages.
     * @return string[] List of stages with key being stage name, value the description.
     */
    public static function defaultStages (){
      return self::DEFAULT_STAGES;
    }

}