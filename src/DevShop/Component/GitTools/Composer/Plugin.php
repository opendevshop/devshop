<?php

/*
 * This file is part of the DevShop package.
 *
 * (c) Jon Pugh <jon@thinkdrop.net
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

namespace DevShop\Component\GitTools\Composer;

use Composer\Composer;
use Composer\EventDispatcher\Event;
use Composer\EventDispatcher\EventSubscriberInterface;
use Composer\IO\IOInterface;
use Composer\Plugin\Capable;
use Composer\Plugin\PluginInterface;
use Composer\Plugin\PluginEvents;
use Composer\Plugin\PreFileDownloadEvent;
use Composer\Script\ScriptEvents;

/**
 * Composer Plugin for DevShop Git Tools
 *
 * @author Jon Pugh <jon@thinkdrop.net>
 */
class Plugin implements PluginInterface, Capable, EventSubscriberInterface
{
    protected $composer;
    protected $io;
    
    public function activate(Composer $composer, IOInterface $io)
    {
        $this->composer = $composer;
        $this->io = $io;
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
            'Composer\Plugin\Capability\CommandProvider' => 'DevShop\Component\GitTools\Composer\CommandProvider',
        );
    }

    /**
     * @param Event $event
     */
    public function pluginDemoMethod(Event $event)
    {
    }
}
