<?php

namespace DevShop\Component\GitRemoteMonitor;

use Composer\Autoload\ClassLoader;
use Robo\Common\ConfigAwareTrait;
use Robo\Common\IO;
use Robo\Config\Config;
use Robo\Robo;
use Robo\Runner as RoboRunner;
use Symfony\Component\Console\Application as SymfonyApplication;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Output\OutputInterface;

class Application {

    const APPLICATION_NAME = 'Git Remote Monitor';
    const REPOSITORY = 'devshop-packages/git-remote-monitor';
    const CONFIG_PREFIX = 'GRM';
    const CONFIG_FILENAME = '.grm.yml';

    use ConfigAwareTrait;
    use IO;

    private $runner;

    public function __construct(
      InputInterface $input = NULL,
      OutputInterface $output = NULL,
      ClassLoader $classLoader = NULL
    ) {

        // Load Config
        $configFilePath =  getenv(self::CONFIG_PREFIX . '_CONFIG') ?: getenv('HOME') . '/' . self::CONFIG_FILENAME;
        $config = \Robo\Robo::createConfiguration([$configFilePath]);

        // Create applicaton.
        $this->setConfig($config);
        $application = new SymfonyApplication(self::APPLICATION_NAME, self::getVersion());

        // Determine what commands to load.
        $commandClasses[] = Commands::class;

        // Create and configure container.
        $container = Robo::createDefaultContainer($input, $output, $application, $config);

        // Instantiate Robo Runner.
        $this->runner = new RoboRunner($commandClasses);
        $this->runner
          ->setContainer($container)
          ->setClassLoader($classLoader)
          ->setSelfUpdateRepository(self::REPOSITORY)
          ->setConfigurationFilename($configFilePath)
          ->setEnvConfigPrefix(self::CONFIG_PREFIX)
        ;
    }

    /**
     * Wrapper for $this->runner->execute().
     *
     * @param \Symfony\Component\Console\Input\InputInterface $input
     * @param \Symfony\Component\Console\Output\OutputInterface $output
     *
     * @return int
     */
    public function execute($argv, OutputInterface $output) {
        return $this->runner->execute($argv, self::APPLICATION_NAME, self::getVersion(), $output);
    }

    /**
     * Load the version from the VERSION file.
     * @return string
     */
    static public function getVersion() {
        return trim(file_get_contents(dirname(__DIR__) . '/VERSION'));
    }

}