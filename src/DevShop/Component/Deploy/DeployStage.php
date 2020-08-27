<?php

namespace DevShop\Component\Deploy;

use DevShop\Component\Common\GitRepository;
use DevShop\Component\Common\GitRepositoryAwareTrait;

class DeployStage implements DeployStageInterface {

    use GitRepositoryAwareTrait;

    /**
     * @var string The stage name.
     */
    public $name = 'none';

    /**
     * @var string The stage command
     */
    private $command = null;

    /**
     * DeployStage constructor.
     *
     * @param $name
     * @param null $command
     * @param \DevShop\Component\Common\GitRepository|null $repository
     */
    public function __construct($name, $command = null, GitRepository $repository = NULL)
    {
        $this->name = $name;
        $this->command = $command?: $this->command;
        $this->setRepository($repository);
    }

    /**
     * Run the command for this stage.
     * @TODO: Use Process component or ProcessAwareTrait.
     */
    public function runStage() {
        $pwd = getenv("PWD");
        chdir($this->getRepository()->getRepositoryPath());
        print shell_exec($this->command);
        chdir($pwd);
    }

    /**
     * @return string
     */
    public function getCommand(){
        return $this->command;
    }
}