<?php

/**
 * @file
 * Contains \DrupalProject\composer\ScriptHandler.
 */

namespace DrupalProject\composer;

use Composer\EventDispatcher\ScriptExecutionException;
use Composer\Script\Event;
use Composer\Semver\Comparator;
use DrupalFinder\DrupalFinder;
use Symfony\Component\Filesystem\Filesystem;

class ScriptHandler {

  public static function checkDevmasterPackage(Event $event) {
    $fs = new Filesystem();
    $io = $event->getIO();
    $drupalFinder = new DrupalFinder();
    $drupalFinder->locateRoot(getcwd());
    $drupalRoot = $drupalFinder->getDrupalRoot();

    $devmaster_path = $drupalRoot . '/profiles/devmaster';
    if (is_link($devmaster_path)) {
      $real_devmaster_path = realpath($devmaster_path);
      $devmaster_path = $real_devmaster_path;
    }

    $devmaster_info_path = $devmaster_path  . '/devmaster.info';

    // Both web/profiles/devmaster directory and devmaster.info file are found.
    if ($fs->exists($devmaster_path) && $fs->exists($devmaster_info_path)) {
      if (is_link($drupalRoot . '/profiles/devmaster')) {
        $io->write("<info>SUCCESS</info> The package <comment>devshop/devmaster</comment> was installed via symlink from <comment>$real_devmaster_path</comment> to <comment>web/profiles/devmaster</comment> ");
        passthru("ls -la web/profiles/devmaster");
        passthru("ls -la $real_devmaster_path/devmaster.info");
      }
      else {
        $io->write("<info>SUCCESS</info> The package <comment>devshop/devmaster</comment> was installed to <comment>web/profiles/devmaster</comment>");
        passthru("ls -la web/profiles/devmaster/devmaster.info");
      }
    }
    // Error: web/profiles/devmaster directory exists but no info file found.
    elseif ($fs->exists($devmaster_path) && !$fs->exists($devmaster_info_path)) {

      // @TODO: Uncomment this when attempting to fix the missing profile problenm for good.
      // throw new \Exception('There is no devmaster.info file in the path for package devshop/devmaster: ' . $devmaster_info_path);
      $io->writeError('<error>ERROR</error> There is no devmaster.info file in the path for package devshop/devmaster: ' . $devmaster_info_path);
    }
    // Error: No web/profiles/devmaster directory found at all.
    elseif (!$fs->exists($devmaster_path)) {
      // @TODO: Uncomment this when attempting to fix the missing profile problenm for good.
      // throw new \Exception('There is no directory at the expected location for the devshop/devmaster install profile. A second call to composer install will fix the problem. Expected path: ' . $devmaster_path);
      $io->writeError('<error>ERROR</error>There is no directory at the expected location for the devshop/devmaster install profile. A second call to composer install will fix the problem. Expected path: ' . $devmaster_path);
    }
  }

  public static function createRequiredFiles(Event $event) {
    $fs = new Filesystem();
    $drupalFinder = new DrupalFinder();
    $drupalFinder->locateRoot(getcwd());
    $drupalRoot = $drupalFinder->getDrupalRoot();

    $dirs = [
      'sites/all/modules',
      'profiles',
      'sites/all/themes',
    ];

    // Required for unit testing
    foreach ($dirs as $dir) {
      if (!$fs->exists($drupalRoot . '/'. $dir)) {
        $fs->mkdir($drupalRoot . '/'. $dir);
        $fs->touch($drupalRoot . '/'. $dir . '/.gitkeep');
      }
    }

    // Prepare the settings file for installation
    if (!$fs->exists($drupalRoot . '/sites/default/settings.php') && $fs->exists($drupalRoot . '/sites/default/default.settings.php')) {
      $fs->copy($drupalRoot . '/sites/default/default.settings.php', $drupalRoot . '/sites/default/settings.php');
      $fs->chmod($drupalRoot . '/sites/default/settings.php', 0666);
      $event->getIO()->write("Created a sites/default/settings.php file with chmod 0666");
    }

    // Create the files directory with chmod 0777
    if (!$fs->exists($drupalRoot . '/sites/default/files')) {
      $oldmask = umask(0);
      $fs->mkdir($drupalRoot . '/sites/default/files', 0777);
      umask($oldmask);
      $event->getIO()->write("Created a sites/default/files directory with chmod 0777");
    }
  }

  /**
   * Remove project-internal files after create project.
   */
  public static function removeInternalFiles(Event $event) {
    $fs = new Filesystem();

    // List of files to be removed.
    $files = [
      '.travis.yml',
      'LICENSE',
      'README.md',
      'phpunit.xml.dist',
    ];

    foreach ($files as $file) {
      if ($fs->exists($file)) {
        $fs->remove($file);
      }
    }
  }

  /**
   * Checks if the installed version of Composer is compatible.
   *
   * Composer 1.0.0 and higher consider a `composer install` without having a
   * lock file present as equal to `composer update`. We do not ship with a lock
   * file to avoid merge conflicts downstream, meaning that if a project is
   * installed with an older version of Composer the scaffolding of Drupal will
   * not be triggered. We check this here instead of in drupal-scaffold to be
   * able to give immediate feedback to the end user, rather than failing the
   * installation after going through the lengthy process of compiling and
   * downloading the Composer dependencies.
   *
   * @see https://github.com/composer/composer/pull/5035
   */
  public static function checkComposerVersion(Event $event) {
    $composer = $event->getComposer();
    $io = $event->getIO();

    $version = $composer::VERSION;

    // The dev-channel of composer uses the git revision as version number,
    // try to the branch alias instead.
    if (preg_match('/^[0-9a-f]{40}$/i', $version)) {
      $version = $composer::BRANCH_ALIAS_VERSION;
    }

    // If Composer is installed through git we have no easy way to determine if
    // it is new enough, just display a warning.
    $io->write("Composer version <comment>$version</comment> detected.");
    if ($version === '@package_version@' || $version === '@package_branch_alias_version@') {
      $io->writeError('<warning>You are running a development version of Composer. If you experience problems, please update Composer to the latest stable version.</warning>');
    }
    elseif (Comparator::lessThan($version, '1.0.0')) {
      $io->writeError('<error>Drupal-project requires Composer version 1.0.0 or higher. Please update your Composer before continuing</error>.');
      exit(1);
    }
  }

}
