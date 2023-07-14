<?php

namespace DevShop\Command;

use DevShop\Console\Command;

use Symfony\Component\Console\Input\InputArgument;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Input\InputOption;
use Symfony\Component\Console\Output\OutputInterface;

use Symfony\Component\Process\Process;

class Status extends Command
{
  protected function configure()
  {
    $this
      ->setName('status')
      ->setDescription('Check status')
    ;
  }

  protected function execute(InputInterface $input, OutputInterface $output)
  {

    // Attaches input and output to the Command class.
    parent::execute($input, $output);

    // Track if we found an error, so we can return an exit code.
    $error = FALSE;

    // Announce ourselves.
    $devshop_control_path = '/usr/share/devshop/src/DevShop/Control';
    $output->writeln($this->getApplication()->getLogo());
    $this->announce('Status');
    $this->checkCliVersion();
    $output->writeln(array(
      "",
      "Your contributions make DevShop possible. Please consider becoming a patron of open source!",
      "    https://opencollective.com/devshop",
      "    https://www.patreon.com/devshop ",
    ));

    // Check for DevShop CLI
    $output->write("<comment>Checking for DevShop CLI...  </comment>");
    $version = trim($this->getApplication()->getVersion());
    $output->write("<info>DevShop CLI is installed.</info>  ");
    $output->writeln($version);

    // Check for Drush
    $output->write("<comment>Checking for Drush in $devshop_control_path...  </comment>");

    $process = new Process('drush --version', $devshop_control_path);
    $process->run();
    if (!$process->isSuccessful()) {
      $output->writeln("<question>Drush not detected.</question>");
      $output->writeln($process->getErrorOutput());
      $error = TRUE;
    }
    else {
      $output->write("<info>Drush is installed.  </info>");
      $output->writeln(trim($process->getOutput()));
    }

    // Check for devmaster
    $output->write("<comment>Checking for DevMaster... in $devshop_control_path </comment>");
    if ($this->user == 'aegir') {
      $process = new Process('drush @hostmaster vget install_profile', $devshop_control_path);
    }
    else {
      $process = new Process('sudo su aegir -c "drush @hostmaster vget install_profile"', $devshop_control_path);
    }

    $process->run();
    $profile = trim($process->getOutput());
    if (empty($profile)) {
      $output->writeln("<error>Devmaster not installed.</error>");

      $output->writeln($process->getErrorOutput());
      $output->writeln('');
      $output->writeln("<error>Devmaster site not detected.</error>");
      $output->writeln('The command <comment>drush @hostmaster vget install_profile</comment> failed.');
      $error = TRUE;
    }
    else {
      $output->write("<info>Devmaster is installed.</info>  ");

      // Output drush status --fields=drupal-version,uri
      $output->writeln($this->getApplication()->getDevmasterVersion());

      $process = new Process('drush @hostmaster status --fields=drupal-version,uri,database', $devshop_control_path);
      $process->run();

      $output->writeln(preg_replace('!\s+!', ' ', $process->getOutput()));
      $output->writeln('');
    }

    $output->writeln('');

    // Check for deprecated .devshop-version file.
    if (file_exists('/var/aegir/.devshop-version')) {
      $output->writeln("<fg=red>A deprecated file was found. You should delete '/var/aegir/.devshop-version' to reduce confusion.</>");
    }

    // If an error was detected, return a non-zero exit code.
    if ($error) {
      exit(1);
    }
  }
}