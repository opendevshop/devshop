<?php
/**
 * This is project's console commands configuration for Robo task runner.
 *
 * @see http://robo.li/
 */
class RoboFile extends \Robo\Tasks
{
  /**
   * Build then run the devshop docker image.
   */
  function up() {
    $this->build();
    $this->run();
  }

  /**
   * Rebuild the image from the dockerfile.
   */
  function build() {
    $this->_exec('docker build -t devshop/server .');
  }

  /**
   * Run docker-compose up -d and follow the logs.
   */
  function run() {
    $this->_exec('docker-compose up -d; docker-compose logs -f');
  }

  /**
   * Destroy the containers and the docker volumes.
   */
  function destroy() {
    $this->_exec('docker-compose kill; docker-compose rm -fv; docker volume rm dockerfiles_aegir; docker volume rm dockerfiles_mysql;');
  }


  /**
   * Enter a bash shell in the devmaster container.
   */
  public function shell() {
    $process = new \Symfony\Component\Process\Process("docker-compose exec devshop bash");
    $process->setTty(TRUE);
    $process->run();
  }

  /**
   * Get a log
   */
  public function login() {
    $this->_exec('docker-compose exec -T devmaster drush @hostmaster uli');
  }
}