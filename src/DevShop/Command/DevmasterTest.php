<?php

namespace DevShop\Command;

use DevShop\Console\Command;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Input\InputOption;
use Symfony\Component\Console\Output\OutputInterface;
use Symfony\Component\Process\Process;

class DevmasterTest extends Command {
  
  public $root;
  public $uri;
  
  protected function configure() {
    $this
      ->setName('devmaster:test')
      ->setDescription('Runs internal tests on this devmaster instance.')
      ->addOption(
        'name',
        '',
        InputOption::VALUE_OPTIONAL,
        'A test name to pass to bin/behat --name option'
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
      $this->root = $aliases['hostmaster']['root'];
      $this->uri = $aliases['hostmaster']['uri'];
    }
    else {
      $output->writeln("<error>Hostmaster alias not found.</error>");
      exit(1);
    }
  }

  protected function execute(InputInterface $input, OutputInterface $output) {
    parent::execute($input, $output);

    $uri = $this->uri;
    $root = $this->root;
    $tests_path = realpath("$root/../tests");

    // Show git info
    $process = new Process('git log -3');
    $process->setWorkingDirectory($tests_path);
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
    $process->setWorkingDirectory($tests_path);

    $env = $_SERVER;
    $env['HTTP_HOST'] = 'devshop.local.computer';
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

    $output->writeln(["Running $cmd in $tests_path with environment:"]);
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
