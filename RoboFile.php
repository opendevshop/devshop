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
   * Launch devshop after running prep:host, prep:source, and prep:containers.
   *
   * If you only run one command, run this one.
   */
  public function launch() {
    $this->prepareHost();
    $this->prepareSourcecode();
    $this->prepareContainers();
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
    'provision' => 'git@git.drupal.org:project/provision.git',
    'aegir-home/.drush/commands/registry_rebuild' => 'git@git.drupal.org:project/registry_rebuild.git',
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
      }
    }
  }
  
  /**
   * Build aegir and devshop containers from the Dockerfiles.
   */
  public function prepareContainers() {
  }
  
  /**
   * Launch devshop containers using docker-compose up, optionally outputting logs.
   */
  public function up($opts = ['follow' => true]) {
    if (!file_exists('aegir-home')) {
      if ($this->ask('aegir-home does not yet exist. Run "prepare:sourcecode" command?')) {
        $this->prepareSourcecode();
      }
      else {
        $this->say('aegir-home must exist for devshop to work. Not running docker-compose up.');
        return;
      }
    }
    $cmd = "docker-compose up -d";
    if ($opts['follow']) {
      $cmd .= "; docker-compose logs -f";
    }
    $this->_exec($cmd);
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
  public function destroy() {
    $this->_exec('docker-compose kill');
    $this->_exec('docker-compose rm -fv');
    
    $version = self::DEVSHOP_LOCAL_VERSION;
    $uri = self::DEVSHOP_LOCAL_URI;
    
    if ($this->taskFilesystemStack()
      ->remove('aegir-home/config')
      ->remove("aegir-home/projects")
      ->remove("aegir-home/.drush/project_aliases")
      ->remove("aegir-home/devmaster-{$version}/sites/{$uri}")
      ->run()
      ->wasSuccessful()) {
      
      // Remove only drush aliases, leave commands
      $this->_exec('rm aegir-home/.drush/*');

      $this->say("Deleted local folders. ");
    }
    else {
      $this->yell('Unable to delete local folders! Remove manually to fully destroy your local install.');
    }
  }
  
  /**
   * Stream logs from the containers using docker-compose logs -f
   */
  public function logs() {
  }
  
  /**
   * Enter a bash shell in the devmaster container.
   */
  public function shell() {
  }
  
  /**
   * Run all devshop tests on the containers.
   */
  public function test() {
  }
  
}