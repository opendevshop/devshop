<?php

namespace DevShop\Component\Deploy;

class DeployStage {

    /**
     * @var string The stage name.
     */
    public $name = 'deploy';

    /**
     * @var string The stage command
     */
    private $command = 'echo "DeployStage command not set. Define DeployStage::command property."';

    public function __construct($name, $command = null)
    {
        $this->name = $name;
        $this->command = $command?: $this->command;
    }

    /**
     * Run the command for this stage.
     * @TODO: Use Process component or ProcessAwareTrait.
     */
    public function runStage() {
        print shell_exec($this->command);
    }

    /**
     * @return string
     */
    public function getCommand(){
        return $this->command;
    }
}