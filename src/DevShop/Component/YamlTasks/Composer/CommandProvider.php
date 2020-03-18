<?php

namespace DevShop\Component\YamlTasks\Composer;

use DevShop\Component\YamlTasks\Command\YamlTasksComposerCommand;
use Composer\Plugin\Capability\CommandProvider as CommandProviderCapability;

class CommandProvider implements CommandProviderCapability
{
    public function getCommands()
    {
        return array(new YamlTasksComposerCommand);
    }
}
