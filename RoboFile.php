<?php

require_once 'vendor/autoload.php';

use Symfony\Component\Yaml\Yaml;
use Symfony\Component\Console\Output\OutputInterface;
use Symfony\Component\Console\Exception\RuntimeException;

/**
 * This file provides commands to the robo CLI for managing development and
 * testing of devshop.
 *   1. Install Composer: https://github.com/composer/composer/releases
 *   2. Clone this repo and change into the directory.
 *   3. Run `bin/robo` to see the commands.
 *   4. If you have docker, and docker compose, you can launch a devshop
 *      with `robo up`.
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

  use \Robo\Common\IO;

  /**
   * @var The path to devshop root. Used for upgrades.
   */
  private $devshop_root_path;

  /**
   * Map of $opts keys to $_SERVER variables.
   *
   * SERVER vars are set for the "docker-compose" process.
   *
   * @var array
   */
  private $serverOptionsMap = [
    'verbose' => 'ANSIBLE_VERBOSITY',
    'vars' => 'ANSIBLE_EXTRA_VARS',
    'tags' => 'ANSIBLE_TAGS',
    'skip-tags' => 'ANSIBLE_SKIP_TAGS',
    'playbook' => 'ANSIBLE_PLAYBOOK',
    'playbook-command-options' => 'ANSIBLE_PLAYBOOK_COMMAND_OPTIONS',
    'build-command' => 'DEVSHOP_DOCKER_COMMAND_BUILD',
    'run-command' => 'DEVSHOP_DOCKER_COMMAND_RUN',

    // Used in docker compose image.
    'from' => 'FROM_IMAGE',
    'os' => 'OS_VERSION',
    'dockerfile' => 'DOCKERFILE',
    'compose-file' => 'COMPOSE_FILE',
  ];

  /**
   * Map of Symfony Console verbosity to Ansible Verbosity value.
   * @var array
   */
  private $ansibleVerbosityMap = [
    OutputInterface::VERBOSITY_NORMAL => 0,
    OutputInterface::VERBOSITY_VERBOSE => 1,
    OutputInterface::VERBOSITY_VERY_VERBOSE => 2,
    OutputInterface::VERBOSITY_DEBUG => 3,
  ];

  /**
   * Merge robo $opts and $_SERVER environment vars into the runtime environment
   * of the docker-compose calls.
   *
   * @param array $opts
   */

  /**
   * @param array $opts The robo options array.
   * @param array $env The initial environment.
   *
   * @return array
   */
  private function generateEnvironment(array $opts, array $env = []) {
    $env += $this->optionsToArray($opts['environment']);

    foreach ($this->serverOptionsMap as $opt_name => $var_name) {
      // Use $_SERVER var if it exists...
      $env[$var_name] = !empty($_SERVER[$var_name])? $_SERVER[$var_name]:
        // or use --options value if it exists.
        // If not, set to empty string.
        (!empty($opts[$opt_name])? $opts[$opt_name]: '');
    }

    $env['ANSIBLE_VERBOSITY'] = $this->ansibleVerbosityMap[$this->output()->getVerbosity()];

    return $env;
  }

  /**
   * Append _ARG to all variable names of an environment vars array.
   * @param bool new Set to "true" to reset environment with only the new _ARG values.
   * @return array
   */
  private function generateEnvironmentArgs(array $opts, $new = false) {

    // Convert opts to environment vars.
    $environment = $this->generateEnvironment($opts, $_ENV);

    // Load default environment, either empty or from existing.
    $return_env = $new? []: $environment;

    // Append _ARG to all environment variable names.
    foreach ($environment as $name => $value) {
      $return_env["{$name}_ARG"] = $value;
    }
    return $return_env;
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
    'aegir-home/.drush/commands/registry_rebuild' => 'http://git.drupal.org/project/registry_rebuild.git#',
  ];

  /**
   * Clone all needed source code and build devmaster from the makefile.
   *
   * @option no-dev Skip setting git remotes to SSH URLs.
   */
  public function prepareSourcecode($opts = [
    'no-dev' => FALSE,
    'test-upgrade' => FALSE,
  ]) {

    if (empty($this->git_ref)) {
      parent::yell("Preparing Sourcecode: Branch Unknown.");
    }
    else {
      parent::yell("Preparing Sourcecode: Branch $this->git_ref");
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
      $this->taskExecStack()
        ->exec("mkdir -p {$this->devshop_root_path}/aegir-home/test-artifacts")
        ->run();
    }

    // Clone all git repositories.
    foreach ($this->repos as $path => $url) {
      // Allow repos to specify a branch after #.
      [$url, $branch] = explode('#', $url);

      if (file_exists($this->devshop_root_path . '/' . $path)) {
        $this->say("$path already exists.");
      }
      else {
        $this->taskGitStack()
          ->cloneRepo($url, $this->devshop_root_path . '/' . $path, $branch)
          ->run();
      }
    }

    // Set git remote urls
    if ($opts['no-dev'] == FALSE) {
      // @TODO: Set git url for others like provision
      $devshop_ssh_git_url = "git@github.com:opendevshop/devshop.git";

      if ($this->taskExec("git remote set-url origin $devshop_ssh_git_url")->run()->wasSuccessful()) {
        $this->yell("Set devshop git remote 'origin' to $devshop_ssh_git_url!");
      }
      else {
        $this->say("<comment>Unable to set devshop git remote to $devshop_ssh_git_url !</comment>");
      }
    }
  }

  /**
   * Build all devshop containers from scratch. Rarely necessary: `robo up` will re-run ansible configuration.
   *
   * @param folder The folder to run 'docker-compose build' in. Use "all" to build in folder 'docker', then 'roles'.
   * @param service The service to build. passed to 'docker-compose build $SERVICE'. Use "all" to build all services.
   *
   * @option $tag The string to tag the resulting container with.
   * @option $from The image to use to build the docker image FROM. Ignored if "os" is set.
   * @option $os An OS "slug" for any of the geerlingguy/docker-*-ansible images: https://hub.docker.com/u/geerlingguy/
   * @option $vars Ansible vars to pass to --extra-vars option.
   * @option $tags Ansible tags to pass to --tags option.
   * @option $skip_tags Ansible tags to pass to --skip-tags option.
   * @option $playbook Ansible tags to pass to ansible-playbook command.
   * @option install-at-runtime Launch bare containers and then install devshop.
   * @option $build-command The command to run at the end of the docker build process. (Defaults to scripts/devshop-ansible-playbook)
   */
  public function build($folder = 'roles', $service = 'devshop.server', $opts = [
      'docker-image' => 'devshop/server:latest',
      'from' => NULL,
      'build-command' => NULL,
      'os' => NULL,
      'vars' => '',
      'tags' => '',
      'skip-tags' => '',
      'playbook' => 'roles/devshop.server/play.yml',
      'environment' => [],
      'install-at-runtime' => FALSE,
  ]) {

    if ($service == "all") {
      $service = '';
    }
    if ($folder == "all") {
      $folders = ['docker', 'roles'];
    } else {
      $folders = [$folder];
    }

    // Define docker-image (name for the "image" in docker-compose)
    // Set FROM_IMAGE and DEVSHOP_DOCKER_IMAGE if --os option is used. (and --from was not used)
    if (empty($opts['from']) && $opts['os'] !== NULL) {
      $opts['from'] = "geerlingguy/docker-{$opts['os']}-ansible";
      $opts['docker-image'] = 'devshop/server:' . $opts['os'];
    }

    // Append the absolute path in the container.
    $opts['playbook'] = '/usr/share/devshop/' . $opts['playbook'] ;

    $this->yell('Building DevShop Containers...', 40, 'blue');

    // Block anything from running on build.
    // @TODO: Figure out why centos can't enable service in build phase.
    if ($opts['os'] == 'centos7' || $opts['install-at-runtime']) {
      $opts['tags'] = $_SERVER['ANSIBLE_TAGS'] = 'none';
      $opts['skip-tags'] = $_SERVER['ANSIBLE_SKIP_TAGS'] = '';

      if ($opts['os'] == 'centos7') {
        $this->yell('CENTOS DETECTED in RUNTIME. Running full playbook in container.', 40, 'red');
      }
      else {
        $this->yell('--install-at-runtime option detected. Skipping build in container.', 40, 'red');
      }

      $this->yell('CENTOS DETECTED in BUILDTIME. Skipping playbook run in image build.', 40, 'red');
    }

    // Runtime Environment for the docker-compose build command.
    $env_build = $this->generateEnvironmentArgs($opts);

    $provision_io = new \DevShop\Component\PowerProcess\PowerProcessStyle($this->input(), $this->output());
    $process = new \DevShop\Component\PowerProcess\PowerProcess("docker-compose build --no-cache $service", $provision_io);
    $process->setEnv($env_build);
    $process->disableOutput();
    $process->setTimeout(null);
    $process->setTty(!empty($_SERVER['XDG_SESSION_TYPE']) && $_SERVER['XDG_SESSION_TYPE'] == 'tty');

    // @TODO: Figure out why PowerProcess::mustRun() fails so miserably: https://github.com/opendevshop/devshop/pull/541/checks?check_run_id=518074346#step:7:45

    // Run docker-compose build in docker and in roles folder.
    foreach ($folders as $compose_files_path) {
      $this->yell("Building in directory: $compose_files_path", 40, 'blue');
      $process->setWorkingDirectory($compose_files_path);
      $process->run();
    }

    if ($process->getExitCode() != 0) {
      throw new \Exception('Process failed: ' . $process->getExitCodeText());
    }

  }

  /**
   * Launch devshop in a variety of ways. Useful for local development and CI
   * testing.
   *
   * Examples:
   *
   *   robo up
   *   Launch a devshop in containers using docker-compose.yml
   *
   *   robo up --test
   *   Launch then test a devshop in a single process.
   *
   *   robo up --upgrade --test
   *   Launch, upgrade, then test a devshop in a single process.
   *
   *   robo up --mode=install.sh --test
   *   Launch an OS container, then install devshop using install.sh, then run
   * tests.
   *
   *   robo up ps
   *   Launch the container and run "ps" instead of "devshop-ansible-playbook"
   *
   *   robo up --build
   *   Run robo build before launching.
   *
   *   bin/robo up whoami --build --build-command=whoami
   *   Launch containers using "whoami" instead of devshop-ansible-playbook, after building containers using "whoami" instead of "devshop-ansible-playbook".
   *
   *   bin/robo up --tags=install-devmaster
   *   Launch containers passing ansible tags "install-devmaster" to skip straight to installing Drupal Devmaster.
   *
   *   bin/robo up --destroy --no-interaction
   *   Destroy and relaunch containers without being prompted to confirm destruction of data.
   *
   * @option $destroy Run 'robo destroy' before up to rebuild the entire stack.
   * @option $no-follow Don't tail the docker logs after launching.
   * @option $test Run tests after containers are up and devshop is installed.
   * @option $test-upgrade Install an old version, upgrade it to this version,
   *   then run tests.
   * @option $mode Set to 'install.sh' to use the install.sh script for setup.
   * @option $user-uid Override the detected current user's UID when building
   *   containers.
   * @option $no-dev Skip setting git remotes to SSH URLs.
   * @option $build Rebuild devshop containers from scratch before launching.
   * @option $os An OS "slug" for any of the geerlingguy/docker-*-ansible images: https://hub.docker.com/u/geerlingguy/
   * @option $environment pass an environment variable to docker-compose in the form --environment NAME=VALUE
   * @option $ci Run 'robo up' in CI mode. Disables docker volumes.
   * @option $install-at-runtime Run with ansible tag "all" and skip-tags set to "none". Ensures a full playbook run on docker up.
   * @option $build-command The command to run at the end of the docker build process. (Defaults to scripts/devshop-ansible-playbook)
   * @option $skip-source-prep Prevent prepare:sourcecode from running if the aegir-home folder does not exist.
   * @option $vars Ansible variables string to pass to the 'ansible-playbook' command in devshop-ansible-playbook.
   * @option $tags Ansible tags to pass to the 'ansible-playbook' command in devshop-ansible-playbook.
   * @option $skip-tags Ansible tags to pass to the 'ansible-playbook' command in devshop-ansible-playbook.
   * @option $playbook The path to an ansible playbook to pass to the 'ansible-playbook' command in devshop-ansible-playbook.
   * @option $playbook-command-options Command line options to append to the 'ansible-playbook' command in devshop-ansible-playbook.
   * @option $compose-file Use a different docker-compose.yml file. Passes to COMPOSE_FILE env var.
   * @option $force-reinstall Delete and reinstall the devmaster site, if it exists.
   * @option $build-folder If using --build, the folder to run 'docker-compose build' in. Use "all" to build in folder 'docker', then 'roles'.
   * @option $build-service If using --build, the service to build. passed to 'docker-compose build $SERVICE'. Use "all" to build all services.
   */
  public function up($docker_command = '/usr/share/devshop/scripts/devshop-ansible-playbook', $opts = [
    'destroy' => FALSE,
    'no-follow' => FALSE,
    'test' => FALSE,
    'test-upgrade' => FALSE,

    // Set 'mode' => 'install.sh' to run a traditional OS install.
    'mode' => 'docker-compose',
    'user-uid' => NULL,
    'no-dev' => FALSE,
    'build' => FALSE,
    'skip-source-prep' => FALSE,
    // This is the image string used in docker-compose.
    'docker-image' => 'devshop/server:latest',
    // The OS "slug" to use instead of devshop/server:ubuntu1804. If specified, "docker-image" option will be ignored.
    'os' => NULL,
    'from' => NULL,
    'vars' => '',
    'tags' => '',
    'skip-tags' => '',
    'playbook' => 'roles/devshop.server/play.yml',
    'playbook-command-options' => '',
    'environment' => [],
    'ci' => FALSE,
    'install-at-runtime' => FALSE,
    'build-command' => NULL,
    'compose-file' => NULL,
    'force-reinstall' => FALSE,
    'build-folder' => 'roles',
    'build-service' => 'devshop.server',
  ]) {

    // Override the DEVSHOP_DOCKER_COMMAND_RUN if specified.
    if (!empty($docker_command)) {
      # Ensures argument is passed to DEVSHOP_DOCKER_COMMAND_RUN later.
      $opts['run-command'] = $docker_command;
    }

    if ($opts['destroy']) {
      $this->yell('Destroying instance: --destroy option was used.', 80, "red");
      $this->destroy();
    }

    // Define docker-image (name for the "image" in docker-compose.
    // Set FROM_IMAGE and DEVSHOP_DOCKER_IMAGE if --os option is used. (and --from was not used)
    if (empty($opts['from']) && !empty($opts['os'])) {
      $opts['from'] = "geerlingguy/docker-{$opts['os']}-ansible";
      $opts['docker-image'] = 'devshop/server:' . $opts['os'];
    }
    else {
      $opts['from'] = $opts['docker-image'];
    }

    // Check for tools
    $this->prepareHost();

    if (empty($this->devshop_root_path)) {
      $this->devshop_root_path = __DIR__;
    }

    if (empty($this->git_ref)) {
      parent::yell("Launching DevShop: Branch Unknown.");
    }
    else {
      parent::yell("Launching DevShop: Branch $this->git_ref");
    }

    // Determine current UID.
    if (empty($opts['user-uid'])) {
      $opts['user-uid'] = trim(shell_exec('id -u'));
    }

    // Build the image if --build option specified, or if the image doesn't exist yet locally.
    // If we don't, docker-compose up will automatically build it, but without these options.
    // Run a "docker-compose pull" here confirms that the remote container by this name exists, and gets us a local copy.
    $this->say("<comment>Getting latest devshop/server image...</comment>");
    $docker_image_name_exists = $this->_exec("docker pull {$opts['docker-image']}")->wasSuccessful();

    // The image was just pulled, so this should always be true if $docker_image_name_exists is true.
    $docker_image_exists_locally = $this->_exec("docker inspect {$opts['docker-image']} > /dev/null")->wasSuccessful();

    // If --build option is used, or if docker image does not exist anywhere, build it with "local-$OS" tag
    if ($opts['build'] || !$docker_image_name_exists && !$docker_image_exists_locally) {
      $this->yell("Docker Image {$opts['docker-image']} was not found on this system or on docker hub.", 40, "blue");
      $this->say("Building it locally...");
      $this->build($opts['build-folder'], $opts['build-service'], $opts);
    }
    // Warn the user that this container is not being built.
    elseif (!$opts['build'] && $docker_image_name_exists) {
      $this->yell("Launching {$opts['docker-image']}... Use --build to rebuild it.", 40, "blue");
    }

    // @TODO: Figure out why centos can't enable service in build phase.
    if ($opts['os'] == 'centos7' || $opts['install-at-runtime']) {
      // Set tags to all so it does a full install at runtime.
      $opts['tags'] = $_SERVER['ANSIBLE_TAGS'] = 'all';
      $opts['skip-tags'] = $_SERVER['ANSIBLE_SKIP_TAGS'] = 'none';

      if ($opts['os'] == 'centos7') {
        $this->yell('CENTOS DETECTED in RUNTIME. Running full playbook in container.', 40, 'red');
      }
      else {
        $this->yell('--install-at-runtime option detected. Running full playbook in container.', 40, 'red');
      }
    }

    if ($opts['mode'] == 'docker-compose') {

      // Volumes
      if (!$opts['ci']) {
        $this->yell('Mounting Docker Volumes... Use --ci to disable volumes.', 40, 'blue');

        // Set COMPOSE_FILE to include volumes.
        $opts['compose-file'] = 'docker-compose.yml:docker-compose.volumes.yml';

        if (!file_exists('aegir-home') && !$opts['skip-source-prep']) {
          $this->say('<warning>The aegir-home folder not present. Running prepare source code command.</warning>');
          $this->prepareSourcecode($opts);
        }
      }


      // Test commands must be run as application user.
      // The `--test` command is run in GitHub Actions.
      $test_command = '';
      if ($opts['test']) {
        // Do not run a playbook on docker-compose up, because it will launch as a separate process and we won't know when it ends.
        $cmd[] = "docker-compose run devshop {$docker_command}";
        $env_run['DEVSHOP_DOCKER_COMMAND_RUN'] = $docker_command;

        $test_command = "su aegir --command /usr/share/devshop/tests/devshop-tests.sh";
      }
      // @TODO: The `--test-upgrade` command is NOT YET run in GitHub Actions.
      // The PR with the update hook can be used to finalize upgrade tests: https://github.com/opendevshop/devshop/pull/426
      elseif ($opts['test-upgrade']) {
        $cmd[] = "docker-compose run --rm devshop {$docker_command}";
        $test_command = "/usr/share/devshop/tests/devshop-tests-upgrade.sh";
      }
      else {
        $cmd[] = "docker-compose up --detach --force-recreate devshop";
        if (!$opts['no-follow']) {
          $cmd[] = "docker-compose logs -f";
        }
        else {
          $cmd[] = "docker-compose logs";
        }
      }

      // Runtime Environment for the $cmd list.
      $env_run = $this->generateEnvironmentArgs($opts);
      $extra_vars = array();
      $extra_vars['devshop_version'] = $this->git_ref;

      # @TODO: Move all static vars into vars.development.yml.
      # Don't upgrade every time we robo up.
      $extra_vars['devshop_cli_skip_update'] = true;
      $extra_vars['devmaster_skip_upgrade'] = true;

      // Set extra ansible vars when not in CI.
      if (empty($_SERVER['CI'])) {
        if ($opts['force-reinstall']) {
          $extra_vars['devshop_control_install_options'] = '--force-reinstall';
        }

        if ($opts['user-uid']) {
          $extra_vars['aegir_user_uid'] = $opts['user-uid'];
          $extra_vars['aegir_user_gid'] = $opts['user-uid'];
        }
      }

      // Run a test command after the docker command.
      if ($test_command) {
        $env_run['DOCKER_COMMAND_POST'] = $test_command;
        $env_run['DOCKER_COMMAND_RUN_POST_EXIT'] = 1;

        $extra_vars['supervisor_started'] = false;
      }
      else {
        $env_run['DOCKER_COMMAND_POST'] = 'devshop login';
      }

      // Process $extra vars into JSON for ENV var.
      $env_run['ANSIBLE_EXTRA_VARS'] = json_encode($extra_vars);

      // Override the DEVSHOP_DOCKER_COMMAND_RUN if specified.
      if (!empty($docker_command)) {
        $env_run['DEVSHOP_DOCKER_COMMAND_RUN'] = $docker_command;
      }

      if ($this->output->isVerbose()) {
        $this->say('Ansible Extra Vars:');
        print_r($extra_vars);
        $this->say('Execution environment:');
        print_r($env_run);
      }

      if (!empty($cmd)) {
        foreach ($cmd as $command) {
          $provision_io = new \DevShop\Component\PowerProcess\PowerProcessStyle($this->input, $this->output);
          $process = new \DevShop\Component\PowerProcess\PowerProcess($command, $provision_io);
          $process->setEnv($env_run);
          $isTty = !empty($_SERVER['XDG_SESSION_TYPE']) && $_SERVER['XDG_SESSION_TYPE'] == 'tty';
          $process->setTty($isTty);
          $process->setTimeout(NULL);
          $process->disableOutput();

          // @TODO: Figure out why PowerProcess::mustRun() fails so miserably: https://github.com/opendevshop/devshop/pull/541/checks?check_run_id=518074346#step:7:45
          // $process->mustRun();
          $process->run();
          if ($process->getExitCode() != 0) {
            throw new \Exception('Process failed: ' . $process->getExitCodeText());
          }
        }
        return;
      }
    }
  }

  /**
   * Convert this:    to this:
   *
   * array(           array(
   *   "this=that"      "this" => "that"
   * );               );
   *
   * @param $options_list
   *
   * @return array
   */
  private function optionsToArray($options_list) {
    $vars = [];
    foreach ($options_list as $options_string) {
      [$name, $value] = explode("=", $options_string);
      $vars[$name] = $value;
    }
    return $vars;
  }

  /**
   * Run a command in the devshop container.
   */
  public function exec($cmd = '') {
    return $this->_exec("docker-compose exec -T \
      --env ANSIBLE_TAGS=runtime \
      --env ANSIBLE_SKIP_TAGS \
      --env ANSIBLE_VARS \
      devshop $cmd")->getExitCode();
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
    if (!$this->input()->isInteractive() || $this->confirm("Destroy all local data? (docker containers, volumes, config)")) {
      // Remove devmaster site folder
      $version = self::DEVSHOP_LOCAL_VERSION;
      $uri = self::DEVSHOP_LOCAL_URI;
      $this->_exec("docker-compose exec devshop rm -rf /usr/share/devshop/src/DevShop/Component/DevShopControlTemplate/web/sites/{$uri}");
      $this->_exec('docker-compose kill');
      $this->_exec('docker-compose rm -fv');
    }

    // Don't run when -n is specified,
    if (!$this->input()->isInteractive() || $this->confirm("Destroy container home directory? (aegir-home)")) {
      if ($this->_exec("rm -rf aegir-home")->wasSuccessful()) {
        $this->say("Entire aegir-home folder deleted.");
      }
    }
    else {
      $this->say("The aegir-home directory was retained. It will be  present when 'robo up' is run again.");
    }

    // Uninstall composer vendor code?
    if (!$this->input()->isInteractive() || $this->confirm("Composer uninstall DevShop Control?")) {
      $this->taskExec("composer uninstall")
        ->dir("src/DevShop/Component/DevShopControlTemplate")
        ->run();
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
    return $process->getExitCode();
  }

  /**
   * Run all devshop tests on the containers.
   */
  public function test($user = 'aegir', $opts = array(
    'compose-file' => 'docker-compose.yml:docker-compose.volumes.yml',
    'reinstall' => FALSE
  )) {
    $is_tty = !empty($_SERVER['XDG_SESSION_TYPE']) && $_SERVER['XDG_SESSION_TYPE'] == 'tty';
    $no_tty = !$is_tty? '-T': '';

    // If running in CI, create the test-artifacts directory and ensure ownership first.
    // @TODO: Move logic to a special CI container.
    if (!empty($_SERVER['CI'])) {
      $commands[] = "docker-compose exec $no_tty devshop mkdir -p /var/aegir/test-artifacts";
      $commands[] = "docker-compose exec $no_tty devshop chown aegir:aegir /var/aegir/test-artifacts -R";
      $commands[] = "docker-compose exec $no_tty devshop chmod 777 /var/aegir/test-artifacts -R";
    }

    if ($opts['reinstall']) {
      $commands[] = "docker-compose exec $no_tty --user $user devshop drush @hostmaster provision-install --force-reinstall";
    }

    $commands[] = "docker-compose exec $no_tty --user $user devshop /usr/share/devshop/tests/devshop-tests.sh";
    $provision_io = new \DevShop\Component\PowerProcess\PowerProcessStyle($this->input, $this->output);
    foreach ($commands as $command) {
      $process = new \DevShop\Component\PowerProcess\PowerProcess($command, $provision_io);

      $process->setTty(!empty($_SERVER['XDG_SESSION_TYPE']) && $_SERVER['XDG_SESSION_TYPE'] == 'tty');

      $process->setEnv([
        'COMPOSE_FILE' => $opts['compose-file'],
      ]);
      $process->setTimeout(NULL);
      $process->disableOutput();
    // @TODO: Figure out why PowerProcess::mustRun() fails so miserably: https://github.com/opendevshop/devshop/pull/541/checks?check_run_id=518074346#step:7:45
    // $process->mustRun();
      $process->run();
      if (!$process->isSuccessful()) {
        return $process->getExitCode();
      }
    }
    return $process->getExitCode();
  }

  /**
   * Get a one-time login link to Devamster.
   */
  public function login($user = 'aegir') {
      // @TODO: Figure out why PATH is gone.
    $this->_exec("docker-compose exec --user $user -T devshop /usr/share/devshop/bin/drush @hostmaster uli");
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
