#!/usr/bin/env php
<?php

// Include autoloader
include('vendor/autoload.php');

// PowerProcess needs IO.
$input = new \Symfony\Component\Console\Input\ArgvInput($argv);
$output = new \Symfony\Component\Console\Output\ConsoleOutput();

// Replace Style with your own to change the output style.
$io = new DevShop\Component\PowerProcess\PowerProcessStyle($input, $output);

// Run any command.
$command = 'ls -la';
$process = new DevShop\Component\PowerProcess\PowerProcess($command, $io);
$process->run();

// Output comes back in real-time.
$command = 'ping packagist.org -c 5';
$process = new DevShop\Component\PowerProcess\PowerProcess($command, $io);
$process->run();

$command = 'rm -rf /';
$process = new DevShop\Component\PowerProcess\PowerProcess($command, $io);
$process->run();