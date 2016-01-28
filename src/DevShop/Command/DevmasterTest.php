<?php

namespace DevShop\Command;

use DevShop\Console\Command;

use Github\Exception\RuntimeException;
use Symfony\Component\Console\Input\ArrayInput;
use Symfony\Component\Console\Input\InputArgument;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Input\InputOption;
use Symfony\Component\Console\Output\OutputInterface;
use Symfony\Component\Console\Question\Question;
use Symfony\Component\Console\Question\ChoiceQuestion;
use Symfony\Component\Console\Question\ConfirmationQuestion;

use Symfony\Component\Process\Process;
use Github\Client;

use Symfony\Component\Filesystem\Filesystem;
use Symfony\Component\Filesystem\Exception\IOExceptionInterface;
use Symfony\Component\Finder\Finder;

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
  }

  protected function execute(InputInterface $input, OutputInterface $output) {
    parent::execute($input, $output);

    $uri = $input->getOption('uri');
    $root = $input->getOption('root');

    // @TODO: This is all to get these tests running on Drupal6 devmaster! should be able to remove this for drupal7
    // Lookup password from @hostmaster alias
    $output->writeln('Looking up hostmaster database credentials...');

    $process = new Process('drush @hostmaster sql-conf --format=var_export --show-passwords');
    $process->mustRun();
    $db_var_export = $process->getOutput();
    $db = (object) eval("return {$db_var_export};");

    // Write to local.settings.php
    $path = "{$root}/sites/{$uri}/local.settings.php";
    $output->writeln("Writing db credentials to $path...");

      $output = '<?php  ';
    $db_url = "{$db->driver}://{$db->username}:{$db->password}@{$db->host}:{$db->port}/{$db->database}";

    $output .= <<<PHP
<?php
  \$databases['default']['default'] = $db_var_export;
  \$db_url = "$db_url";

PHP;
    $fs = new Filesystem();
    $fs->dumpFile($path, trim($output));

    // Run bin/behat
    $process = new Process('bin/behat --colors');
    $process->setTimeout(NULL);
    $process->setWorkingDirectory(__DIR__ . '/../../../tests');
    $process->setEnv(array('BEHAT_PARAMS' => json_encode(array(
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
        )
      )
    ))));

    $process->run(function ($type, $buffer) {
      if (Process::ERR === $type) {
        echo $buffer;
      } else {
        echo $buffer;
      }
    });

    // executes after the command finishes
    if (!$process->isSuccessful()) {
      exit(1);
    }


  }

}