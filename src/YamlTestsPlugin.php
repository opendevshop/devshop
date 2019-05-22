<?php
/**
 * @file
 *
 * https://getcomposer.org/doc/articles/plugins.md#creating-a-plugin
 *
 */

namespace ProvisionOps\YamlTests;

use Composer\Composer;
use Composer\IO\IOInterface;
use Composer\Plugin\PluginInterface;

class YamlTestsPlugin implements PluginInterface
{
  public function activate( $composer, IOInterface $io)
  {
//    $installer = new TemplateInstaller($io, $composer);
//    $composer->getInstallationManager()->addInstaller($installer);
  }

  public function getCapabilities()
  {
    return array(
      'Composer\Plugin\Capability\CommandProvider' => 'YamlTestsCommandProvider',
    );
  }
}