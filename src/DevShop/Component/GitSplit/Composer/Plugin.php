<?php

/*
 * This file is part of the DevShop package.
 *
 * (c) Jon Pugh <jon@thinkdrop.net
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

namespace DevShop\Component\GitSplit\Composer;

use DevShop\Component\GitSplit\Splitter;
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

  /**
   * @inheritDoc
   */
    public function activate(Composer $composer, IOInterface $io)
    {
        $this->composer = $composer;
        $this->io = $io;
    }

    /**
     * @inheritDoc
     */
    public function deactivate(Composer $composer, IOInterface $io)
    {
    }

    /**
     * @inheritDoc
     */
    public function uninstall(Composer $composer, IOInterface $io)
    {
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
      $git_tools_bin_dir = $event->getComposer()->getConfig()->get('bin-dir');
      $this->io->write("Installing <info>splitsh-lite</info> to $git_tools_bin_dir");
      Splitter::install($git_tools_bin_dir);
    }

  public function getCapabilities()
    {
        return array(
            'Composer\Plugin\Capability\CommandProvider' => 'DevShop\Component\GitSplit\Composer\CommandProvider',
        );
    }
}
