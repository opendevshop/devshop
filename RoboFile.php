<?php
/**
 * This is project's console commands configuration for Robo task runner.
 *
 * @see http://robo.li/
 */
class RoboFile extends \Robo\Tasks
{
  
  // The version of docker-compose to suggest the user install.
  const DOCKER_COMPOSE_VERSION = '1.10.0';
  
  // Defines where devmaster is installed.  'aegir-home/devmaster-$DEVSHOP_LOCAL_VERSION'
  const DEVSHOP_LOCAL_VERSION = '1.x';
  
  // Defines the URI we will use for the devmaster site.
  const DEVSHOP_LOCAL_URI = 'devshop.local.computer';
  
  /**
   * Launch devshop after running prep:host and prep:source. Use --build to build new local containers.
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
   * Check for docker, docker-compose and drush. Install them if they are missing.
   */
  public function prepareHost() {
    // Check for docker
    $this->say('Checking for Docker...');
    if ($this->taskDockerRun('hello-world')->printed(FALSE)->run()->wasSuccessful()) {
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
      
      $this->say('curl -L "https://github.com/docker/compose/releases/download/'  . self::DOCKER_COMPOSE_VERSION .'/docker-compose-$(uname -s)-$(uname -m)" -o /usr/local/bin/docker-compose && chmod +x /usr/local/bin/docker-compose');
    }
    
    // Check for drush
    $this->say('Checking for drush...');
    if ($this->_exec('drush --version')->wasSuccessful()) {
      $this->say('drush detected.');
    }
    else {
      $this->yell('Could not run drush.', 40, 'red');
      $this->say("Run the following command as root to install it or see http://www.drush.org/en/master/install/ for more information.");
      
      $this->say('php -r "readfile(\'https://s3.amazonaws.com/files.drush.org/drush.phar\');" > /usr/local/bin/drush
 && chmod +x /usr/local/bin/drush');
    }
  }
  
  private $repos = [
    'provision' => 'http://git.drupal.org/project/provision.git',
    'aegir-home/.drush/commands/registry_rebuild' => 'http://git.drupal.org/project/registry_rebuild.git',
    'documentation' => 'http://github.com/opendevshop/documentation.git',
    'dockerfiles' => 'http://github.com/opendevshop/dockerfiles.git',
    'aegir-dockerfiles' => 'http://github.com/aegir-project/dockerfiles.git',
  ];
  
  /**
   * Clone all needed source code and build devmaster from the makefile.
   */
  public function prepareSourcecode() {
    
    
    // Create the Aegir Home directory.
    if (file_exists("aegir-home/.drush/commands")) {
      $this->say("aegir-home/.drush/commands already exists.");
    }
    else {
      $this->taskExecStack()
        ->exec('mkdir -p aegir-home/.drush/commands')
        ->run();
    }
    
    // Clone all git repositories.
    foreach ($this->repos as $path => $url) {
      if (file_exists($path)) {
        $this->say("$path already exists.");
      }
      else {
        $this->taskGitStack()
          ->cloneRepo($url, $path)
          ->run();
      }
    }
    
    // Run drush make to build the devmaster stack.
    $make_path = "aegir-home/devmaster-1.x";
    if (file_exists("aegir-home/devmaster-1.x")) {
      $this->say("Path aegir-home/devmaster-1.x already exists.");
    } else {
      $result = $this->_exec("drush make build-devmaster.make aegir-home/devmaster-1.x --working-copy --no-gitinfofile");
      if ($result->wasSuccessful()) {
        $this->say('Built devmaster from makefile.');
      }
      else {
        $this->say("Drush make failed with the exit code " . $result->getExitCode());
        exit(1);
      }
    }
  }
  
  /**
   * Build aegir and devshop containers from the Dockerfiles. Detects your UID or you can pass as an argument.
   */
  public function prepareContainers($user_uid = NULL) {
  
    if (is_null($user_uid)) {
      $user_uid = $this->_exec('id -u')->getMessage();
    }

    $this->say("Found UID $user_uid. Passing to docker build as a build-arg...");

    // aegir/hostmaster
    $this->taskDockerBuild('aegir-dockerfiles')
      ->option('file', 'aegir-dockerfiles/Dockerfile')
      ->option('build-arg', "AEGIR_UID=$user_uid")
      ->tag('aegir/hostmaster')
      ->run()
      ;
    // aegir/hostmaster:xdebug
    $this->taskDockerBuild('aegir-dockerfiles')
      ->option('file', 'aegir-dockerfiles/Dockerfile-xdebug')
      ->tag('aegir/hostmaster:xdebug')
      ->run()
      ;
    // devshop/devmaster:xdebug
    $this->taskDockerBuild('dockerfiles')
      ->option('file', 'dockerfiles/Dockerfile-xdebug')
      ->tag('devshop/devmaster:xdebug')
      ->run()
      ;
    // aegir/web
    $this->taskDockerBuild('aegir-dockerfiles')
      ->option('file', 'aegir-dockerfiles/Dockerfile-web')
      ->tag('aegir/web')
      ->run()
      ;
  }
  
  /**
   * Launch devshop containers using docker-compose up and follow logs.
   *
   * Use "--test" option to run tests instead of the hosting queue.
   */
  public function up($opts = ['follow' => 1, 'test' => false]) {
    
    if (!file_exists('aegir-home')) {
      if ($opts['no-interaction'] || $this->ask('aegir-home does not yet exist. Run "prepare:sourcecode" command?')) {
        $this->prepareSourcecode();
      }
      else {
        $this->say('aegir-home must exist for devshop to work. Not running docker-compose up.');
        return;
      }
    }
    
    if ($opts['test']) {
      $cmd = "docker-compose run devmaster 'run-tests.sh'";
    }
    else {
      $cmd = "docker-compose up -d";
      if ($opts['follow']) {
        $cmd .= "; docker-compose logs -f";
      }
    }
    if ($this->_exec($cmd)->wasSuccessful()) {
      exit(0);
    }
    else {
      exit(1);
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
   */
  public function destroy($opts = ['force' => 0]) {
    $this->_exec('docker-compose kill');
    $this->_exec('docker-compose rm -fv');
    
    $version = self::DEVSHOP_LOCAL_VERSION;
    $uri = self::DEVSHOP_LOCAL_URI;
  
    if (!$opts['no-interaction'] && $this->confirm("Keep aegir-home/devmaster-{$version} folder?")) {
      if ($this->taskFilesystemStack()
        ->remove('aegir-home/config')
        ->remove("aegir-home/projects")
        ->remove("aegir-home/.drush/project_aliases")
        ->remove("aegir-home/.drush/*.php")
        ->run()
        ->wasSuccessful()) {
    
        // Remove devmaster site folder
        $this->_exec("sudo rm -rf aegir-home/devmaster-{$version}/sites/{$uri}");
    
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
  public function shell() {
    $this->say('Not yet implemented. Run the command:');
    $this->say('docker-compose exec devmaster bash');
  }
  
  /**
   * Run all devshop tests on the containers.
   */
  public function test() {
    $this->say('Not yet implemented. Run the command:');
    $this->say('docker-compose exec devmaster run-tests.sh');
  }
  
  /**
   * Get a one-time login link to Devamster.
   */
  public function login() {
    $this->_exec('docker-compose exec -T devmaster drush @hostmaster uli');
  }
}