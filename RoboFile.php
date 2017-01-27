<?php
/**
 * This is project's console commands configuration for Robo task runner.
 *
 * @see http://robo.li/
 */
class RoboFile extends \Robo\Tasks
{
  
  /**
   * Check for docker, docker-compose and drush. Install them if they are missing.
   */
  public function prepareHost() {
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