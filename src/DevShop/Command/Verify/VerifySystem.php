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

    private $host_vars_path = '/etc/ansible/host_vars/';
    private $group_vars_file = '/etc/ansible/group_vars/devmaster';
    private $ansible_cfg_file = '/etc/ansible/ansible.cfg';

    /**
     * @var bool Tell Command::execute() that we require ansible.
     */
    protected $ansibleRequired = TRUE;

    protected function initialize(InputInterface $input, OutputInterface $output) {

      parent::initialize($input, $output);

      // Announce ourselves.
      $output->writeln($this->getApplication()->getLogo());
      $this->announce('Verify System');
      $this->checkCliVersion();

      $output->writeln(
        '<info>Confirming Ansible inventory and variables...</info>'
      );

      // If "inventory" exists in ansible configuration, use that instead of the default '/etc/ansible/hosts'
        if ($this->getAnsibleInventory()) {

            $output->writeln('<info>Ansible Config Loaded</info> from ' . $this->config_file);

            $input->setOption('inventory-file', $this->getAnsibleInventory());
        }

        // If ansible.cfg file does not exist, create it.
        if (!file_exists($this->ansible_cfg_file)) {
          $this->IO->note('Ansible config file does not exist at ' . $this->ansible_cfg_file);

          $source = dirname(dirname(dirname(dirname(__DIR__)))) . '/ansible.cfg';
          $destination = $this->ansible_cfg_file;

          if (!$input->isInteractive() || $this->IO->confirm('Copy DevShop ansible.cfg to ' . $destination . '?')) {
              $this->FS->copy($source, $destination);
              $this->IO->success('Wrote ansible.cfg file to ' . $this->ansible_cfg_file);

          }
        }

        // Set server_hostname variable.
        $vars['server_hostname'] = $hostname = trim(shell_exec('hostname -f'));

        // If inventory file does not exist, or is not executable and does not contain [devmaster] create it.
        if (!file_exists($input->getOption('inventory-file')) || strpos(file_get_contents($input->getOption('inventory-file')),'[devmaster]') === FALSE && !is_executable($input->getOption('inventory-file'))) {

          if (!file_exists($input->getOption('inventory-file'))) {
            $this->IO->note('Ansible inventory file does not exist at ' . $input->getOption('inventory-file'));
          }
          else {
            $this->IO->note('Ansible inventory file located at ' . $input->getOption('inventory-file') . ' does not contain [devmaster] group.');
          }
          if (!$input->isInteractive() || $this->IO->confirm('Create a new inventory file at ' . $input->getOption('inventory-file'))) {

            if (!file_exists(dirname($input->getOption('inventory-file')))) {
              $this->FS->mkdir(dirname($input->getOption('inventory-file')));
              $this->IO->success('Created folder ' . dirname($input->getOption('inventory-file')));
            }

            $inventory_contents = <<<TXT
[devmaster]
$hostname
TXT;
            $this->FS->dumpFile($input->getOption('inventory-file'), $inventory_contents);
            $this->IO->success('Wrote inventory file for ' . $hostname . ' to ' . $input->getOption('inventory-file'));
          }
        }

        // If group_vars file does not exist, create it.
        if (!file_exists($this->group_vars_file)) {

          // Lookup necessary variables
          $vars['devshop_version'] = $this->getApplication()->getVersion();
          $vars['aegir_user_uid'] = trim(shell_exec('id aegir -u'));
          if (empty($vars['aegir_user_uid'])) {
            $this->IO->note('Unable to determine aegir user UID. Defaulting to 12345');
            $vars['aegir_user_uid'] = 12345;
          }

          if (file_exists('/root/.my.cnf')) {
            $vars['mysql_root_password'] = trim(shell_exec('awk -F "=" \'/pass/ {print $2}\' /root/.my.cnf'), " \t\n\r\0\x0B\"'");
          }

          // @TODO: Detect existing installation and load mysql root user and pasword from there.
          //          elseif (file_exists('/var/aegir/.drush/server_master.drushrc.php')) {
          //
          //          }
          else {
            $vars['mysql_root_password'] = trim(shell_exec('echo $MYSQL_ROOT_PASSWORD'));
          }

          if (empty($vars['mysql_root_password'])) {
            $this->IO->note("Unable to determine MySQL root password from /root/.my.cnf or MYSQL_ROOT_PASSWORD environment variable. 
If this is an existing installation, you must enter the correct mysql root password.
If this is a new installation, you may select the default randomly generated password.
            ");
            $vars['mysql_root_password'] = $this->user_password(32);
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
            $this->FS->dumpFile($this->group_vars_file, Yaml::dump($vars));
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
        $output->writeln('Installing Ansible roles from ' . $roles_file_path . ' to /etc/ansible/roles ...');
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

  /**
   * Generate a random alphanumeric password.
   * Based on Drupal's user_password()
   */
  function user_password($length = 10) {
    // This variable contains the list of allowable characters for the
    // password. Note that the number 0 and the letter 'O' have been
    // removed to avoid confusion between the two. The same is true
    // of 'I', 1, and 'l'.
    $allowable_characters = 'abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ23456789';

    // Zero-based count of characters in the allowable list:
    $len = strlen($allowable_characters) - 1;

    // Declare the password as a blank string.
    $pass = '';

    // Loop the number of times specified by $length.
    for ($i = 0; $i < $length; $i++) {
      do {
        // Find a secure random number within the range needed.
        $index = ord($this->drupal_random_bytes(1));
      } while ($index > $len);

      // Each iteration, pick a random character from the
      // allowable string and append it to the password:
      $pass .= $allowable_characters[$index];
    }

    return $pass;
  }

  /**
   * Returns a string of highly randomized bytes (over the full 8-bit range).
   *
   * This function is better than simply calling mt_rand() or any other built-in
   * PHP function because it can return a long string of bytes (compared to < 4
   * bytes normally from mt_rand()) and uses the best available pseudo-random
   * source.
   *
   * @param $count
   *   The number of characters (bytes) to return in the string.
   */
  function drupal_random_bytes($count)  {
    // $random_state does not use drupal_static as it stores random bytes.
    static $random_state, $bytes, $has_openssl;

    $missing_bytes = $count - strlen($bytes);

    if ($missing_bytes > 0) {
      // PHP versions prior 5.3.4 experienced openssl_random_pseudo_bytes()
      // locking on Windows and rendered it unusable.
      if (!isset($has_openssl)) {
        $has_openssl = version_compare(PHP_VERSION, '5.3.4', '>=') && function_exists('openssl_random_pseudo_bytes');
      }

      // openssl_random_pseudo_bytes() will find entropy in a system-dependent
      // way.
      if ($has_openssl) {
        $bytes .= openssl_random_pseudo_bytes($missing_bytes);
      }

      // Else, read directly from /dev/urandom, which is available on many *nix
      // systems and is considered cryptographically secure.
      elseif ($fh = @fopen('/dev/urandom', 'rb')) {
        // PHP only performs buffered reads, so in reality it will always read
        // at least 4096 bytes. Thus, it costs nothing extra to read and store
        // that much so as to speed any additional invocations.
        $bytes .= fread($fh, max(4096, $missing_bytes));
        fclose($fh);
      }

      // If we couldn't get enough entropy, this simple hash-based PRNG will
      // generate a good set of pseudo-random bytes on any system.
      // Note that it may be important that our $random_state is passed
      // through hash() prior to being rolled into $output, that the two hash()
      // invocations are different, and that the extra input into the first one -
      // the microtime() - is prepended rather than appended. This is to avoid
      // directly leaking $random_state via the $output stream, which could
      // allow for trivial prediction of further "random" numbers.
      if (strlen($bytes) < $count) {
        // Initialize on the first call. The contents of $_SERVER includes a mix of
        // user-specific and system information that varies a little with each page.
        if (!isset($random_state)) {
          $random_state = print_r($_SERVER, TRUE);
          if (function_exists('getmypid')) {
            // Further initialize with the somewhat random PHP process ID.
            $random_state .= getmypid();
          }
          $bytes = '';
        }

        do {
          $random_state = hash('sha256', microtime() . mt_rand() . $random_state);
          $bytes .= hash('sha256', mt_rand() . $random_state, TRUE);
        }
        while (strlen($bytes) < $count);
      }
    }
    $output = substr($bytes, 0, $count);
    $bytes = substr($bytes, $count);
    return $output;
  }
}