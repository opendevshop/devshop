<?php

namespace DevShop\Command\Verify;

use DevShop\Console\Command;
use Symfony\Component\Console\Input\InputArgument;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Input\InputOption;
use Symfony\Component\Console\Output\OutputInterface;
use Symfony\Component\Console\Question\Question;
use Symfony\Component\Console\Question\ChoiceQuestion;
use Symfony\Component\Console\Question\ConfirmationQuestion;

use Symfony\Component\Process\Process;
use Asm\Ansible\Ansible;

class VerifySystem extends Command
{
    protected function configure()
    {
        $this
            ->setName('verify:system')
            ->setDescription('Run ansible playbooks to ensure the state of the system.')
            ->addArgument(
                'limit',
                InputArgument::OPTIONAL,
                'A pattern to limit the ansible playbook run. Use server hostname, service, or service type.'
            )
            ->addOption(
                'playbook',
                'p',
                InputOption::VALUE_REQUIRED,
                'The path to playbook.yml.',
                dirname(dirname(dirname(dirname(__DIR__)))). '/playbook.yml'
            )
            ->addOption(
                'inventory-file',
                'i',
                InputOption::VALUE_REQUIRED,
                'The path to an ansible inventory file or comma separated host list. Defaults to ansible configuration.',
                '/etc/ansible/hosts'
            )
            ->addOption(
                'user',
                'u',
                InputOption::VALUE_OPTIONAL,
                'Remote user: connect as this user. Defaults to currently acting user.'
            )
        ;
    }

    private $inventory;
    private $config_file;
    private $config;

    protected function initialize(InputInterface $input, OutputInterface $output) {

        // If "inventory" exists in ansible configuration, use that instead of the default '/etc/ansible/hosts'
        if ($this->getAnsibleInventory()) {

            $output->writeln('<info>Ansible Config Loaded</info> from ' . $this->config_file);

            $input->setOption('inventory-file', $this->getAnsibleInventory());
        }

        // Last check: does the inventory file exist?
        if (!file_exists($input->getOption('inventory-file'))) {
            throw new \Exception('No file was found at the path specified by the "inventory-file" option: ' . $input->getOption('inventory-file'));
        }

        // Last check: does the playbook file exist?
        if (!file_exists($input->getOption('playbook'))) {
            throw new \Exception('No file was found at the path specified by the "playbook" option: ' . $input->getOption('playbook'));
        }
    }

    protected function execute(InputInterface $input, OutputInterface $output)
    {
        parent::execute($input, $output);
        $ansible = $this->ansible->playbook();

        $ansible->play($input->getOption('playbook'));

        if ($input->getOption('user')) {
            $ansible->user($input->getOption('user'));
        }
        if ($input->getArgument('limit')) {
            $ansible->limit($input->getArgument('limit'));
        }

        if ($input->getOption('inventory-file')) {
            $ansible->inventoryFile($input->getOption('inventory-file'));
        }

        $ansible->execute(function ($type, $buffer) {
            print $buffer;
        });

    }

    /**
     * Return the inventory file from ansible configuration.
     *
     * @return string
     */
    protected function getAnsibleInventory() {
        if (!$this->inventory) {
            $this->config = $this->getAnsibleConfig();
            if (isset($this->config['inventory'])) {
                $this->inventory = $this->config['inventory'];
            }
        }
        return $this->inventory;
    }

    /**
     * Loads ansible configuration from the default ansible.cfg files.
     *
     * @see http://docs.ansible.com/ansible/intro_configuration.html
     *
     * @return array
     */
    protected function getAnsibleConfig() {
        $ansible_cfg[] = getenv('ANSIBLE_CONFIG');
        $ansible_cfg[] = getcwd() . '/ansible.cfg';
        $ansible_cfg[] = getenv('HOME') . '/.ansible.cfg';
        $ansible_cfg[] = '/etc/ansible/ansible.cfg';

        foreach ($ansible_cfg as $path) {
            if (file_exists($path)) {
                $file = @parse_ini_file($path);
                if (is_array($file)) {
                    $this->config_file = $path;
                    return $file;
                }
            }
        }
        return array();
    }
}