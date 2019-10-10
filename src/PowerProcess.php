<?php

namespace ProvisionOps\Tools;

use Psr\Log\LoggerAwareTrait;
use Robo\Common\OutputAwareTrait;
use Robo\Common\TimeKeeper;
use Symfony\Component\Console\Output\Output;
use Symfony\Component\Console\Style\SymfonyStyle;
use Symfony\Component\Process\Process as BaseProcess;

class PowerProcess extends BaseProcess {

    /**
     * @var Style
     */
    public $io;
    public $successMessage = 'Process Succeeded';
    public $failureMessage = 'Process Failed';
    public $duration = '';

    /**
     * @param $io Style
     */
    public function setIo(SymfonyStyle $io) {
        $this->io = $io;
    }

    /**
     * PowerProcess constructor.
     * @param $commandline
     * @param Style $io
     * @param null $cwd
     * @param array|null $env
     * @param null $input
     * @param int $timeout
     * @param array $options
     */
    public function __construct($commandline, $io, $cwd = null, array $env = null, $input = null, $timeout = 60, array $options = array())
    {
        $this->setIo($io);
        parent::__construct($commandline, $cwd, $env, $input, $timeout, $options);
    }


    /**
     * @param null $callback
     * @return int
     */
    public function run($callback = null)
    {
        $this->io->write(" <comment>$</comment> {$this->getCommandLine()} <fg=black>Output:/path/to/file</>");

        $timer = new TimeKeeper();
        $timer->start();

        $exit = parent::run(function ($type, $buffer) {
            $lines = explode("\n", $buffer);
            foreach ($lines as $line) {
              if (getenv('PROVISION_PROCESS_OUTPUT') == 'direct') {
                echo $line . PHP_EOL;
                // @TODO: detect EOL and add only if needed.
              }
              else {
                $this->io->outputBlock(trim($line), false, false);
              }
            }
        });
        $timer->stop();
        $this->duration = $timer->formatDuration($timer->elapsed());

        if ($exit == 0) {
            $this->io->newLine();
            $this->io->writeln(" <info>✔</info> {$this->successMessage} in {$this->duration} <fg=black>Output: /path/to/file</>");
        }
        else {
            $this->io->newLine();
            $this->io->writeln(" <fg=red>✘</> {$this->failureMessage} in {$this->duration} <fg=black>Output: /path/to/file</>");
        }

        return $exit;

    }
}
