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
                'playbook_path',
                'p',
                InputOption::VALUE_OPTIONAL,
                'The path to playbook.yml.  Defaults to the playbook.yml file in the Aegir Ansible module in devmaster.',
                $this->findPlaybookPath()
            )
        ;
    }

    private $ansible;

    protected function initialize(InputInterface $input, OutputInterface $output) {

        if (!file_exists($this->getApplication()->getDevmasterRoot() . '/profiles/devmaster/modules/aegir/aegir_ansible')) {
            throw new \Exception('The "Aegir Ansible Inventory" module is not installed in DevMaster');
        }

        $this->ansible = new Ansible(
            $this->getApplication()->getDevmasterRoot() . '/profiles/devmaster/modules/aegir/aegir_ansible/aegir_ansible_inventory',
            '/usr/bin/ansible-playbook',
            '/usr/bin/ansible-galaxy'
        );
    }

    protected function execute(InputInterface $input, OutputInterface $output)
    {
        $limit = $input->getArgument('limit');
        $this->ansible
            ->playbook()
            ->play('playbook.yml')
            ->user('root')
            ->limit($limit)
            ->inventoryFile('ansible-inventory.php')
            ->execute(function ($type, $buffer) {
                print $buffer;
            });

    }
}