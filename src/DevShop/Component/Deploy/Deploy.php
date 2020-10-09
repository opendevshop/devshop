<?php
/**
 * @file Deploy.php
 *
 * Represents a single deploy action.
 */

namespace DevShop\Component\Deploy;

use DevShop\Component\Common\GitRepository;
use DevShop\Component\Common\GitRepositoryAwareTrait;
use DevShop\Component\DeployStageGit;

class Deploy {

    use GitRepositoryAwareTrait;

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
     * @TODO: replace with protected access and addStage() method.
     */
    public $stages = [];

    /**
     * @var DeployStage[] The stages to run during this deploy.
     * @TODO: replace with protected access and addStage() method.
     */
    public $options = [];

    /**
     * Deploy constructor.
     *
     * @param $stages
     * @param \DevShop\Component\Common\GitRepository|null $repository
     */
    function __construct($stages = null, GitRepository $repository = NULL) {
        $this->stages = $stages;
        $this->setRepository($repository);
    }

    /**
     * @param $name
     * @param null $value
     *
     * @return mixed|null
     */
    public function setOption($name, $value = null) {
        return $this->options[$name] = $value;
    }

    /**
     * @param $name
     * @param null $value
     *
     * @return string
     */
    public function getOption($name, $value = null) {
        return isset($this->options[$name])
          ? $this->options[$name]
          : $value;
    }

    /**
     * Run the stages.
     * @TODO: Throw exceptions on fail.
     */
    public function runStages() {
        foreach ($this->stages as $stage) {
            // @TODO: Make IOAware
            $time = date(DATE_RFC2822);
            $path = $stage->getRepository()->getRepositoryPath();
            echo " -----------------------------------------------------------------------\n";
            echo " Deploy Stage: $stage->name  |  $time \n";

            echo " > $path \n";
            echo " > {$stage->getCommand()} \n";
            echo " -----------------------------------------------------------------------\n";
            $stage->runStage();
        }
    }
}