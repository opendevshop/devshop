#!/usr/bin/env php
<?php

// Include autoloader
function includeIfExists(string $file): bool
{
    return file_exists($file) && include $file;
}

if (
    !includeIfExists(__DIR__ . '/../../autoload.php') &&
    !includeIfExists(__DIR__ . '/vendor/autoload.php') &&
    !includeIfExists(__DIR__ . '/../../../../vendor/autoload.php')
) {
    fwrite(STDERR, 'Dependencies not found. Install with Composer.'.PHP_EOL);
    exit(1);
}

// PowerProcess needs IO.
$input = new \Symfony\Component\Console\Input\ArgvInput($argv);
$output = new \Symfony\Component\Console\Output\ConsoleOutput();

// Replace Style with your own to change the output style.
$io = new DevShop\Component\PowerProcess\PowerProcessStyle($input, $output);

// Run any command.
$command = 'ls -la';
$process = new DevShop\Component\PowerProcess\PowerProcess($command, $io);
$process->mustRun();

// Output comes back in real-time.
$command = 'ping packagist.org -c 5';
$process = new DevShop\Component\PowerProcess\PowerProcess($command, $io);
$process->mustRun();

$command = 'rm -rf /';
$process = new DevShop\Component\PowerProcess\PowerProcess($command, $io);

try {
    $process->mustRun();
}
catch (\Symfony\Component\Process\Exception\ProcessFailedException $e) {
    $io->customLite("The 'rm -rf /' command exited, which throws an exception, but demo.php caught it so the script will still return a successful exit code. Here's the Exception message: ", '!');
    echo $e->getMessage();
}
