<?php

require_once 'vendor/autoload.php';

use Symfony\Component\Yaml\Yaml;

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

  /**
   * @var The path to devshop root. Used for upgrades.
   */
  private $devshop_root_path;

  public function  __construct()
  {
    $this->git_ref = trim(str_replace('refs/heads/', '', shell_exec("git describe --tags --exact-match 2> /dev/null || git symbolic-ref -q HEAD 2> /dev/null")));

    if (empty($this->git_ref) && !empty($_SERVER['TRAVIS_PULL_REQUEST_BRANCH'])) {
      $this->git_ref = $_SERVER['TRAVIS_PULL_REQUEST_BRANCH'];
    }
  }

  /**
   * Launch devshop after running prep:host and prep:source. Use --build to
   * build new local containers.
   *
   * If you only run one command, run this one.
   */
  public function launch($opts = ['build' => 0]) {
    $this->prepareHost();
    $this->prepareSourcecode();

    if ($opts['build']) {
      $this->prepareContainers();
    }

    $this->up(['follow' => TRUE]);
  }

  /**
   * Check for docker, docker-compose and drush. Install them if they are
   * missing.
   */
  public function prepareHost() {
    // Check for docker
    $this->say('Checking for Docker...');
    if ($this->taskDockerRun('hello-world')
      ->printed(FALSE)
      ->run()
      ->wasSuccessful()) {
      $this->_exec('docker -v');
      $this->say('Docker detected.');
    }
    else {
      $this->say('Could not run docker command. Find instructons for installing at https://www.docker.com/products/docker');
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
    }

    // Check for drush
    $this->say('Checking for drush...');
    if ($this->_exec('drush --version')->wasSuccessful()) {
      $this->say('drush detected.');
    }
    else {
      $this->yell('Could not run drush.', 40, 'red');
      $this->say("Run the following command as root to install it or see http://www.drush.org/en/master/install/ for more information.");

      $this->say('php -r "readfile(\'https://s3.amazonaws.com/files.drush.org/drush.phar\');" > /usr/local/bin/drush && chmod +x /usr/local/bin/drush');
    }
  }

  private $repos = [
    'provision' => 'http://git.drupal.org/project/provision.git',
    'aegir-home/.drush/commands/registry_rebuild' => 'http://git.drupal.org/project/registry_rebuild.git',
    'documentation' => 'http://github.com/opendevshop/documentation.git',
    'aegir-dockerfiles' => 'http://github.com/aegir-project/dockerfiles.git',
  ];

  /**
   * Clone all needed source code and build devmaster from the makefile.
   *
   * @option no-dev Use build-devmaster.make instead of the development makefile.
   */
  public function prepareSourcecode($opts = [
    'no-dev' => FALSE,
    'fork' => FALSE,
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
    }

    // @TODO: Detect and clone the right version. This will not be necessary in Ansible 2.6.
    // Ansible roles
    $role_repos = [
      'geerlingguy.apache' => 'http://github.com/geerlingguy/ansible-role-apache.git',
      'geerlingguy.composer' => 'http://github.com/geerlingguy/ansible-role-composer.git',
      'geerlingguy.git' => 'http://github.com/geerlingguy/ansible-role-git.git',
      'geerlingguy.mysql' => 'http://github.com/geerlingguy/ansible-role-mysql.git',
      'geerlingguy.nginx' => 'http://github.com/geerlingguy/ansible-role-nginx.git',
      'geerlingguy.php' => 'http://github.com/geerlingguy/ansible-role-php.git',
      'geerlingguy.php-mysql' => 'http://github.com/geerlingguy/ansible-role-php-mysql.git',
      'opendevshop.aegir-apache' => 'http://github.com/opendevshop/ansible-role-aegir-apache',
      'opendevshop.aegir-nginx' => 'http://github.com/opendevshop/ansible-role-aegir-nginx',
      'opendevshop.aegir-user' => 'http://github.com/opendevshop/ansible-role-aegir-user',
      'opendevshop.devmaster' => 'http://github.com/opendevshop/ansible-role-devmaster.git',
    ];

    $this->yell("Cloning Ansible Roles...");

    $roles_yml = Yaml::parse(file_get_contents($this->devshop_root_path . '/roles.yml'));
    foreach ($roles_yml as $role) {
      $roles[$role['name']] = [
        'repo' => $role_repos[$role['name']],
        'version' => $role['version'],
      ];
    }

    foreach ($roles as $name => $role) {
      $path = $this->devshop_root_path . '/roles/' . $name;
      if (file_exists($path)) {
        $this->say("$path already exists.");
      }
      else {
        $this->taskGitStack()
          ->cloneRepo($role['repo'], $path, $role['version'])
          ->run();
      }
    }


    // Run drush make to build the devmaster stack.
    $make_destination = $this->devshop_root_path . "/aegir-home/devmaster-" . $opts['devshop-version'];
    $makefile_path = $opts['no-dev']? 'build-devmaster.make': "build-devmaster-dev.make.yml";

    // If "fork" option is set, use travis forks makefile.
    $makefile_path = $opts['fork']? 'build-devmaster-travis-forks.make.yml': $makefile_path;

    // Append the desired devshop root path.
    $makefile_path = $this->devshop_root_path . '/' . $makefile_path;

    if (file_exists($make_destination)) {
      $this->say("Path {$make_destination} already exists.");
    }
    else {

      $this->yell("Building devmaster from makefile $makefile_path to $make_destination");

      $result = $this->_exec("bin/drush make {$makefile_path} {$make_destination} --working-copy --no-gitinfofile");
      if ($result->wasSuccessful()) {
        return TRUE;
      }
      else {
        $this->say("Drush make failed with the exit code " . $result->getExitCode());
        return FALSE;
      }
    }
  }

  /**
   * Build aegir and devshop containers from the Dockerfiles. Detects your UID
   * or you can pass as an argument.
   */
  public function prepareContainers($user_uid = NULL) {

    if (is_null($user_uid)) {
      $user_uid = trim(shell_exec('id -u'));
    }

    $this->say("Found UID $user_uid. Passing to docker build as a build-arg...");

    // aegir/hostmaster
    $this->taskDockerBuild('aegir-dockerfiles')
      ->option('file', 'aegir-dockerfiles/Dockerfile-php7')
      ->option('build-arg', "AEGIR_UID=$user_uid")
      ->tag('aegir/hostmaster:php7')
      ->run();

//      $this->taskDockerBuild('aegir-dockerfiles')
//        ->option('file', 'aegir-dockerfiles/Dockerfile-xdebug')
//        ->tag('aegir/hostmaster:xdebug')
//        ->run();

    // aegir/hostmaster:xdebug

      // @TODO: Put this dockerfile back
      //    $this->taskDockerBuild('aegir-dockerfiles')
//      ->option('file', 'aegir-dockerfiles/Dockerfile-xdebug-php7')
//      ->tag('aegir/hostmaster:xdebug')
//      ->run();
    //    // devshop/devmaster
    //    $this->taskDockerBuild('dockerfiles')
    //      ->option('file', 'dockerfiles/Dockerfile')
    //      ->tag('devshop/devmaster')
    //      ->run()
    //      ;
    //    // devshop/devmaster:xdebug
    //    $this->taskDockerBuild('dockerfiles')
    //      ->option('file', 'dockerfiles/Dockerfile-xdebug')
    //      ->tag('devshop/devmaster:xdebug')
    //      ->run()
    //      ;
    // aegir/web
//    $this->taskDockerBuild('aegir-dockerfiles')
//      ->option('file', 'aegir-dockerfiles/Dockerfile-web')
//      ->tag('aegir/web')
//      ->run();
//
//    $this->taskDockerBuild('aegir-dockerfiles')
//      ->option('file', 'aegir-dockerfiles/Dockerfile-privileged')
//      ->tag('aegir/hostmaster:privileged')
//      ->run();
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
   *   robo up --mode=install.sh
   * --install-sh-image=geerlingguy/docker-centos7-ansible Launch an OS
   * container, then install devshop using install.sh in a CentOS 7 image.
   *
   *   robo up --mode=manual
   *   Just launch the container. Allows you to manually run the install.sh script.
   *
   * @option $test Run tests after containers are up and devshop is installed.
   * @option $test-upgrade Install an old version, upgrade it to this version,
   *   then run tests.
   * @option $mode Set to 'install.sh' to use the install.sh script for setup.
   * @option $install-sh-image Enter any docker image to use for running the
   *   install-sh-image. Since we need ansible, we are using geerlingguy's
   *   geerlingguy/docker-centos7-ansible and
   *   geerlingguy/docker-ubuntu1404-ansible images.
   * @option $user-uid Override the detected current user's UID when building
   *   containers.
   * @option $xdebug Set this option to launch with an xdebug container.
   * @option no-dev Use build-devmaster.make instead of the development makefile.
   */
  public function up($opts = [
    'follow' => 1,
    'test' => FALSE,
    'test-upgrade' => FALSE,

    // Set 'mode' => 'install.sh' to run a traditional OS install.
    'mode' => 'docker-compose',
    'install-sh-image' => 'geerlingguy/docker-ubuntu1404-ansible',
    'install-sh-options' => '--server-webserver=apache',
    'user-uid' => NULL,
    'disable-xdebug' => TRUE,
    'no-dev' => FALSE,
    'fork' => FALSE,
    'devshop-version' => '1.x',
  ]) {

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

    if (!file_exists('aegir-home')) {
      if ($opts['no-interaction'] || $this->ask('aegir-home does not yet exist. Run "prepare:sourcecode" command?')) {
        if ($this->prepareSourcecode($opts) == FALSE) {
          $this->say('Prepare source code failed.');
          exit(1);
        }
      }
      else {
        $this->say('aegir-home must exist for devshop to work. Not running docker-compose up.');
        return;
      }
    }

    if ($opts['mode'] == 'docker-compose') {
      $env = "-e TERM=xterm";
      $env .= !empty($_SERVER['BEHAT_PATH'])? " -e BEHAT_PATH={$_SERVER['BEHAT_PATH']}": '';

      if ($opts['test']) {
        $command = "/usr/share/devshop/tests/devshop-tests.sh";
        $cmd = "docker-compose -f docker-compose-tests.yml run -T $env devmaster '$command'";
      }
      elseif ($opts['test-upgrade']) {
        $version = self::UPGRADE_FROM_VERSION;
        $provision_version = self::UPGRADE_FROM_PROVISION_VERSION;
        $command = "/usr/share/devshop/tests/devshop-tests-upgrade.sh";

        // @TODO: Have this detect the branch and use that for the version.
        $root_target = '/var/aegir/devmaster-' . $opts['devshop-version'];

        //      $cmd = "docker-compose run -e UPGRADE_FROM_VERSION={$version} -e UPGRADE_TO_MAKEFILE= -e AEGIR_HOSTMASTER_ROOT=/var/aegir/devmaster-{$version} -e AEGIR_VERSION={$version} -e AEGIR_MAKEFILE=https://raw.githubusercontent.com/opendevshop/devshop/{$version}/build-devmaster.make -e TRAVIS_BRANCH={$_SERVER['TRAVIS_BRANCH']}  -e TRAVIS_REPO_SLUG={$_SERVER['TRAVIS_REPO_SLUG']} -e TRAVIS_PULL_REQUEST_BRANCH={$_SERVER['TRAVIS_PULL_REQUEST_BRANCH']} devmaster 'run-tests.sh' ";

        // Launch a devmaster container as if it were the last release, then run hostmaster-migrate on it, then run the tests.
        $cmd = "docker-compose -f {$this->devshop_root_path}/docker-compose-tests.yml run $env -e UPGRADE_FROM_VERSION={$version} -e AEGIR_HOSTMASTER_ROOT=/var/aegir/devmaster-{$version} -e AEGIR_HOSTMASTER_ROOT_TARGET=$root_target -e AEGIR_VERSION={$version} -e AEGIR_MAKEFILE=https://raw.githubusercontent.com/opendevshop/devshop/{$version}/build-devmaster.make -e PROVISION_VERSION={$provision_version} devmaster '$command'";
      }
      else {
        $cmd = "docker-compose up -d";
        if ($opts['follow']) {
          $cmd .= "; docker-compose logs -f";
        }
      }

      // Build a local container.
      if ($opts['user-uid'] != '1000') {
        $dockerfile = $opts['disable-xdebug'] ? 'aegir-dockerfiles/Dockerfile-local' : 'aegir-dockerfiles/Dockerfile-local-xdebug';
        $this->taskDockerBuild($this->devshop_root_path . '/aegir-dockerfiles')
          ->option('file', $this->devshop_root_path . '/' . $dockerfile)
          ->tag('aegir/hostmaster:local')
          ->option('build-arg', "NEW_UID=" . $opts['user-uid'])
          ->option('no-cache')
          ->run();
      }


      if (isset($cmd)) {
        if ($this->_exec($cmd)->wasSuccessful()) {
          exit(0);
        }
        else {
          exit(1);
        }
      }
    }
    elseif ($opts['mode'] == 'install.sh' || $opts['mode'] == 'manual') {

      $init_map = [
        'centos:7' => '/usr/lib/systemd/systemd',
        'ubuntu:14.04' => '/sbin/init',
        'geerlingguy/docker-ubuntu1404-ansible' => '/sbin/init',
        'geerlingguy/docker-ubuntu1604-ansible' => '/lib/systemd/systemd',
        'geerlingguy/docker-centos7-ansible' => '/usr/lib/systemd/systemd',
      ];

      $init = isset($init_map[$opts['install-sh-image']])? $init_map[$opts['install-sh-image']]: '/sbin/init';

      # This is the list of test sites, set in .travis.yml.
      # This is so requests to these sites go back to localhost.
      if (empty($_SERVER['SITE_HOSTS'])) {
        $_SERVER['SITE_HOSTS'] = 'devshop.local.computer';
      }

      # Launch Server container
      if (!$this->taskDockerRun($opts['install-sh-image'])
        ->name('devshop_container')
        ->volume($this->devshop_root_path, '/usr/share/devshop')
        ->volume($this->devshop_root_path . '/aegir-home', '/var/aegir')
        ->volume($this->devshop_root_path . '/roles', '/etc/ansible/roles')
        ->option('--hostname', 'devshop.local.computer')
        ->option('--add-host', '"' . $_SERVER['SITE_HOSTS'] . '":127.0.0.1')
        ->option('--volume', '/sys/fs/cgroup:/sys/fs/cgroup:ro')
        ->option('-t')
        ->publish(80,80)
        ->detached()
        ->privileged()
        ->env('TERM', 'xterm')
        ->env('TRAVIS', TRUE)
        ->env('TRAVIS_BRANCH', $_SERVER['TRAVIS_BRANCH'])
        ->env('TRAVIS_REPO_SLUG', $_SERVER['TRAVIS_REPO_SLUG'])
        ->env('TRAVIS_PULL_REQUEST_BRANCH', $_SERVER['TRAVIS_PULL_REQUEST_BRANCH'])
        ->env('AEGIR_USER_UID', $opts['user-uid'])
        ->exec('/usr/share/devshop/tests/run-tests.sh')
        ->exec($init)
        ->run()
        ->wasSuccessful()) {
        $this->say('Docker Run failed.');
        exit(1);
      }

      # Install mysql first to ensure it is started.
      if ($opts['install-sh-image'] == 'ubuntu:14.04') {
        if (!$this->taskDockerExec('devshop_container')
          ->exec("sed -i 's/101/0/' /usr/sbin/policy-rc.d")
          ->run()
          ->wasSuccessful()
        ) {
          $this->say('Set init policy failed.');
          exit(1);
        }
      }
      elseif ($opts['install-sh-image'] == 'geerlingguy/docker-ubuntu1604-ansible') {
        // Hostname install fails without dbus, so I am told: https://github.com/ansible/ansible/issues/25543
        if (!(
          $this->taskDockerExec('devshop_container')
            ->exec("apt-get update")
            ->run()
            ->wasSuccessful() &&
          $this->taskDockerExec('devshop_container')
            ->exec("apt-get install dbus -y")
            ->env('DEBIAN_FRONTEND', 'noninteractive')
            ->run()
            ->wasSuccessful()
        )) {
          $this->say('Unable to install dbus. Setting hostname wont work. See https://github.com/ansible/ansible/issues/25543');
          exit(1);
        }
      }

      // Display home folder.
      $this->taskDockerExec('devshop_container')
        ->exec('ls -la /var/aegir')
        ->run();

      // Try to set ownership of home folder to AEGIR_UID.
      $this->taskDockerExec('devshop_container')
        ->exec("chown {$opts['user-uid']} /var/aegir -R")
        ->run();

      # If test-upgrade requested, install older version first, then run devshop upgrade $VERSION
      if ($opts['test-upgrade']) {

//        // This is needed because the old playbook has an incompatibility with newer ansible.
        // UPDATE: Seems to be not needed now?? This was triggering sh: 1: cannot create /root/.ansible.cfg: Permission denied
//        $this->taskDockerExec('devshop_container')
//          ->exec('echo "invalid_task_attribute_failed = false" >> /root/.ansible.cfg')
//          ->run();

        // get geerlingguy.git role, it's not in the old release but it needs to be there because the aegir-apache role has it listed as a dependency.
        $this->taskDockerExec('devshop_container')
          ->exec('ansible-galaxy install geerlingguy.git geerlingguy.apache')
          ->run();

        $this->yell("Running install.sh for old version...");

        // Run install.sh old version.
        $version = self::UPGRADE_FROM_VERSION;
        $this->_exec("wget https://raw.githubusercontent.com/opendevshop/devshop/{$version}/install.sh");
        $this->_exec('bash install.sh ' . $opts['install-sh-options']);

        $this->taskExec('composer install --no-plugins --no-scripts')
          ->dir($this->devshop_root_path);

//        $this->taskDockerExec('devshop_container')
//          ->exec('/usr/share/devshop/bin/devshop self-update -n ' . $_SERVER['TRAVIS_PULL_REQUEST_BRANCH'])
//          ->run();

        // Run devshop upgrade. This command runs:
        $this->yell("Running devshop upgrade...");
        //  - self-update, which checks out the branch being tested and installs the roles.
        //  - verify:system, which runs the playbook with those roles, along with a devmaster:upgrade
        $this->taskDockerExec('devshop_container')
          ->exec('/usr/share/devshop/bin/devshop upgrade -n ' . $_SERVER['TRAVIS_PULL_REQUEST_BRANCH'])
          ->run();

        $this->taskDockerExec('devshop_container')
          ->exec('/usr/share/devshop/bin/devshop status')
          ->run();
      }
      else {
        # Run install script on the container.
        $this->yell("Running install.sh ...");
        $install_command = '/usr/share/devshop/install.sh ' . $opts['install-sh-options'];
        if ($opts['mode'] != 'manual' && ($this->input()
              ->getOption('no-interaction') || $this->confirm('Run install.sh script?')) && !$this->taskDockerExec('devshop_container')
            ->exec($install_command)
            //        ->option('tty')
            ->run()
            ->wasSuccessful()) {
          $this->say('Docker Exec install.sh failed.');
          exit(1);
        }
      }

      if ($opts['test']) {

        # Disable supervisor
        if ($opts['install-sh-image'] == 'geerlingguy/docker-ubuntu1404-ansible') {
          $service = 'supervisor';
        }
        elseif ($opts['install-sh-image'] == 'geerlingguy/docker-ubuntu1604-ansible') {
          $service = FALSE;
        }
        else {
          $service = 'supervisord';
        }

        if ($service && !$this->taskDockerExec('devshop_container')
          ->exec("service $service stop")
          ->run()
          ->wasSuccessful()
        ) {
          $this->say('Unable to disable supervisor.');
          exit(1);
        }

        $this->yell("Running devshop-tests.sh ...");

        # Run test script on the container.
        if (!$this->taskDockerExec('devshop_container')
          ->exec('su - aegir -c  - /usr/share/devshop/tests/devshop-tests.sh')
          ->run()
          ->wasSuccessful()
        ) {
          $this->say('Docker Exec devshop-tests.sh failed.');
          exit(1);
        }
      }
    }
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
      $this->_exec('docker kill devshop_container');
      $this->_exec('docker rm -fv devshop_container');
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
   * Enter a bash shell in the devmaster container.
   */
  public function shell($user = NULL) {

    if ($user) {
      $process = new \Symfony\Component\Process\Process("docker-compose exec --user $user devmaster bash");
    }
    else {
      $process = new \Symfony\Component\Process\Process("docker-compose exec devmaster bash");
    }
    $process->setTty(TRUE);
    $process->run();
  }

  /**
   * Run all devshop tests on the containers.
   */
  public function test() {
    $process = new \Symfony\Component\Process\Process("docker-compose exec devmaster /usr/share/devshop/tests/devshop-tests.sh");
    $process->setTty(TRUE);
    $process->run();
  }

  /**
   * Get a one-time login link to Devamster.
   */
  public function login() {
    $this->_exec('docker-compose exec -T devmaster drush @hostmaster uli');
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

    $release_directories[] = '.';
    $release_directories[] = './aegir-home/devmaster-1.x/profiles/devmaster';

    $dirs = implode(' ', $release_directories);
    $cwd = getcwd();
    if ($this->confirm("Create the branch $release_branch in directories $dirs")) {
      foreach ($release_directories as $directory) {
        chdir($directory);
        $this->_exec("git checkout -b $release_branch");
      }
      chdir($cwd);
    }

    // Write version to files.
    if ($this->confirm("Write '$version' to ./aegir-home/devmaster-1.x/profiles/devmaster/VERSION.txt")) {
      file_put_contents('./aegir-home/devmaster-1.x/profiles/devmaster/VERSION.txt', $version);
    }

    if ($this->confirm("Write '$version' to install.sh? ")) {
      $this->_exec("sed -i -e 's/DEVSHOP_VERSION=1.x/DEVSHOP_VERSION=$version/' ./install.sh");
    }

    if ($this->confirm("Write '$drupal_org_version' to build-devmaster.make? ")) {
      $this->_exec("sed -i -e 's/projects\[devmaster\]\[version\] = 1.x-dev/projects[devmaster][version] = $drupal_org_version/' build-devmaster.make");
      $this->_exec("sed -i -e 's/projects\[devmaster\]\[download\]\[branch\]/; projects[devmaster][download][branch]' build-devmaster.make");
      $this->_exec("sed -i -e 's/projects\[devmaster\]\[download\]\[url\]/; projects[devmaster][download][url]' build-devmaster.make");
    }

    if ($this->confirm("Write '$drupal_org_version' to drupal-org.make for devshop_stats? ")) {
      $this->_exec("sed -i -e 's/projects\[devshop_stats\]\[version\] = 1.x/projects[devshop_stats][version] = $drupal_org_version/' ./aegir-home/devmaster-1.x/profiles/devmaster/drupal-org.make");
    }

    if ($this->confirm("Show git diff before committing?")) {
      foreach ($release_directories as $dir) {
        chdir($dir);
        $this->_exec("git diff -U1");
      }
      chdir($cwd);
    }
    if ($this->confirm("Commit changes to $release_branch? ")) {
      foreach ($release_directories as $dir) {
        chdir($dir);
        $this->_exec("git commit -am 'Automated Commit from DevShop Robofile.php `release` command. Preparing version $version.'");
      }
      chdir($cwd);
    }

    if ($this->confirm("Tag release of devshop_stats module to 7.x-$drupal_org_version? ")) {
      chdir('./aegir-home/devmaster-1.x/sites/all/modules/aegir/devshop_stats');

      if (file_exists('.git/config')) {
        $this->taskGitStack()
          ->tag($drupal_org_tag)
          ->run();
        $this->taskGitStack()
          ->push("origin", $drupal_org_tag)
          ->run();
      }
      else {
        $this->yell('No Tag Written! devshop_stats folder is not a git clone. Check ./aegir-home/devmaster-1.x/profiles/devmaster/modules/contrib/devshop_stats and ensure it is a git clone of http://git.drupal.org/project/devshop_stats');
      }
      chdir($cwd);
    }

    $this->say("Now, go create a new release for devshop_stats, so the build is ready before we push the new version of devmaster: https://www.drupal.org/node/add/project-release/2676696");
    $this->say("Use the tag $drupal_org_tag");

    $not_ready = TRUE;
    while ($not_ready) {
      $not_ready = !$this->confirm("Is the release ready?");
    }

    if ($this->confirm("Create a tag in devshop and devmaster repos? ")) {

      // Tag devshop with $version.
      chdir($cwd);
      $this->taskGitStack()
        ->tag($version, "DevShop $version")
        ->run();

      // Tag devmaster with version and drupal_org_version.
      chdir('./aegir-home/devmaster-1.x/profiles/devmaster');
      $this->taskGitStack()
        ->tag($version, "DevShop $version")
        ->tag($drupal_org_tag, "DevShop $version")
        ->run();
      chdir($cwd);
    }

    if ($this->confirm("Push the new release tags devshop $version and devmaster $drupal_org_version? (will attempt to push devmaster to remote 'drupal'.)")) {

      chdir($cwd);
      $this->taskGitStack()
        ->push("origin", $version)
        ->run();

      chdir('./aegir-home/devmaster-1.x/profiles/devmaster');
      if (!$this->taskGitStack()
        ->push("origin", $version)
        ->push("origin", $drupal_org_tag)
        ->push("drupal", $version)
        ->push("drupal", $drupal_org_tag)
        ->run()
        ->wasSuccessful()
      ) {
        $this->say('Pushing to remotes origin or drupal failed. If you need to add drupsl origin: git remote add origin username@git.drupal.org:project/devmaster.git');
      }

      chdir($cwd);
    }


    $this->say("The final steps we still have to do manually:");
    $this->say("1. Go create a new release of devmaster: https://www.drupal.org/node/add/project-release/1779370 using the tag $drupal_org_tag");
    $this->say("2. Wait for drupal.org to package up the distribution: https://www.drupal.org/project/devmaster");
    $this->say("3. Create a new 'release' on GitHub: https://github.com/opendevshop/devshop/releases/new using tag $version.  Copy CHANGELOG from  https://raw.githubusercontent.com/opendevshop/devshop/1.x/CHANGELOG.md");
    $this->say("4. Put the new version in gh-pages branch index.html");
  }
}
