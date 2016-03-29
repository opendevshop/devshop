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
                'The path to playbook.yml.  Defaults to ANSIBLE_PLAYBOOK environment variable.',
                getenv('ANSIBLE_PLAYBOOK')
            )
            ->addOption(
                'inventory-file',
                'i',
                InputOption::VALUE_OPTIONAL,
                'The path to an ansible inventory file or comma separated host list. Defaults to ansible configuration.'
            )
            ->addOption(
                'user',
                'u',
                InputOption::VALUE_OPTIONAL,
                'Remote user: connect as this user. Defaults to currently acting user.'
            )
        ;
    }

    /**
     * @var Ansible;
     */
    private $ansible;

    protected function initialize(InputInterface $input, OutputInterface $output) {

        // If inventory-file option was not specified, try to derive it from ansible.cfg
        if (empty($input->getOption('inventory-file'))) {
            $input->setOption('inventory-file', $this->getAnsibleInventoryFromConfig());

            // If inventory file is still empty, look for the default.
            if (empty($input->getOption('inventory-file'))) {

                if (file_exists('/etc/ansible/hosts')) {
                    $input->setOption('inventory-file', '/etc/ansible/hosts');
                }
                else {
                    throw new \Exception('No inventory-file option found. Pass `-i` or `--inventory-file` option or set an ansible.cfg file with the `inventory` option.');
                }
            }
        }

        // Last check: does the inventory file exist?
        if (!file_exists($input->getOption('inventory-file'))) {
            throw new \Exception('No file was found at the path specified by the "inventory-file" option: ' . $input->getOption('inventory-file'));
        }

        // Prepare the Ansible object.
        $this->ansible = new Ansible(
            getcwd(),
            '/usr/bin/ansible-playbook',
            '/usr/bin/ansible-galaxy'
        );
    }

    protected function execute(InputInterface $input, OutputInterface $output)
    {
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
    protected function getAnsibleInventoryFromConfig() {

        $config = $this->getAnsibleConfig();

        if (isset($config['inventory']) && file_exists($config['inventory'])) {
            return $config['inventory'];
        }
        else {
            return '';
        }
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
        $ansible_cfg[] = 'ansible.cfg';
        $ansible_cfg[] = '.ansible.cfg';
        $ansible_cfg[] = '/etc/ansible/ansible.cfg';

        foreach ($ansible_cfg as $path) {
            if (file_exists($path)) {
                $file = @parse_ini_file($path);

                if (is_array($file)) {
                    return $file;
                }
            }
        }
        return array();
    }
}