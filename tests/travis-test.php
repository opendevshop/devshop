#!/bin/bash
<?php

require_once '../vendor/autoload.php';

use Symfony\Component\Process\Process;
use Symfony\Component\Process\Exception\ProcessFailedException;

$hostname = $argv[1];

$process = new Process('bin/behat');
$process->setTimeout(NULL);
$process->setWorkingDirectory(__DIR__);
$process->setEnv(array('BEHAT_PARAMS' => json_encode(array(
  'extensions' => array(
    'Behat\\MinkExtension' => array(
      'base_url' => "http://{$hostname}"
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