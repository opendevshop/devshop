<?php
/**
 * @file Deploy.php
 *
 * Represents a single deploy action.
 */

namespace DevShop\Component\Deploy;

class Deploy {

    const DEFAULT_STAGES = [
      'git',
      'build',
      'deploy',
    ];

    /**
     * @return string[] List of stages that will be run by default during a deploy.
     */
    public static function getDefaultStages(){
        return self::DEFAULT_STAGES;
    }

    /**
     * Get the list of stages that are run by default.
     *
     * @return string[] List of stages that run by default during a "deploy" command.
     */
    public static function isDefaultStage($stage_name){
        return in_array($stage_name, self::getDefaultStages());
    }

    /**
     * @var DeployStage[] The stages to run during this deploy.
     */
    private $stages = [];

    function __construct($stages) {
        $this->stages = $stages;
    }

    /**
     * Run the stages.
     * @TODO: Throw exceptions on fail.
     */
    public function runStages() {
        foreach ($this->stages as $stage) {
            // @TODO: Make IOAware
            $time = date(DATE_RFC2822);
            echo " -----------------------------------------------------------------------\n";
            echo " Deploy Stage: $stage->name $time \n";
            echo " > {$stage->getCommand()} \n";
            echo " -----------------------------------------------------------------------\n";
            $stage->runStage();
        }
    }
}