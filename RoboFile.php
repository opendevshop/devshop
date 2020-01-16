<?php

require_once 'vendor/autoload.php';

use Symfony\Component\Yaml\Yaml;
use Symfony\Component\Console\Output\OutputInterface;
use Symfony\Component\Console\Exception\RuntimeException;

/**
 * This file provides commands to the robo CLI for managing development and
 * testing of devshop.
 *   1. Install robo CLI: http://robo.li/
 *   2. Clone this repo and change into the directory.
 *   3. Run `robo` to see the commands.
 *   4. If you have drush, docker, and docker compose, you can launch a devshop
 * with `robo up`
 *
 * Available commands:
 *
 *   destroy             Destroy all containers, docker volumes, and aegir
 * configuration. help                Displays help for a command launch
 *       Launch devshop after running prep:host and prep:source. Use --build to
 * build new local containers. list                Lists commands login
 *       Get a one-time login link to Devamster. logs                Stream
 * logs from the containers using docker-compose logs -f shell
 * Enter a bash shell in the devmaster container. stop                Stop
 * devshop containers using docker-compose stop test                Run all
 * devshop tests on the containers. up                  Launch devshop
 * containers using docker-compose up and follow logs. prepare
 * prepare:containers  Build aegir and devshop containers from the Dockerfiles. Detects your UID or you can pass as an argument. prepare:host        Check for docker, docker-compose and drush. Install them if they are missing. prepare:sourcecode  Clone all needed source code and build devmaster from the makefile.
 *
 * @see http://robo.li/
 */
class RoboFile extends \Robo\Tasks {

  // Install this version first when testing upgrades.
  const UPGRADE_FROM_VERSION = '1.0.0-rc4-testing';
  const UPGRADE_FROM_PROVISION_VERSION = '7.x-3.10';

  // The version of docker-compose to suggest the user install.
  const DOCKER_COMPOSE_VERSION = '1.10.0';

  // Defines where devmaster is installed.  'aegir-home/devmaster-$DEVSHOP_LOCAL_VERSION'
  const DEVSHOP_LOCAL_VERSION = '1.x';

  // Defines the URI we will use for the devmaster site.
  const DEVSHOP_LOCAL_URI = 'devshop.local.computer';

  // DevShop application user name.
  protected $devshopInstall = "ansible-playbook /usr/share/devshop/docker/playbook.server.yml --tags install-devmaster --extra-vars \"devmaster_skip_install=false\"";
  protected $devshopUsername = "aegir";

  /**
   * @var int Ansible verbosity. Passed from robo verbosity.
   */
  protected $ansibleVerbosity = 0;

  use \Robo\Common\IO;

  /**
   * @var The path to devshop root. Used for upgrades.
   */
  private $devshop_root_path;


  /**
   * Pass robo -v to Ansible -v.
   */
  private function setVerbosity() {
    switch ($this->output->getVerbosity()) {
      case OutputInterface::VERBOSITY_VERBOSE:
        $this->ansibleVerbosity = 1;
        break;
      case OutputInterface::VERBOSITY_VERY_VERBOSE:
        $this->ansibleVerbosity = 2;
        break;
      case OutputInterface::VERBOSITY_DEBUG:
        $this->ansibleVerbosity = 3;
        break;
      default:
        $this->ansibleVerbosity = 0;
        break;
    }
  }

  public function  __construct()
  {
    $this->git_ref = trim(str_replace('refs/heads/', '', shell_exec("git describe --tags --exact-match 2> /dev/null || git symbolic-ref -q HEAD 2> /dev/null")));

    if (empty($this->git_ref) && !empty($_SERVER['GITHUB_REF'])) {
      $this->git_ref = $_SERVER['GITHUB_REF'];
    }

    // Tell Provision power process to print output directly.
    putenv('PROVISION_PROCESS_OUTPUT=direct');
  }

//
//  /**
//   * Launch devshop after running prep:host and prep:source. Use --build to
//   * build new local containers.
//   *
//   * If you only run one command, run this one.
//   */
//  public function launch($opts = ['build' => 0]) {
//    $this->prepareHost();
//    $this->prepareSourcecode();
//
//    if ($opts['build']) {
//      $this->prepareContainers();
//    }
//
//    $this->up(['follow' => TRUE]);
//  }

  /**
   * Check for docker, docker-compose and drush. Install them if they are
   * missing.
   */
  public function prepareHost() {
    // Check for docker
    $this->say('Checking for Docker...');
    if ($this->taskExec('docker info')
      ->printOutput(FALSE)
      ->run()
      ->wasSuccessful()) {
      $this->_exec('docker -v');
      $this->say('Docker detected.');
    }
    else {
      $this->say('Could not run docker command. Find instructons for installing at https://www.docker.com/products/docker');
      throw new RuntimeException('Unable to continue.');
    }

    // Check for docker-compose
    $this->say('Checking for docker-compose...');
    if ($this->_exec('docker-compose -v')->wasSuccessful()) {
      $this->say('docker-compose detected.');
    }
    else {
      $this->yell('Could not run docker-compose command.', 40, 'red');
      $this->say("Run the following command as root to install it or see https://docs.docker.com/compose/install/ for more information.");

      $this->say('curl -L "https://github.com/docker/compose/releases/download/' . self::DOCKER_COMPOSE_VERSION . '/docker-compose-$(uname -s)-$(uname -m)" -o /usr/local/bin/docker-compose && chmod +x /usr/local/bin/docker-compose');
      throw new RuntimeException('Unable to continue.');
    }
  }

  private $repos = [
    'provision' => 'http://git.drupal.org/project/provision.git',
    'aegir-home/.drush/commands/registry_rebuild' => 'http://git.drupal.org/project/registry_rebuild.git',
    'documentation' => 'http://github.com/opendevshop/documentation.git',
  ];

  /**
   * Clone all needed source code and build devmaster from the makefile.
   *
   * @option no-dev Use build-devmaster.make instead of the development makefile.
   * @option devshop-version The directory to put the
   */
  public function prepareSourcecode($opts = [
    'no-dev' => FALSE,
    'devshop-version' => '1.x',
    'test-upgrade' => FALSE
  ]) {

    if (empty($this->git_ref)) {
      parent::yell("Preparing Sourcecode: Branch Unknown.");
    }
    else {
      parent::yell("Preparing Sourcecode: Branch $this->git_ref");
    }

    if ($opts['devshop-version'] == NULL) {
      $opts['devshop-version'] = $this->git_ref;
    }
    $this->devshop_root_path = __DIR__;

    // Create the Aegir Home directory.
    if (file_exists($this->devshop_root_path . "/aegir-home/.drush/commands")) {
      $this->say($this->devshop_root_path . "/aegir-home/.drush/commands already exists.");
    }
    else {
      $this->taskExecStack()
        ->exec("mkdir -p {$this->devshop_root_path}/aegir-home/.drush/commands")
        ->run();
    }

    // Clone all git repositories.
    foreach ($this->repos as $path => $url) {
      if (file_exists($this->devshop_root_path . '/' . $path)) {
        $this->say("$path already exists.");
      }
      else {
        $this->taskGitStack()
          ->cloneRepo($url, $this->devshop_root_path . '/' . $path)
          ->run();
      }

      // Checkout provision to the 7.x-3.x-devshop branch.
      if ($path == 'provision') {
        $this->taskGitStack()
          ->dir($this->devshop_root_path . '/' . $path)
          ->checkout('7.x-3.x-devshop')
          ->run();
      }
    }

    // Run drush make to build the devmaster stack.
    $make_destination = $this->devshop_root_path . "/aegir-home/devmaster-" . $opts['devshop-version'];
    $makefile_path = $opts['no-dev']? 'build-devmaster.make': "build-devmaster-dev.make.yml";

    // Append the desired devshop root path.
    $makefile_path = $this->devshop_root_path . '/' . $makefile_path;

    if (file_exists($make_destination)) {
      $this->say("Path {$make_destination} already exists.");
    }
    else {

      $this->yell("Building devmaster from makefile $makefile_path to $make_destination");

      $result = $this->_exec("bin/drush make {$makefile_path} {$make_destination} --working-copy --no-gitinfofile");
      if (!$result->wasSuccessful()) {
        throw new \RuntimeException("Drush make failed with the exit code " . $result->getExitCode());
      }
    }

    // Set git remote urls
    if ($opts['no-dev'] == FALSE) {
      $devshop_ssh_git_url = "git@github.com:opendevshop/devshop.git";

      if ($this->taskExec("git remote set-url origin $devshop_ssh_git_url")->run()->wasSuccessful()) {
        $this->yell("Set devshop git remote 'origin' to $devshop_ssh_git_url!");
      }
      else {
        $this->say("<comment>Unable to set devshop git remote to $devshop_ssh_git_url !</comment>");
      }

//      if ($this->taskExec("cd {$make_destination}/profiles/devmaster && git remote set-url origin $devmaster_ssh_git_url && git remote set-url origin --add $devmaster_drupal_git_url")->run()->wasSuccessful()) {
//        $this->yell("Set devmaster git remote 'origin' to $devmaster_ssh_git_url and added remote drupal!");
//      }RuntimeException
//      else {
//        $this->say("<comment>Unable to set devmaster git remote to $devmaster_ssh_git_url !</comment>");
//      }

//      // Check for drupal remote
//      if ($this->taskExec("cd {$make_destination}/profiles/devmaster && git remote get-url drupal")->run()->wasSuccessful()) {
//        $this->say('Git remote "drupal" already exists in devmaster.');
//      }
//      // If remote does not exist, add it.
//      elseif ($this->taskExec("cd {$make_destination}/profiles/devmaster && git remote add drupal $devmaster_drupal_git_url")->run()->wasSuccessful()) {
//        $this->yell("Added 'drupal' git remote and added git.drupal.org as a second push target on origin!");
//      }
//      else {
//        $this->say("<comment>Unable to add 'drupal' git remote and add git.drupal.org as a second push target on origin!</comment>");
//      }
    }
  }

  /**
   * Build devshop containers.
   *
   * By default, `robo prepare:containers` will build a new container image
   * using the 'Dockerfile' using FROM 'devshop/server:latest'. This shortens
   * build times because the image was pre-built on docker hub.
   *
   * To force a new local build of the 'devshop/server' container image from
   * scratch, use the '--from' option to specify a full docker image string or
   * the `--os` option to use a  `geerlingguy/docker-*-ansible` image.
   *
   * For example:
   *
   *   robo up --os centos7
   *
   * will build the container from geerlingguy/docker-centos7-ansible.
   *
   * @example bin/robo prepare:containers
   *
   * @param $user_uid Pass a UID to build the image with. Defaults to the UID of the user running `robo`
   *
   * @option $tag The string to tag the resulting container with.
   * @option $from The image to use to build the docker image FROM. Ignored if "os" is set.
   * @option $dockerfile The dockerfile to use.
   * @option $os An OS "slug" for any of the geerlingguy/docker-*-ansible images: https://hub.docker.com/u/geerlingguy/
   * @option $vars Ansible vars to pass to --extra-vars option.
   * @option $tags Ansible tags to pass to --tags option.
   * @option $skip_tags Ansible tags to pass to --skip-tags option.
   * @option $playbook Ansible tags to pass to ansible-playbook command.
   */
  public function prepareContainers($user_uid = NULL, $hostname = 'devshop.local.computer', $opts = [
      'tag' => 'local',
      'from' => 'devshop/server:latest',
      'dockerfile' => 'Dockerfile',
      'os' => '',
      'vars' => '',
      'tags' => '',
      'skip-tags' => '',
      'playbook' => 'roles/server.playbook.yml',
      'compose-file' => 'docker-compose.yml'
  ]) {

    $this->setVerbosity();

    // Environment variables at build time: AKA Build Args.
    $env_build = array();

    // Determine current UID.
    if (is_null($user_uid)) {
      $env_build['DEVSHOP_USER_UID'] = trim(shell_exec('id -u'));
    }

    // Set FROM_IMAGE. If os is set, generate the name.
    // If os is the default, set FROM to latest.
    if (!empty($opts['os'])) {
      $opts['from'] = "geerlingguy/docker-{$opts['os']}-ansible";
    }

    // Append the absolute path in the container.
    $opts['playbook'] = '/usr/share/devshop/' . $opts['playbook'] ;

    $this->yell('Building DevShop Container from: ' . $opts['from'], 40, 'blue');

    // @TODO: Document how  ENV vars in the process running `robo` are passed to
    // the `docker-compose build` command, which, if they are listed in docker-compose.yml,
    // will get passed into the containers.

    $env_build['DEVSHOP_DOCKER_TAG'] = $opts['tag'];

    // Set FROM using --from option.
    // @TODO: Tell users FROM _IMAGE env var doesn't work for prepare:containers?
    $env_build['FROM_IMAGE'] = $opts['from'];
    $env_build['ANSIBLE_CONFIG'] = '/usr/share/devshop/ansible.cfg';
    $env_build['ANSIBLE_VERBOSITY'] = $this->ansibleVerbosity;
    $env_build['ANSIBLE_EXTRA_VARS'] = $opts['vars'];
    $env_build['ANSIBLE_TAGS'] = $opts['tags'];
    $env_build['ANSIBLE_SKIP_TAGS'] = $opts['skip-tags'];

    // Pass `robo --playbook` option to Dockerfile.
    $env_build['ANSIBLE_PLAYBOOK'] = $opts['playbook'];
    $env_build['COMPOSE_FILE'] = $opts['compose-file'];

    $this->say("Custom Build Environment: " . print_r($env_build, 1));

    $provision_io = new \ProvisionOps\Tools\Style($this->input(), $this->output());
    $process = new \ProvisionOps\Tools\PowerProcess('docker-compose build --pull --no-cache', $provision_io);
    $process->setEnv($env_build);
    $process->disableOutput();
    $process->setTimeout(null);
    $process->setTty(!empty($_SERVER['XDG_SESSION_TYPE']) && $_SERVER['XDG_SESSION_TYPE'] == 'tty');
    $process->mustRun();

  }

  /**
   * Launch devshop in a variety of ways. Useful for local development and CI
   * testing.
   *
   * Builds a container to match the local user to allow write permissions to
   * Aegir Home.
   *
   * Examples:
   *
   *   robo up
   *   Launch a devshop in containers using docker-compose.
   *
   *   robo up --test
   *   Launch then test a devshop in a single process.
   *
   *   robo up --test
   *   Launch, upgrade, then test a devshop in a single process.
   *
   *   robo up --mode=install.sh --test
   *   Launch an OS container, then install devshop using install.sh, then run
   * tests.
   *
   *   robo up --mode=manual
   *   Just launch the container. Allows you to manually run the install.sh script.
   *
   * @option $test Run tests after containers are up and devshop is installed.
   * @option $test-upgrade Install an old version, upgrade it to this version,
   *   then run tests.
   * @option $mode Set to 'install.sh' to use the install.sh script for setup.
   * @option $user-uid Override the detected current user's UID when building
   *   containers.
   * @option $xdebug Set this option to launch with an xdebug container.
   * @option no-dev Use build-devmaster.make instead of the development makefile.
   * @option $build Run `robo prepare:containers` to rebuild the container first.
   * @option os-version An OS "slug" for any of the geerlingguy/docker-*-ansible images: https://hub.docker.com/u/geerlingguy/
   */
  public function up($opts = [
    'follow' => 1,
    'test' => FALSE,
    'test-upgrade' => FALSE,

    // Set 'mode' => 'install.sh' to run a traditional OS install.
    'mode' => 'docker-compose',
    'user-uid' => NULL,
    'disable-xdebug' => TRUE,
    'no-dev' => FALSE,
    'devshop-version' => '1.x',
    'build' => FALSE,
    'skip-source-prep' => FALSE,
    'skip-install' => FALSE,
    'os' => '',
    'from' => 'devshop/server:latest',
    'vars' => '',
    'tags' => '',
    'skip-tags' => '',
    'file' => 'Dockerfile',
    'playbook' => 'roles/server.playbook.yml',
    'compose-file' => 'docker-compose.yml'
  ]) {

    // Check for tools
    $this->prepareHost();
    $this->setVerbosity();

    if (empty($this->devshop_root_path)) {
      $this->devshop_root_path = __DIR__;
    }

    if (empty($this->git_ref)) {
      parent::yell("Launching DevShop: Branch Unknown.");
    }
    else {
      parent::yell("Launching DevShop: Branch $this->git_ref");
    }

    if ($opts['devshop-version'] == NULL) {
      $opts['devshop-version'] = $this->git_ref;
    }

    // Determine current UID.
    if (is_null($opts['user-uid'])) {
      $opts['user-uid'] = trim(shell_exec('id -u'));
    }

    // Build the container if desired.
    if ($opts['build']) {
      // @TODO: Make the playbook a CLI option and figure out a better way to do this.
      // $playbook = (!empty($opts['test']) || !empty($opts['test-upgrade']))? 'playbook.testing.yml': 'docker/playbook.server.yml';
      $playbook = $opts['playbook'];
      $this->say("Preparing containers with playbook: $playbook");
      $docker_tag = $opts['tag'] = 'local';
      $this->prepareContainers($opts['user-uid'], 'devshop.local.computer', $opts);
    }
    else {
      // If the --build option was not specified, pull the containers first.
      // If we don't, `docker-compose up` will BUILD and tag the image.
      $this->say("Pulling containers before docker-compose up...");
      $cmd[] = "docker-compose pull --quiet";
      $docker_tag = 'latest';
    }

    if ($opts['mode'] == 'docker-compose') {

      if ($opts['test'] || $opts['test-upgrade']) {
        $this->yell("Test Environment Requested: Using docker-compose-tests.yml.", 40, 'cyan');
        $this->say("No docker volumes are enabled using this mode.");

        if (!$opts['build']) {
          $this->say("The --build option was not specified: The latest code may not be in the container.");
        }

        $compose_file = 'docker-compose-tests.yml';
      }
      else {
        $this->yell("Local Development Environment Requested: Using {$opts['compose-file']}", 40, 'cyan');
        $this->say('Volumes will be mounted for:');
        $this->say(' - ' . __DIR__ . '/aegir-home to /var/aegir');
        $this->say(' - ' . __DIR__ . '/devmaster to /var/aegir/devmaster-1.x/profiles/devmaster');

        $compose_file = $opts['compose-file'];

        if (!file_exists('aegir-home/.drush') && !$opts['skip-source-prep']) {
          $this->prepareSourcecode($opts);
        }
        elseif ($opts['skip-source-prep']) {
          $this->say("Source code prep skipped because --skip-source-prep option was used.");
        }
        elseif (file_exists('aegir-home/.drush')) {
          $this->say("Source code prep skipped because 'aegir-home/.drush' folder already exists.");
        }
      }

      $cmd[] = 'echo "Running docker-compose up with COMPOSE_FILE=$COMPOSE_FILE"... ';
      $cmd[] = "docker-compose up --detach";

      // Start mysqld. Not sure why it's not kicking on.
      $cmd[] = "sleep 3";
      $cmd[] = "docker-compose exec -T devshop service mysql start";
      $cmd[] = "docker-compose exec -T devshop systemctl status --no-pager";
      $cmd[] = "docker-compose exec -T devshop ls -la";

      // Run final playbook to install devshop.
      // Test commands must be run as application user.
      // The `--test` command is run in GitHub Actions.
      if ($opts['test']) {

        // Do not run a playbook on docker-compose up, because it will launch as a separate process and we won't know when it ends.
        // @TODO: If we had the hostmaster-wait script, we would not use this. The tests could run only once devshop is installed.
        $opts['tags'] = "skip-all";
        $cmd[]= "docker-compose exec -T devshop service supervisord stop";
        $cmd[]= "docker-compose exec -T devshop site-wait @hostmaster";

        $command = "/usr/share/devshop/tests/devshop-tests.sh";
        $cmd[]= "docker-compose exec -T --user $this->devshopUsername devshop $command";
      }
      // @TODO: The `--test-upgrade` command is NOT YET run in GitHub Actions.
      // The PR with the update hook can be used to finalize upgrade tests: https://github.com/opendevshop/devshop/pull/426
      elseif ($opts['test-upgrade']) {
        $opts['tags'] = "skip-all";
        $cmd[]= "docker-compose exec -T devshop service supervisord stop";
        $cmd[]= "docker-compose exec -T devshop site-wait @hostmaster";

        $command = "/usr/share/devshop/tests/devshop-tests-upgrade.sh";
        $cmd[]= "docker-compose exec -T --user $this->devshopUsername devshop $command";
      }
      else {

        $cmd[]= "docker-compose exec -T devshop env";

        // This is run if neither --test or --test-upgrade commands are run.
        // We assume this means launch a development environment.
        if (!$opts['skip-install']) {
          $opts['tags'] = 'skip-install';
        }

        $cmd[] = "docker-compose exec -T devshop devshop status";
        $cmd[] = "docker-compose exec -T devshop devshop login";

        if ($opts['follow']) {
          $cmd[] = "docker-compose logs";
        }
      }

      //Environment variables at run time: AKA Environment variables.
      $env_run = [];
      $env_run['DEVSHOP_DOCKER_TAG'] = $docker_tag;
      $env_run['ANSIBLE_CONFIG'] = '/usr/share/devshop/ansible.cfg';
      $env_run['COMPOSE_FILE'] = $compose_file;
      $env_run['ANSIBLE_VERBOSITY_RUNTIME'] = $this->ansibleVerbosity;
      $env_run['ANSIBLE_TAGS_RUNTIME'] = $opts['tags'];
      $env_run['ANSIBLE_SKIP_TAGS_RUNTIME'] = $opts['skip-tags'];
      $env_run['ANSIBLE_PLAYBOOK_RUNTIME'] = '/usr/share/devshop/' . $opts['playbook'];
      $env_run['ANSIBLE_ROLES_PATH'] = '/usr/share/devshop/roles';

      $this->say("Custom Environment: " . print_r($env_run, 1));

      if (!empty($cmd)) {
        foreach ($cmd as $command) {
          $provision_io = new \ProvisionOps\Tools\Style($this->input, $this->output);
          $process = new \ProvisionOps\Tools\PowerProcess($command, $provision_io);
          $process->setEnv($env_run);
          $isTty = !empty($_SERVER['XDG_SESSION_TYPE']) && $_SERVER['XDG_SESSION_TYPE'] == 'tty';
          $process->setTty($isTty);
          $process->setTimeout(NULL);
          $process->disableOutput();
          $process->mustRun();
        }
        return;
      }
    }
    // @TODO: Leaving here until the "Upgrade Test" is migrated to the new pattern.
//    elseif ($opts['mode'] == 'install.sh' || $opts['mode'] == 'manual') {
//
//      $init_map = [
//        'centos:7' => '/usr/lib/systemd/systemd',
//        'ubuntu:14.04' => '/sbin/init',
//        'geerlingguy/docker-ubuntu1404-ansible' => '/sbin/init',
//        'geerlingguy/docker-ubuntu1604-ansible' => '/lib/systemd/systemd',
//        'geerlingguy/docker-ubuntu1804-ansible' => '/lib/systemd/systemd',
//        'geerlingguy/docker-centos7-ansible' => '/usr/lib/systemd/systemd',
//      ];
//
//      $init = isset($init_map[$opts['install-sh-image']])? $init_map[$opts['install-sh-image']]: '/sbin/init';
//
//      # This is the list of test sites, set in .travis.yml.
//      # This is so requests to these sites go back to localhost.
//      if (empty($_SERVER['SITE_HOSTS'])) {
//        $_SERVER['SITE_HOSTS'] = 'devshop.local.computer';
//      }
//
//      # Launch Server container
//      if (!$this->taskDockerRun($opts['install-sh-image'])
//        ->name('devshop_container')
//        ->volume($this->devshop_root_path, '/usr/share/devshop')
//        ->volume($this->devshop_root_path . '/aegir-home', '/var/aegir')
//        ->volume($this->devshop_root_path . '/roles', '/etc/ansible/roles')
//        ->volume($this->devshop_root_path . '/provision', '/var/aegir/.drush/commands/provision')
//        ->option('--hostname', 'devshop.local.computer')
//        ->option('--add-host', '"' . $_SERVER['SITE_HOSTS'] . '":127.0.0.1')
//        ->option('--volume', '/sys/fs/cgroup:/sys/fs/cgroup:ro')
//        ->option('-t')
//        ->publish(80,80)
//        ->detached()
//        ->privileged()
//        ->env('COMPOSE_FILE', 'docker-compose-tests.yml')
//        ->env('GITHUB_TOKEN', $_SERVER['GITHUB_TOKEN']?: '')
//        ->env('TERM', 'xterm')
//        ->env('GITHUB_REF', $_SERVER['GITHUB_REF'])
//        ->env('AEGIR_USER_UID', $opts['user-uid'])
//        ->env('PATH', "/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/usr/share/devshop/bin")
//        ->exec('/usr/share/devshop/tests/run-tests.sh')
//        ->exec($init)
//        ->run()
//        ->wasSuccessful()) {
//        throw new RuntimeException('Docker Run failed.');
//      }
//
//      # Install mysql first to ensure it is started.
//      if ($opts['install-sh-image'] == 'ubuntu:14.04') {
//        if (!$this->taskDockerExec('devshop_container')
//          ->exec("sed -i 's/101/0/' /usr/sbin/policy-rc.d")
//          ->run()
//          ->wasSuccessful()
//        ) {
//          throw new RuntimeException('Set init policy failed.');
//        }
//      }
//      elseif ($opts['install-sh-image'] == 'geerlingguy/docker-ubuntu1604-ansible') {
//// @TODO: If this is the cause of wonkiness, let's not install dbus just for testing. There are better ways to set hostname.
//        // Hostname install fails without dbus, so I am told: https://github.com/ansible/ansible/issues/25543
////        if (!(
////          $this->taskDockerExec('devshop_container')
////            ->exec("apt-get update")
////            ->run()
////            ->wasSuccessful()
////          && $this->taskDockerExec('devshop_container')
////            ->exec("apt-get install dbus -y")
////            ->env('DEBIAN_FRONTEND', 'noninteractive')
////            ->run()
////            ->wasSuccessful()
////
////          // @TODO: Hack attempt to fix failing apache restarts: https://travis-ci.org/opendevshop/devshop/jobs/608769926#L2447
////          // Idea from: https://unix.stackexchange.com/questions/239489/dbus-system-failed-to-activate-service-org-freedesktop-login1-timed-out
////          && $this->taskDockerExec('devshop_container')
////            ->exec("systemctl restart systemd-logind")
////            ->run()
////            ->wasSuccessful()
////        )) {
////          $this->say('Unable to install dbus. Setting hostname wont work. See https://github.com/ansible/ansible/issues/25543');
////
////          exit(1);
////        }
//      }
//
//      // Display home folder.
//      $this->taskDockerExec('devshop_container')
//        ->exec('ls -la /var/aegir')
//        ->run();
//
//      // Try to set ownership of home folder to AEGIR_UID.
//      $this->taskDockerExec('devshop_container')
//        ->exec("chown {$opts['user-uid']} /var/aegir -R")
//        ->run();
//
//      # If test-upgrade requested, install older version first, then run devshop upgrade $VERSION
//      if ($opts['test-upgrade']) {
//
////        // This is needed because the old playbook has an incompatibility with newer ansible.
//        // UPDATE: Seems to be not needed now?? This was triggering sh: 1: cannot create /root/.ansible.cfg: Permission denied
////        $this->taskDockerExec('devshop_container')
////          ->exec('echo "invalid_task_attribute_failed = false" >> /root/.ansible.cfg')
////          ->run();
//
//        // get geerlingguy.git role, it's not in the old release but it needs to be there because the aegir-apache role has it listed as a dependency.
//        $this->taskDockerExec('devshop_container')
//          ->exec('ansible-galaxy install geerlingguy.git geerlingguy.apache')
//          ->run();
//
//        $this->yell("Running install.sh for old version...");
//
//        // Run install.sh old version.
//        $version = self::UPGRADE_FROM_VERSION;
//        $this->_exec("curl -fsSL https://raw.githubusercontent.com/opendevshop/devshop/{$version}/install.sh -o {$this->devshop_root_path}/install.{$version}.sh");
//
//        // Set makefile and devshop install path options because they need to be different than the defaults for upgrading.
//        $install_path = "/usr/share/devshop-{$version}";
//        $makefile_filename = $opts['no-dev']? 'build-devmaster.make': "build-devmaster-dev.make.yml";
//
//        $opts['install-sh-options'] .= " --makefile=https://raw.githubusercontent.com/opendevshop/devshop/{$version}/{$makefile_filename}" ;
//        $opts['install-sh-options'] .= " --install-path={$install_path}";
//        $opts['install-sh-options'] .= " --force-ansible-role-install";
//
//        if (!empty($opts['user-uid'])) {
//          $opts['install-sh-options'] .= " --aegir-uid={$opts['user-uid']}";
//        }
//
//        if (!$this->taskDockerExec('devshop_container')
//          ->exec("bash /usr/share/devshop/install.{$version}.sh " . $opts['install-sh-options'])
//          ->run()
//          ->wasSuccessful()) {
//          throw new RunException("Installation of devshop $version failed.");
//        };
//
//        // Run devshop upgrade. This command runs:
//        $this->yell("Running devshop upgrade...");
//        //  - self-update, which checks out the branch being tested and installs the roles.
//        //  - verify:system, which runs the playbook with those roles, along with a devmaster:upgrade
//        $upgrade_to_branch = !empty($_SERVER['GITHUB_REF'])? $_SERVER['GITHUB_REF']: '1.x';
//        $upgrade_command = '/usr/share/devshop/bin/devshop upgrade -n ' . $upgrade_to_branch;
//        if (!$this->taskDockerExec('devshop_container')
//          ->exec($upgrade_command)
//          ->run()
//          ->wasSuccessful()) {
//          throw new RuntimeException("Command $upgrade_command failed.");
//        };
//
//        if (!$this->taskDockerExec('devshop_container')
//          ->exec('/usr/share/devshop/bin/devshop status')
//          ->run()
//          ->wasSuccessful()) {
//          throw new RuntimeException("Command 'devshop status' failed.");
//        };
//      }
//      else {
//        # Run install script on the container.
//        $this->yell("Running install.sh ...");
//        $install_command = '/usr/share/devshop/install.sh ' . $opts['install-sh-options'];
//        if ($opts['mode'] != 'manual' && ($this->input()
//              ->getOption('no-interaction') || $this->confirm('Run install.sh script?')) && !$this->taskDockerExec('devshop_container')
//            ->exec($install_command)
//            //        ->option('tty')
//            ->run()
//            ->wasSuccessful()) {
//          throw new RuntimeException('Docker Exec install.sh failed.');
//        }
//      }
//
//      if ($opts['test']) {
//
//        $this->yell("Running devshop-tests.sh ...");
//
//        # Run test script on the container.
//        if (!$this->taskDockerExec('devshop_container')
//          ->exec('su - aegir -c  - /usr/share/devshop/tests/devshop-tests.sh')
//          ->run()
//          ->wasSuccessful()
//        ) {
//          throw new RuntimeException('Docker Exec devshop-tests.sh failed.');
//        }
//      }
//    }
  }

  /**
   * Stop devshop containers using docker-compose stop
   */
  public function stop() {
    $this->_exec('docker-compose stop');
  }

  /**
   * Destroy all containers, docker volumes, and aegir configuration.
   *
   * Running with --no-interaction will keep the drupal devmaster codebase in
   * place.
   *
   * Running with --force
   */
  public function destroy($opts = ['force' => 0]) {

    if ($opts['no-interaction'] || $this->confirm("Destroy docker containers and volumes?")) {
      $this->_exec('docker-compose kill');
      $this->_exec('docker-compose rm -fv');
    }

    $version = self::DEVSHOP_LOCAL_VERSION;
    $uri = self::DEVSHOP_LOCAL_URI;

    if (!$opts['force'] && (!$opts['no-interaction'] && !$this->confirm("Destroy entire aegir-home folder? (If answered 'n', devmaster root will be saved.)"))) {
      if ($this->confirm("Destroy local config, drush aliases, and projects?")) {

        // Remove devmaster site folder
        $this->_exec("sudo rm -rf aegir-home/.drush");
        $this->_exec("sudo rm -rf aegir-home/config");
        $this->_exec("sudo rm -rf aegir-home/clients");
        $this->_exec("sudo rm -rf aegir-home/projects");
        $this->_exec("sudo rm -rf aegir-home/devmaster-{$version}/sites/{$uri}");
        $this->_exec("sudo rm -rf aegir-home/devmaster-1.0.0-beta10/sites/{$uri}");

        $this->say("Deleted local folders. Source code is still in place.");
        $this->say("To launch a new instance, run `robo up`");
      }
      else {
        $this->yell('Unable to delete local folders! Remove manually to fully destroy your local install.');
      }
    }
    else {
      if ($this->_exec("sudo rm -rf aegir-home")->wasSuccessful()) {
        $this->say("Entire aegir-home folder deleted.");
      }
      else {
        $this->yell("Unable to delete aegir-home folder, even with sudo!");
      }
    }
  }

  /**
   * Stream logs from the containers using docker-compose logs -f
   */
  public function logs() {
    $this->_exec('docker-compose logs -f');
  }

  /**
   * Stream watchdog logs from drupal
   */
  public function watchdog() {
    $user = 'aegir';
    $this->_exec("docker-compose exec --user $user -T devshop drush @hostmaster wd-show --tail --extended");
  }

  /**
   * Restart the containers.
   */
  public function restart() {
      $this->_exec('docker-compose restart');
      $this->logs();
  }

  /**
   * Enter a bash shell in the devmaster container.
   */
  public function shell($user = 'aegir') {

    if ($user) {
        $process = new \Symfony\Component\Process\Process("docker-compose exec --user $user devshop bash");
    }
    else {
        $process = new \Symfony\Component\Process\Process("docker-compose exec devshop bash");
    }
    $process->setTty(TRUE);
    $process->setTimeout(NULL);
    $process->run();
  }

  /**
   * Run all devshop tests on the containers.
   */
  public function test($user = 'aegir') {
    $is_tty = !empty($_SERVER['XDG_SESSION_TYPE']) && $_SERVER['XDG_SESSION_TYPE'] == 'tty';
    $no_tty = !$is_tty? '-T': '';
    $command = "docker-compose exec $no_tty --user $user devshop /usr/share/devshop/tests/devshop-tests.sh";
    $provision_io = new \ProvisionOps\Tools\Style($this->input, $this->output);
    $process = new \ProvisionOps\Tools\PowerProcess($command, $provision_io);

    $process->setTty(!empty($_SERVER['XDG_SESSION_TYPE']) && $_SERVER['XDG_SESSION_TYPE'] == 'tty');

    $process->setTimeout(NULL);
    $process->disableOutput();
    $process->mustRun();
  }

  /**
   * Get a one-time login link to Devamster.
   */
  public function login($user = 'aegir') {
    $this->_exec("docker-compose exec --user $user -T devshop drush @hostmaster uli");
  }

  /**
   * Create a new release of DevShop.
   */
  public function release($version = NULL, $drupal_org_version = NULL) {

    if (empty($version)) {
      // @TODO Verify version string.
      $version = $this->ask('What is the new version? ');
    }
    $version = trim($version);
    if (!$this->confirm("Are you sure you want the version number to be $version?")) {
      $this->release();
      return;
    }

    if (empty($drupal_org_version)) {
      $drupal_org_version = $this->ask("What should the Drupal.org version be? (Do not include 7.x or the second dot of the semantic version. ie 1.00-rc1 for 1.0.0-rc1)");
    }

    if (empty($drupal_org_version)) {
      $this->release($version);
      return;
    }

    if (!$this->confirm("Are you sure you want the Drupal.org tag to be 7.x-$drupal_org_version?")) {
      $this->release();
      return;
    }

    $drupal_org_tag = "7.x-$drupal_org_version";

    $this->yell("The new version shall be $version!!!");
    $release_branch = "release-{$version}";

    $not_ready = TRUE;
    while ($not_ready) {
      $not_ready = !$this->confirm("Are you absolutely sure all contrib modules and drupal core are up to date in ./aegir-home/devmaster-1.x/profiles/devmaster/devmaster.make?? Go check. I'll wait.");
    }

    $not_ready = TRUE;
    while ($not_ready) {
      $not_ready = !$this->confirm("Did you write a great CHANGELOG.md? Please make sure all (good) changes are included on the main branch (1.x) before continuing!");
    }

    if ($this->confirm("Create the branch $release_branch?")) {
        $this->_exec("git checkout -b $release_branch");
    }

    // Write version to files.
    if ($this->confirm("Write '$version' to ./devmaster/VERSION.txt")) {
      file_put_contents('./devmaster/VERSION.txt', $version);
    }

    if ($this->confirm("Write '$version' to install.sh? ")) {
      $this->_exec("sed -i -e 's/DEVSHOP_VERSION=1.x/DEVSHOP_VERSION=$version/' ./install.sh");
    }

    if ($this->confirm("Write '$drupal_org_version' to build-devmaster.make and remove development repos? ")) {
      $this->_exec("sed -i -e 's/projects\[devmaster\]\[version\] = 1.x-dev/projects[devmaster][version] = $drupal_org_version/' build-devmaster.make");
      $this->_exec("sed -i -e 's/projects\[devmaster\]\[download\]\[branch\]/; projects[devmaster][download][branch]' build-devmaster.make");
      $this->_exec("sed -i -e 's/projects\[devmaster\]\[download\]\[url\]/; projects[devmaster][download][url]' build-devmaster.make");
      $this->_exec("sed -i -e '/###DEVELOPMENTSTART###/,/###DEVELOPMENTEND###/d' build-devmaster.make");
    }

    if ($this->confirm("Show git diff before committing?")) {
      $this->_exec("git diff -U1");
    }
    if ($this->confirm("Commit changes to $release_branch? ")) {
      $this->_exec("git commit -am 'Automated Commit from DevShop Robofile.php `release` command. Preparing version $version.'");
    }

    if ($this->confirm("Create a release tag? ")) {

      // Tag devshop and devmaster with $version and drupal_org_tag.
      $this->taskGitStack()
        ->tag($version, "DevShop $version")
        ->tag($drupal_org_tag, "DevShop Devmaster $version")
        ->run();
    }

    if ($this->confirm("Push the new release tags $version and $drupal_org_version?")) {
      if (!$this->taskGitStack()
        ->push("origin", $version)
        ->push("origin", $drupal_org_tag)
        ->run()
        ->wasSuccessful()
      ) {
        $this->say('Pushing tags to remote origin');
      }
    }

    $this->say("The final steps we still have to do manually:");
    $this->say("1. Go create a new release of devmaster: https://www.drupal.org/node/add/project-release/1779370 using the tag $drupal_org_tag");
    $this->say("2. Wait for drupal.org to package up the distribution: https://www.drupal.org/project/devmaster");
    $this->say("3. Create a new 'release' on GitHub: https://github.com/opendevshop/devshop/releases/new using tag $version.  Copy CHANGELOG from  https://raw.githubusercontent.com/opendevshop/devshop/1.x/CHANGELOG.md");
    $this->say("  - Copy CHANGELOG from  https://raw.githubusercontent.com/opendevshop/devshop/1.x/CHANGELOG.md");
    $this->say("  - Upload install.sh script to release files.");
    $this->say("4. Put the new version in gh-pages branch index.html");

    if ($this->confirm("Checkout main branch and run `monorepo-builder split` to push current branch and latest tag to all sub-repos?")) {
      $this->_exec("git checkout 1.x");
      $this->_exec("bin/monorepo-builder split");
    }
  }

  /**
   * Run the molecule test command.
   */
  function moleculeTest() {
    $this->_exec("cd roles/opendevshop.devmaster && molecule test");
  }

  function moleculeConverge() {
    $this->_exec("cd roles/opendevshop.devmaster && molecule converge");
  }

}
