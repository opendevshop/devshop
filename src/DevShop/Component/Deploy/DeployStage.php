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
    protected $command = null;

    /**
     * @var \DevShop\Component\Deploy\Deploy The Deploy object this stage belongs to.
     */
    protected $deploy = null;

    /**
     * DeployStage constructor.
     *
     * @param string $name
     * @param string $command
     * @param \DevShop\Component\Common\GitRepository $repository
     * @param \DevShop\Component\Deploy\Deploy $deploy
     */
    public function __construct($name, $command, GitRepository $repository, Deploy $deploy)
    {
        $this->name = $name;
        $this->command = $command?: $this->command;
        $this->deploy = $deploy;
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