#!/usr/bin/env php
<?php

require_once '../vendor/autoload.php';

use Symfony\Component\Process\Process;
use Symfony\Component\Process\Exception\ProcessFailedException;

if (isset($argv[1]))  {
  $hostname = $argv[1];
}
else {
  $hostname = 'devshop.site';
}

// Look for hostmaster alias file.

if (file_exists('/var/aegir/.drush/hostmaster.alias.drushrc.php')) {
  require_once '/var/aegir/.drush/hostmaster.alias.drushrc.php';
  $path = $aliases['hostmaster']['root'];
}
else {
  echo "Hostmaster alias not found.\n";
  exit(1);
}


$process = new Process('bin/behat --colors');
$process->setTimeout(NULL);
$process->setWorkingDirectory(__DIR__);
$process->setEnv(array('BEHAT_PARAMS' => json_encode(array(
  'extensions' => array(
    'Behat\\MinkExtension' => array(
      'base_url' => "http://{$hostname}"
    ),
    'Drupal\\DrupalExtension' => array(
      'drush' => array(
        'root' => $path
      ),
      'drupal' => array(
        'drupal_root' => $path
      )
    )
  )
))));

$process->run(function ($type, $buffer) {
  if (Process::ERR === $type) {
    echo $buffer;
  } else {
    echo $buffer;
  }
});

// executes after the command finishes
if (!$process->isSuccessful()) {
  exit(1);
}