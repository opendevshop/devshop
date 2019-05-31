#!/usr/bin/env php
<?php

// Include autoloader
include('vendor/autoload.php');

// PowerProcess needs IO.
$input = new \Symfony\Component\Console\Input\ArgvInput($argv);
$output = new \Symfony\Component\Console\Output\ConsoleOutput();

// Replace Style with your own to change the output style.
$io = new ProvisionOps\Tools\Style($input, $output);

// Run any command.
$command = 'ls -la';
$process = new ProvisionOps\Tools\PowerProcess($command, $io);
$process->run();

$command = 'ps';
$process = new ProvisionOps\Tools\PowerProcess($command, $io);
$process->run();

$command = 'rm -rf /';
$process = new ProvisionOps\Tools\PowerProcess($command, $io);
$process->run();