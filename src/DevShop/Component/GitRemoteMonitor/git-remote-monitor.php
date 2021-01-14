<?php
/**
 * @file git-remote-monitor.php
 * Executable file for the git-remote-monitor cli.
 *
 * The git-remote-monitor CLI is used by the grmd daemon service
 * so that the functionality is contained in simple Symfony Console & Robo commands.
 */

global $classLoader;
require 'bootstrap.php';

$appName = "Git Remote Monitor";
$appVersion = trim(file_get_contents(__DIR__ . '/VERSION'));

// Customization variables
$argv = $_SERVER['argv'];

// Load Application
$input = new \Symfony\Component\Console\Input\ArgvInput($argv);
$output = new \Symfony\Component\Console\Output\ConsoleOutput();
$app = new \DevShop\Component\GitRemoteMonitor\Application($input, $output, $classLoader);

// If called via daemon script (grmd), run Daemon::getInstance()->run();
$script_caller = realpath($_SERVER['SCRIPT_FILENAME']);
if (basename($script_caller) == 'grmd') {
    \DevShop\Component\GitRemoteMonitor\Daemon::setFilename($script_caller);
    \DevShop\Component\GitRemoteMonitor\Daemon::getInstance()->run();
}
else {
    // Execute robo Runner.
    $status_code = $app->execute($argv, $output);
    exit($status_code);
}

