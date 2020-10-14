<?php

namespace DevShop\Component\Deploy;

use DevShop\Component\Common\GitRepository;
use DevShop\Component\Common\GitRepositoryAwareTrait;

interface DeployStageInterface {

    /**
     * @return string The command to run for this stage.
     */
    public function getCommand();
}