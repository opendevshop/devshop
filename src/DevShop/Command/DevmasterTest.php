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
        '/var/aegir/devmaster-1.x'
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
    $behat_path = $input->getOption('behat-path');
    if (empty($behat_path)) {
      
      // Use environment variable if one exists.
      if (!empty($_SERVER['BEHAT_PATH'])) {
        $input->setOption('behat-path', $_SERVER['BEHAT_PATH']);
      }
      else {
        // If no behat path is set, use the devmaster repository root / tests.
        $input->setOption('behat-path', $input->getOption('root') . '/profiles/devmaster/tests');
      }
    }
    
    // Check that the behat path is valid.
    if (!file_exists($input->getOption('behat-path') . '/behat.yml')) {
      throw new \Exception('No behat.yml found at ' . $input->getOption('behat-path'));
    }
    else {
      $output->writeln("Running Behat tests located at " . $input->getOption('behat-path'));
    }
  }

  protected function execute(InputInterface $input, OutputInterface $output) {
    parent::execute($input, $output);

    $uri = $input->getOption('uri');
    $root = $input->getOption('root');

//    // @TODO: This is all to get these tests running on Drupal6 devmaster! should be able to remove this for drupal7
//    // Lookup password from @hostmaster alias
//    $output->writeln('Looking up hostmaster database credentials...');
//
//    $process = new Process('drush @hostmaster sql-conf --format=var_export --show-passwords');
//    $process->mustRun();
//    $db_var_export = $process->getOutput();
//    $db = (object) eval("return {$db_var_export};");
//
//    // Write to local.settings.php
//    $path = "{$root}/sites/{$uri}/local.settings.php";
//    $settings_default_path = "{$root}/sites/default/settings.php";
//    $output->writeln("Writing db credentials to $path...");
//
//    $db_url = "{$db->driver}://{$db->username}:{$db->password}@{$db->host}:{$db->port}/{$db->database}";
//
//    $output = <<<PHP
//<?php
//  \$databases['default']['default'] = $db_var_export;
//  \$db_url = "$db_url";
//
//PHP;
//    $fs = new Filesystem();
//    $fs->dumpFile($path, trim($output));
//    $fs->dumpFile($settings_default_path, trim($output));

    // Run composer install
    $process = new Process('composer install');
    $process->setTimeout(NULL);
    $process->setWorkingDirectory($input->getOption('behat-path'));

    $process->run(function ($type, $buffer) {
      if (Process::ERR === $type) {
        echo $buffer;
      } else {
        echo $buffer;
      }
    });

    // Run bin/behat
    $cmd = 'bin/behat --colors --format-settings=\'{"expand": true}\'';
    
    if ($input->getOption('name')) {
      $cmd .= ' --name=' . $input->getOption('name');
    }
    
    $process = new Process($cmd);
    $process->setTimeout(NULL);
    $process->setWorkingDirectory($input->getOption('behat-path'));
    $process->setEnv(array(
      'HOME' => '/var/aegir',
      'PATH' => getenv('PATH') . ':/usr/share/composer/vendor/bin/',
      'BEHAT_PARAMS' => json_encode(array(
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
      )
    )));

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
