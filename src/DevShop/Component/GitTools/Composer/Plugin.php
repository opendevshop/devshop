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
        ScriptEvents::POST_INSTALL_CMD => array(
          array('onPostUpdateInstall', 1),
        ),
        ScriptEvents::POST_UPDATE_CMD => array(
          array('onPostUpdateInstall', 1),
        ),
      );
    }

    /**
     * Script callback; Acted on after install or update.
     */
    public function onPostUpdateInstall(Event $event) {
      $settings = [
        'targetDir' => 'vendor/splitsh/lite',
      ];
      $binDir = $event->getComposer()->getConfig()->get('bin-dir');
      $config = $event->getComposer()->getConfig()->get('devshop');
      $input = $event->getComposer()->getInput();

      // @TODO: Read repos list from composer.json config
      $repos = Splitter::REPOS;

      Splitter::installBins();
      Splitter::splitRepos($repos, $input->getOption('progress'));
    }

  public function getCapabilities()
    {
        return array(
            'Composer\Plugin\Capability\CommandProvider' => 'DevShop\Component\GitTools\Composer\CommandProvider',
        );
    }
}
