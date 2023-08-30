<?php

namespace DevShop\Command;

use DevShop\Console\Command;
use Symfony\Component\Console\Input\ArrayInput;
use Symfony\Component\Console\Input\InputArgument;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Input\InputOption;
use Symfony\Component\Console\Output\OutputInterface;
use Symfony\Component\Console\Question\ConfirmationQuestion;
use Symfony\Component\Console\Question\Question;
use Symfony\Component\Process\Process;

class DevmasterUpgrade extends Command
{
  protected function configure()
  {
    $this
      ->setName('devmaster:upgrade')
      ->setDescription('Upgrade Devmaster: the Drupal front end. This command should only be run by the Ansible playbooks, which is triggered by the `devshop verify:system` command.')
      ->addArgument(
        'devshop-version',
        InputArgument::OPTIONAL,
        'The git tag or branch to install.'
      )
      ->addOption(
        'git_root', NULL, InputOption::VALUE_OPTIONAL,
        'Path to git repository root.',
        '/usr/share/devshop'
      )
      ->addOption(
        'git_remote', NULL, InputOption::VALUE_OPTIONAL,
        'The git remote to use to build the DevShop Control platform. Only needed if you have a custom DevShop web UI. By default, the main devshop repo is used.',
        'https://github.com/opendevshop/devshop.git'
      )
//      ->addOption(
//        'git_reference', NULL, InputOption::VALUE_OPTIONAL,
//        'Git reference to use.',
//      )
      ->addOption(
        'git_docroot', NULL, InputOption::VALUE_OPTIONAL,
        'Path to document root exposed to web server.',
        'src/DevShop/Control/web'
      )
      // Option to allow ansible runs to tell this command that it is being run as part of the playbooks.
      ->addOption(
        'run-from-playbooks', NULL, InputOption::VALUE_NONE,
        'A flag to indicate this command is being run from the playbooks. Dont use if manually running this command.'
      )

    ;
  }

  protected function execute(InputInterface $input, OutputInterface $output)
  {

    // Attaches input and output to the Command class.
    parent::execute($input, $output);

    $helper = $this->getHelper('question');

    // Announce ourselves.
    $output->writeln($this->getApplication()->getLogo());
    $this->announce('Devmaster Upgrade');

    $output->writeln(
      '<info>Welcome to the DevShop Devmaster Upgrader!</info>'
    );

    if ($input->getOption('run-from-playbooks') == FALSE) {
      $output->writeln('<error>This command should not be run manually unless you have a very specific reason. Please run the `devshop verify:system` command instead.</error>');
    }

    // @TODO: Check the CLI for new releases.  If, we should tell the user to run "self-update" then "upgrade".

    // Check for existing devshop install.
    // Look for aegir user
    $users = file_get_contents('/etc/passwd');
    if (strpos($users, 'aegir') === FALSE) {
      throw new \Exception('aegir user does not exist! DevShop is not installed! Installation aborted.');
    }
    $output->writeln('');

    // Check current user is root
    $pwu_data = posix_getpwuid(posix_geteuid());
    if ($pwu_data['name'] != 'root' && $pwu_data['name'] != 'aegir') {
      throw new \Exception('You must run this command as the root or aegir user. Run "sudo devshop upgrade" to run as root. 
       Devmaster upgrade aborted.');
    }
    $output->writeln('');
    $current_version = $this->getApplication()->getVersion();

    // Look for an active devmaster
    $devmaster_version = $this->getApplication()->getDevmasterVersion();
    $devmaster_uri = $this->getApplication()->getDevmasterUri();
    $devmaster_root = $this->getApplication()->getDevmasterRoot();
    if (empty($devmaster_root) || !file_exists($devmaster_root)) {
      throw new \Exception('Unable to find a devmaster.  The drush @hostmaster alias does not exist.  Unable to upgrade.');
    }

    // Lookup latest version.
    if ($input->getOption('run-from-playbooks')) {
      $target_version = $input->getArgument('devshop-version');
      $this->IO->note("Skipping GitHub version verification because --run-from-playbooks option was specified.");
    }
    else {
      $output->writeln('Checking for latest releases...');
      $client = new \Github\Client();
      $release = $client->repositories()->releases()->latest('opendevshop', 'devshop');

      // Make sure we got the release info
      if (empty($release)) {
        $output->writeln("<fg=red>Unable to retrieve releases from GitHub.  Try again later, or specify a release.</>");
        $latest_release = '0.x';
        $output->writeln("Assuming development version <info>$latest_release</info>.");
      }
      else {
        $latest_release = $release['tag_name'];
        $output->writeln("<info>Latest Version</info> $latest_release");
      }

      $default_version = $input->getArgument('devshop-version')? $input->getArgument('devshop-version'): $latest_release;
      $target_version = '';

      // Warn if default is not latest
      if ($latest_release != $default_version) {
        $output->writeln("<fg=red>WARNING:</> You have specified a release that is not the latest.");
      }

      // Confirm version
      while ($this->checkVersion($target_version) == FALSE) {
        $question = new Question("Target Version: (Default: $default_version) ", $default_version);
        $target_version = $helper->ask($input, $output, $question);

        if (!$this->checkVersion($target_version)) {
          $output->writeln("<fg=red>Version $target_version not found</>");
        }
      }
      $this->IO->note("Version $target_version confirmed.");
    }

    $this->IO->success("Upgrading DevShop to version $target_version...");

    // Leaving here for posterity. Wow.
    // $devmaster_makefile = $input->getOption('makefile');
    // if (empty($devmaster_makefile)) {
    //  $devmaster_makefile = "https://raw.githubusercontent.com/opendevshop/devshop/$target_version/build-devmaster.make";
    // }
    //

//    // Determine the target path.
//    $target_path = "/var/aegir/devmaster-{$target_version}";
//
//    // Check for existing path.  If exists, append the date.
//    $variant = date('Y-m-d');
//    if (file_exists($target_path) && $this->targetVersionRef == 'branch') {
//      $target_path = "/var/aegir/devmaster-{$target_version}-{$variant}";
//    }
//    elseif (file_exists($target_path) && $this->targetVersionRef == 'tag') {
//      $output->writeln("<comment>Version $target_version is already installed and is a tag. Nothing to do.</comment>");
//      return;
//    }
//
//    // If this path exists, add a number until we find one that doesn't exist.
//    if (file_exists($target_path)) {
//      $number = 1;
//      while (file_exists($target_path . '-' . $number)) {
//        $output->writeln("File exists at " . $target_path . '-' . $number);
//        $number++;
//      }
//      $output->writeln("File DOES NOT exists at " . $target_path . '-' . $number);
//      $target_path = $target_path . '-' . $number;
//    }

    $target_path = $this->input->getOption('git_root');
    $target_repo = $this->input->getOption('git_remote');

    $output->writeln('');

    $output->writeln('UPGRADE OPTIONS');
    $output->writeln("<info>Current CLI Version: </info>       $current_version");
    $output->writeln("<info>Current DevMaster Version: </info> $devmaster_version");
    $output->writeln("<info>Current DevMaster Path: </info>    $devmaster_root");
    $output->writeln("<info>Current DevMaster Site: </info>    $devmaster_uri");
    $output->writeln('');

    $output->writeln("<info>Target Version: </info> " . $target_version);
    $output->writeln("<info>Target DevShop Control Path: </info> " . $target_path);
    $output->writeln("<info>Target DevShop Control Git Remote: </info> " . $target_repo);

    $output->writeln('');
    $output->writeln("Hello, dedicated DevShop User!");
    $output->writeln("Thank you so much for staying with us through all of this.");
    $output->writeln("This upgrade process was always so tough. Aegir & Hostmaster made it really really difficult.");
    $output->writeln("If you are seeing this, you have already setup the latest DevShop.");
    $output->writeln("");
    $output->writeln("That means you no longer need this command. If you are using stock devshop, the front-end (devmaster) is now the same codebase as the backend.");
    $output->writeln("To upgrade from now on, just run a deploy task on the hostmaster site node itself.");
    $output->writeln("");
    $output->writeln("However... you won't ever need to do that because this is the last version of devshop that will use aegir for a backend.");

    $output->writeln("So, this is really goodbye. This generation of DevShop is now minimally supported.");
    $output->writeln("The next version of DevShop, which won't exist for a while, will be built on the Operations project.");
    $output->writeln("See https://www.drupal.org/project/operations for more information.");

    //@TODO: Finalize the upgrade process.
    // Aegir's process is split between 'upgrade.sh.txt' and a drush command "hostmaster-migrate"

    // 0. Update composer.
    // 1. Update drush  (postponed until we figure out how to leverage composer for this.)
    // 2. Download updated drush components
    // 3. Git checkout /usr/share/devshop to get the latest release.
    // 4. Run `drush hostmaster-migrate $HOSTNAME $PLATFORM_PATH --makefile=$MAKEFILE_PATH.
    // 5. Hope for the best.
    // 6. Run "install.sh" as root (should be renamed) to run the ansible playbook on the server.

    // 3. Git checkout /usr/share/devshop to get the latest release.

  }
}