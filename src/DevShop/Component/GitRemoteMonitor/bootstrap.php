<?php

// If we're running from phar load the phar autoload file.
$pharPath = \Phar::running(true);
if ($pharPath) {
  $autoloaderPath = "$pharPath/vendor/autoload.php";
} else {
  if (file_exists(__DIR__.'/vendor/autoload.php')) {
    $autoloaderPath = __DIR__.'/vendor/autoload.php';
  } elseif (file_exists(__DIR__.'/../../autoload.php')) {
    $autoloaderPath = __DIR__ . '/../../autoload.php';

    // The path to autoloader when used in side the main devshop repo.
  } elseif (file_exists(__DIR__.'/../../../../vendor/autoload.php')) {
    $autoloaderPath = __DIR__ . '/../../../../vendor/autoload.php';
  } else {
    echo ("Could not find autoloader. Run 'composer install'.");
    exit(1);
  }
}
$classLoader = require $autoloaderPath;

