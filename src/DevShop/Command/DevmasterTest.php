<?php

namespace DevShop\Command;

use DevShop\Console\Command;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Input\InputOption;
use Symfony\Component\Console\Output\OutputInterface;
use Symfony\Component\Process\Process;

class DevmasterTest extends Command {
  protected function configure() {
    $this
      ->setName('devmaster:test')
      ->setDescription('Runs internal tests on this devmaster instance.')
      ->addOption(
        'uri',
        '',
        InputOption::VALUE_OPTIONAL,
        'The URI to use if @hostmaster alias is not found.',
        'devshop.site'
      )
      ->addOption(
        'root',
        '',
        InputOption::VALUE_OPTIONAL,
        'The root path to the site to test, if @hostmaster alias is not found.',
        '/var/aegir/devshop-control-1.x'
      )
      ->addOption(
        'name',
        '',
        InputOption::VALUE_OPTIONAL,
        'A test name to pass to bin/behat --name option'
      )
      ->addOption(
        'behat-path',
        '',
        InputOption::VALUE_OPTIONAL,
        'The absolute path to the tests to run. Defaults to DevShop CLI path.'
      )
    ;
  }

  protected function initialize(InputInterface $input, OutputInterface $output) {
    if (file_exists('/var/aegir/.drush/hostmaster.alias.drushrc.php')) {
      $aliases = array();
      include('/var/aegir/.drush/hostmaster.alias.drushrc.php');
      if (empty($aliases['hostmaster'])) {
        throw new \Exception('Hostmaster alias not found.');
      }
      $input->setOption('root', $aliases['hostmaster']['root']);
      $input->setOption('uri', $aliases['hostmaster']['uri']);
    }
    else {
      $output->writeln("<error>Hostmaster alias not found.</error>");
      exit(1);
    }
    
    // Check behat path.
    if (empty($input->getOption('behat-path'))) {
      
      // Use environment variable if one exists.
      if (!empty($_SERVER['BEHAT_PATH'])) {
        $input->setOption('behat-path', $_SERVER['BEHAT_PATH']);
      }
      else {
        // If no behat path is set, use the devmaster repository root / tests.
        $input->setOption('behat-path', realpath($input->getOption('root') . '/../tests'));
      }
    }
  }

  protected function execute(InputInterface $input, OutputInterface $output) {
    parent::execute($input, $output);

    $uri = $input->getOption('uri');
    $root = $input->getOption('root');

    // Show git info
    $process = new Process('git log -3');
    $process->setWorkingDirectory($input->getOption('behat-path'));
    $process->run();
    echo $process->getOutput() . $process->getErrorOutput() ;

    // Run bin/behat
    $cmd = '../bin/behat --colors --format-settings=\'{"expand": true}\'';
    
    if ($input->getOption('name')) {
      $cmd .= ' --name=' . $input->getOption('name');
    }

    // @TODO: require provision-ops/power-process
    $process = new Process($cmd);
    $process->setTimeout(NULL);
    $process->setWorkingDirectory($input->getOption('behat-path'));

    $env = $_SERVER;
    $env['BEHAT_PARAMS'] = json_encode(array(
      'extensions' => array(
        'Behat\\MinkExtension' => array(
          'base_url' => "http://{$uri}"
        ),
        'Drupal\\DrupalExtension' => array(
          'drush' => array(
            'root' => $root
          ),
          'drupal' => array(
            'drupal_root' => $root
          ),
        ),
      )
    ));

    $process->setEnv($env);

    $output->writeln(["Running $cmd with environment:"]);
    $output->writeln(var_export($env));

    $process->run(function ($type, $buffer) {
      if (Process::ERR === $type) {
        echo $buffer;
      } else {
        echo $buffer;
      }
    });

    // Delete the local.settings.php file.
//    $fs->remove($path);
//    $fs->remove($settings_default_path);

    // executes after the command finishes
    if (!$process->isSuccessful()) {
      exit(1);
    }


  }

}
