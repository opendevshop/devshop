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
use Symfony\Component\Yaml\Yaml;

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
            ->addOption(
                'connection',
                'c',
                InputOption::VALUE_OPTIONAL,
                'Connection: connection type to use.',
                'local'
            )
        ;
    }

    private $inventory;
    private $config_file;
    private $config;

    private $group_vars_file = '/etc/ansible/group_vars/devmaster';
    private $ansible_cfg_file = '/etc/ansible/ansible.cfg';

    /**
     * @var bool Tell Command::execute() that we require ansible.
     */
//    protected $ansibleRequired = TRUE;

    protected function initialize(InputInterface $input, OutputInterface $output) {

        parent::initialize($input, $output);

        // If "inventory" exists in ansible configuration, use that instead of the default '/etc/ansible/hosts'
        if ($this->getAnsibleInventory()) {

            $output->writeln('<info>Ansible Config Loaded</info> from ' . $this->config_file);

            $input->setOption('inventory-file', $this->getAnsibleInventory());
        }

        // If ansible.cfg file does not exist, create it.
        if (!file_exists($this->ansible_cfg_file)) {
          $source = dirname(dirname(dirname(dirname(__DIR__)))) . '/ansible.cfg';
          $destination = $this->ansible_cfg_file;
          copy($source, $destination);
        }

        // If inventory file does not exist, create it.
        if (!file_exists($input->getOption('inventory-file')) || strpos(file_get_contents($input->getOption('inventory-file')), '[devmaster]') === FALSE) {

          if (!file_exists($input->getOption('inventory-file'))) {
            $this->IO->warning('Ansible inventory file does not exist at ' . $input->getOption('inventory-file'));
          }
          else {
            $this->IO->warning('Ansible inventory file located at ' . $input->getOption('inventory-file') . ' does not contain [devmaster] group.');
          }
          if (!$input->isInteractive() || $this->IO->confirm('Create a new inventory file at ' . $input->getOption('inventory-file'))) {

            if (!file_exists(dirname($input->getOption('inventory-file')))) {
              mkdir(dirname($input->getOption('inventory-file')));
              $this->IO->success('Created folder ' . dirname($input->getOption('inventory-file')));
            }

            $vars['server_hostname'] = $hostname = trim(shell_exec('hostname -f'));

            $inventory_contents = <<<TXT
[devmaster]
$hostname
TXT;
            file_put_contents($input->getOption('inventory-file'), $inventory_contents);
            $this->IO->success('Wrote inventory file for ' . $hostname . ' to ' . $input->getOption('inventory-file'));
          }
        }

        // If group_vars file does not exist, create it.
        if (!file_exists($this->group_vars_file)) {

          // Lookup necessary variables
          $vars['aegir_user_uid'] = trim(shell_exec('id aegir -u'));

          if (file_exists('/root/.my.cnf')) {
            $vars['mysql_root_password'] = trim(shell_exec('awk -F "=" \'/pass/ {print $2}\' /root/.my.cnf'));
          }
          else {
            $vars['mysql_root_password'] = trim(shell_exec('echo $MYSQL_ROOT_PASSWORD'));
          }

          if (empty($vars['mysql_root_password'])) {
            $this->IO->warning('Unable to determine MySQL root password from /root/.my.cnf or MYSQL_ROOT_PASSWORD environment variable.');
            $vars['mysql_root_password'] = $this->IO->ask('Please provide the MySQL Root User Password');
          }

          if (file_exists('/var/aegir/config/server_master/nginx.conf')) {
            $vars['aegir_server_webserver'] = 'nginx';
          }
          else {
            $vars['aegir_server_webserver'] = 'apache';
          }

          if ($input->isInteractive()) {
            $this->IO->writeln('Confirm Ansible Variables:');
            foreach ($vars as $name => $value) {
              $vars[$name] = $this->ask($name, $value);
            }
          }

          // Dump YML to group vars
          if (!file_exists($this->group_vars_file)) {
            if (!file_exists('/etc/ansible/group_vars')) {
              mkdir('/etc/ansible/group_vars');
              $this->IO->success('Created folder /etc/ansible/group_vars');
            }
            file_put_contents($this->group_vars_file, Yaml::dump($vars));
            $this->IO->success('Wrote variables file to /etc/ansible/group_vars/devmaster');
          }
        }

        if (file_exists($input->getOption('inventory-file'))) {
          $this->IO->writeln('Found Ansible inventory at ' . $input->getOption('inventory-file'));
        }
        else {
          throw new \Exception('No file was found at the path specified by the "inventory-file" option: ' . $input->getOption('inventory-file'));
        }

        // Last check: does the playbook file exist?
        if (file_exists($input->getOption('playbook'))) {
          $this->IO->writeln('Found Ansible playbook at ' . $input->getOption('playbook'));
        }
        else {
          throw new \Exception('No file was found at the path specified by the "playbook" option: ' . $input->getOption('playbook'));
        }

        if (file_exists($this->group_vars_file)) {
          $this->IO->writeln("Found Ansible vars file at $this->group_vars_file");
          $this->IO->writeln(file_get_contents($this->group_vars_file));
        }
        else {
          throw new \Exception("No vars file was found at the path /etc/ansible/group_vars/devmaster. This file is managed by this script, and should have been written automatically but wasn't.");
        }
      }

    protected function execute(InputInterface $input, OutputInterface $output)
    {
        parent::execute($input, $output);
        if (!$this->ansible) {
          throw new \Exception('Ansible not loaded. Unable to find ansible-galaxy or ansible-playbook in the PATH.');
        }

        // Install devshop roles
        $roles_file_path = realpath(dirname(dirname(dirname(dirname(__DIR__)))) . '/roles.yml');
        $output->writeln('Installing Ansible roles from ' . $roles_file_path . ' ...');
        $this->ansible->galaxy()
          ->roleFile($roles_file_path)
          ->rolesPath('/etc/ansible/roles')
          ->force()
          ->install()
          ->execute(function ($type, $buffer) {
            echo $buffer;
          });

        $this->output->writeln("<comment>Ansible Galaxy install complete.</comment>");

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

        // Set connection from option. Defaults to "local"
        // @TODO: This command really can handle all servers, so might want to change the default.
        $ansible->connection($input->getOption('connection'));

        $result = $ansible->execute(function ($type, $buffer) {
            print $buffer;
        });

        if ($result !== 0) {
          throw new \Exception('Ansible playbook run failed.');
        }
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