<?php

namespace DevShop\Component\YamlTasks\Composer;

use Composer\Composer;
use Composer\EventDispatcher\Event;
use Composer\EventDispatcher\EventSubscriberInterface;
use Composer\IO\IOInterface;
use Composer\Plugin\Capable;
use Composer\Plugin\PluginInterface;
use Composer\Plugin\PluginEvents;
use Composer\Plugin\PreFileDownloadEvent;
use Composer\Script\ScriptEvents;

class Plugin implements PluginInterface, Capable, EventSubscriberInterface
{
    protected $composer;
    protected $io;
    
    public function activate(Composer $composer, IOInterface $io)
    {
        $this->composer = $composer;
        $this->io = $io;


        //        print_r($this->composer->getPackage()->getConfig());
        //        die;
    }

    /**
     * @return array
     */
    public static function getSubscribedEvents()
    {
        return array(
            PluginEvents::INIT => 'pluginDemoMethod'
        );
    }

    public function getCapabilities()
    {
        return array(
            'Composer\Plugin\Capability\CommandProvider' => 'DevShop\Component\YamlTasks\Composer\CommandProvider',
        );
    }

    /**
     * @param Event $event
     */
    public function pluginDemoMethod(Event $event)
    {
    }
}
