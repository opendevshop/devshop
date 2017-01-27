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
  
  /**
   * Clone all needed source code and build devmaster from the makefile.
   */
  public function prepareSourcecode() {
  }
  
  /**
   * Build aegir and devshop containers from the Dockerfiles.
   */
  public function prepareContainers() {
  }
  
  /**
   * Launch devshop containers using docker-compose up, optionally outputting logs.
   */
  public function up() {
  }
  
  /**
   * Stop devshop containers using docker-compose stop
   */
  public function stop() {
  }
  
  /**
   * Destroy all containers and volumes using docker-compose rm -f
   */
  public function destroy() {
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