<?php

namespace DevShop\Component\Deploy;

use Eloquent\Composer\Configuration\Element\Configuration;
use TQ\Git\Repository\Repository;

class DeployStages {

    const STAGES = [
        'git' => 'Get the source code.',
        'build' => 'Prepare the source code and services.',
        'install' => 'Install the application data.',
        'deploy' => 'Update application data after code changes.',
        'test' => 'Run a command to verify functionality of the site.'
    ];

    /**
     * Get the list of all Stages.
     * @return string[] List of stages with key being stage name, value the description.
     */
    public static function getStages(){
      return self::STAGES;
    }
}