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

      // site
      ->addOption(
        'site', NULL, InputOption::VALUE_OPTIONAL,
        'The front-end URL to use for Devmaster.'
      )

      // aegir_host
      ->addOption(
        'aegir_host', NULL, InputOption::VALUE_OPTIONAL,
        'The aegir host. Will default to the detected hostname of this server.'
      )

      // script_user
      ->addOption(
        'script_user', NULL, InputOption::VALUE_OPTIONAL,
        'The user running this script.'
      )

      // aegir_db_host
      ->addOption(
        'aegir_db_host', NULL, InputOption::VALUE_OPTIONAL,
        'The database host.',
        'localhost'
      )

      // aegir_db_port
      ->addOption(
        'aegir_db_port', NULL, InputOption::VALUE_OPTIONAL,
        'The database server port.',
        '3306'
      )

      // aegir_db_user
      ->addOption(
        'aegir_db_user', NULL, InputOption::VALUE_OPTIONAL,
        'The database user, one that is allowed to CREATE new databases.',
        'root'
      )

      // aegir_db_pass
      ->addOption(
        'aegir_db_pass', NULL, InputOption::VALUE_OPTIONAL,
        'The database password for the "aegir_db_user"',
        'root'
      )

      // profile
      ->addOption(
        'profile', NULL, InputOption::VALUE_OPTIONAL,
        'The desired install profile.',
        'devmaster'
      )

      // makefile
      ->addOption(
        'makefile', NULL, InputOption::VALUE_OPTIONAL,
        'The makefile to use to build the platform.',
        'devmaster'
      )

      // aegir_root
      ->addOption(
        'aegir_root', NULL, InputOption::VALUE_OPTIONAL,
        'The home directory for the "aegir" user.  If not specified will be automatically detected.'
      )

      // root
      ->addOption(
        'root', NULL, InputOption::VALUE_OPTIONAL,
        'The desired path to install to.  Example: /var/aegir/devmaster-0.x'
      )

      // http_service_type
      ->addOption(
        'http_service_type', NULL, InputOption::VALUE_OPTIONAL,
        'The HTTP service to use: apache or nginx',
        'apache'
      )

      // http_port
      ->addOption(
        'http_port', NULL, InputOption::VALUE_OPTIONAL,
        'The port that the webserver should use.',
        '80'
      )

      // web_group
      ->addOption(
        'web_group', NULL, InputOption::VALUE_OPTIONAL,
        'The web server user group. If not specified, will be detected automatically.'
      )

      // client_name
      ->addOption(
        'client_name', NULL, InputOption::VALUE_OPTIONAL,
        'The name of the aegir "client".',
        'admin'
      )

      // client_email
      // If not specified, will use the aegir_host
      ->addOption(
        'client_email', NULL, InputOption::VALUE_OPTIONAL,
        'The email to use for the administrator user.'
      )



    ;
  }

  protected function execute(InputInterface $input, OutputInterface $output) {
    // Attaches input and output to the Command class.
    parent::execute($input, $output);

    // Validate the database.
    if ($this->validateSecureDatabase()) {
      $this->output->writeln('<info>Database is secure.</info>');
    }
    else {
      $this->output->writeln('<error>Database is NOT Secure. Run "mysql_secure_installation" or see https://dev.mysql.com/doc/refman/5.7/en/mysql-secure-installation.html for more information.</error>');
      return;
    }

    // Confirm all of the options.
    $this->validateOptions();
  }

  /**
   * Ensure the database cannot be accessed by anonymous users, as it will
   * otherwise fail later in the install, and thus be harder to recover from.
   */
  private function validateSecureDatabase() {
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
        return FALSE;
      }

    } catch (ProcessFailedException $e) {
      return TRUE;
    }
  }

  /**
   * Validate the users command line options.
   */
  private function validateOptions() {

    $options = $this->input->getOptions();

    foreach ($options as $option => $value) {
      $this->output->writeln("<info>{$option}:</info> {$value}");
    }
  }
}
