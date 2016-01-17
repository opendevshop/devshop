<?php

namespace DevShop\Command;

use DevShop\Console\Command;
use Symfony\Component\Config\Definition\Exception\Exception;
use Symfony\Component\Console\Input\InputArgument;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Input\InputOption;
use Symfony\Component\Console\Output\OutputInterface;
use Symfony\Component\Console\Question\Question;
use Symfony\Component\Console\Question\ChoiceQuestion;
use Symfony\Component\Console\Question\ConfirmationQuestion;

use Symfony\Component\Process\Process;
use Symfony\Component\Process\Exception\ProcessFailedException;
use Github\Client;

use Symfony\Component\Filesystem\Filesystem;
use Symfony\Component\Filesystem\Exception\IOExceptionInterface;

class InstallDevmaster extends Command
{
  protected function configure() {
    $this
      ->setName('install-devmaster')
      ->setDescription('Install the Devmaster front-end. This command is analogous to "drush hostmaster-install"')

      // aegir_db_host
      ->addOption(
        'aegir_db_host',
        NULL,
        InputOption::VALUE_OPTIONAL,
        'The desired database host.',
        'localhost'
      )

      // aegir_db_port
      ->addOption(
        'aegir_db_port',
        NULL,
        InputOption::VALUE_OPTIONAL,
        'The desired database port.',
        '3306'
      )
    ;
  }

  protected function execute(InputInterface $input, OutputInterface $output) {
    // Attaches input and output to the Command class.
    parent::execute($input, $output);

    // Validate the database
    $this->validateDatabase();

  }

  /**
   * Ensure the database cannot be accessed by anonymous users, as it will
   * otherwise fail later in the install, and thus be harder to recover from.
   */
  private function validateDatabase() {
    $command = sprintf('mysql -u intntnllyInvalid -h %s -P %s -e "SELECT VERSION()"', $this->input->getOption('aegir_db_host'), $this->input->getOption('aegir_db_port'));

    // Run the Mysql process to test the database.
    $process = new Process($command);

    try {
      $process->mustRun();
      $output = $process->getOutput();

      if (preg_match("/Access denied for user 'intntnllyInvalid'@'([^']*)'/", $output, $match)) {
        return TRUE;
      }
      elseif (preg_match("/Host '([^']*)' is not allowed to connect to/", $output, $match)) {
        return TRUE;
      }
      else {
        throw new Exception('Anonymous user was able to log into the database server. This is insecure, Devmaster install cannot continue.  Please run "mysql_secure_installation" or see https://dev.mysql.com/doc/refman/5.7/en/mysql-secure-installation.html for more information.');
      }

    } catch (ProcessFailedException $e) {
      throw new Exception($e->getMessage());
    }
  }
}