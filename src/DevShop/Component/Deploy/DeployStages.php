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

    /**
     * Prepare DeployStages from:
     *   1. TBD
     *   2. Project's composer.json:extra.deploy.stages
     *
     * @param \TQ\Git\Repository\Repository $repository
     * @param \Eloquent\Composer\Configuration\Element\Configuration $configuration
     */
    static public function prepareStages(Repository $repository, Configuration $configuration) {

        // @TODO: Look up default DeployStages for common projects.
        if (empty($configuration->extra()->deploy)) {
            throw new \Exception("No 'extra.deploy' section found in composer.json in directory " . $repository->getRepositoryPath());
        }
        else {
            $deploy_extra_config = $configuration->extra()->deploy;
print_r($deploy_extra_config);
            // @TODO...


        }
    }
}