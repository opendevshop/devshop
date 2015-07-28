<?php

namespace DevShop\Command;

use Symfony\Component\Console\Command\Command;
use Symfony\Component\Console\Input\ArrayInput;
use Symfony\Component\Console\Input\InputArgument;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Input\InputOption;
use Symfony\Component\Console\Output\OutputInterface;
use Symfony\Component\Console\Question\Question;
use Symfony\Component\Console\Question\ChoiceQuestion;
use Symfony\Component\Console\Question\ConfirmationQuestion;

use Symfony\Component\Process\Process;
use Symfony\Component\Process\Exception\ProcessFailedException;

use Github\Client;

use Symfony\Component\Filesystem\Filesystem;
use Symfony\Component\Filesystem\Exception\IOExceptionInterface;

class RemoteInstall extends Command
{
    protected function configure()
    {
        $this
          ->setName('remote:install')
          ->setDescription('Install a remote server and connect it to a devshop server.')
        ;
    }

    protected function execute(InputInterface $input, OutputInterface $output)
    {
        $formatter = $this->getHelper('formatter');
        $helper = $this->getHelper('question');
        $errorMessages = array(
            '╔═══════════════════════════════════════════════════════════════╗',
            '║           ____  Welcome to  ____  _                           ║',
            '║          |  _ \  _____   __/ ___|| |__   ___  _ __            ║',
            '║          | | | |/ _ \ \ / /\___ \|  _ \ / _ \|  _ \           ║',
            '║          | |_| |  __/\ V /  ___) | | | | (_) | |_) |          ║',
            '║          |____/ \___| \_/  |____/|_| |_|\___/| .__/           ║',
            '║               Remote Server Installer        |_|              ║',
            '╚═══════════════════════════════════════════════════════════════╝',
        );
        $formattedBlock = $formatter->formatBlock(
            $errorMessages,
            'fg=black;bg=green'
        );
        $output->writeln($formattedBlock);
        $output->writeln(
            'Welcome to the Remote Server Installer!'
        );
        $output->writeln('');

        // Ensure the command is being run on an existing devshop server, by looking for aegir user.
        $output->writeln(
            "<info>Root Access:</info> To provision your server, we need root access."
        );

        // Look for current user's public key. Throw an exception if we cannot find one.
        $key_file_path = $_SERVER['HOME'] . '/.ssh/id_rsa.pub';
        if (!file_exists($key_file_path)) {
            throw new \Exception("Unable to find a public key at '$key_file_path'. We must have an SSH keypair to provision the new server.");
        }

        $output->writeln(
            "<info>SSH Key Found:</info> Found an SSH key at <comment>$key_file_path</comment>"
        );
        $output->writeln('');

        // Display public key to user so they can add it to their server.
        $pubkey = file_get_contents($key_file_path);
        $output->writeln("To continue, add the following public key to the remote server at <comment>/root/.ssh/authorized_keys</comment>:");
        $output->writeln("<comment>$pubkey</comment>");

        // Ask for hostname
        $question = new Question("Remote hostname? ");
        while (empty($hostname)) {
            $hostname = $helper->ask($input, $output, $question);
            $ip = gethostbyname($hostname);
            if (empty($ip)) {
                $output->writeln(
                    "<error>WARNING: </error> Hostname must resolve to an IP address. Please try a new name or quit to fix your DNS."
                );
                $hostname = '';
                $output->writeln("");
            } else {
                $output->writeln(
                    "Remote server <info>$hostname</info> found at <info>$ip</info>"
                );
                $confirmationQuestion = new ConfirmationQuestion(
                    "Is this the correct IP? [y/N] "
                );

                if (!$helper->ask($input, $output, $confirmationQuestion)) {
                    $hostname = '';
                    $output->writeln("");
                } else {
                    $output->writeln("");
                }
            }
        }

        // Ask for remote username
        $question = new Question("Remote username (must have sudo access) [root]: ", 'root');
        while (empty($root_username)) {
          $root_username = $helper->ask($input, $output, $question, 'root');
        }

        // Confirm ability to SSH in as root.
        $access = FALSE;
        while ($access == FALSE) {
            $confirmationQuestion = new ConfirmationQuestion(
                "Ready to check access to <comment>$root_username@$hostname</comment> ? [Hit Enter to continue] ", true
            );

            if (!$helper->ask($input, $output, $confirmationQuestion)) {
                $output->writeln(
                  "<error>Remote Server Install Cancelled.</error>"
                );
                $output->writeln("");
                return;
            }
            $command = "ssh $root_username@$hostname -o 'PasswordAuthentication no' -C 'echo \$SSH_CLIENT'";

            $output->writeln("");
            $output->writeln(
                "<info>Access Test:</info> Running <comment>$command</comment> to test access..."
            );

            $process = new Process($command);
            $process->setTimeout(null);
            $process->run(
                function ($type, $buffer) {
                    echo $buffer;
                }
            );

            // Extract client IP from output.
            $ssh_client_output = $process->getOutput();
            $ssh_client = explode(' ', $ssh_client_output);
            $mysql_client_ip = array_shift($ssh_client);

            if ($ssh_client_output) {
                $output->writeln("");
                $output->writeln(
                    "<info>Access Granted!</info> SSH connection was successful.  Client IP detected: <comment>$mysql_client_ip</comment>"
                );
                $output->writeln("");
                $access = TRUE;
            } else {
                $output->writeln("");
                $output->writeln(
                    "<error>Access Denied:</error> Unable to access $root_username@$hostname.  Please check your keys and try again."
                );
                $output->writeln("");
            }
        }

        // Check for 'ansible' command.
        $process = new Process('which ansible-playbook');
        $process->run();
        if (!$process->getOutput()) {
            throw new \Exception("Command 'ansible-playbook' not found.  Please install ansible and try again. If you are running on a devshop server, ansible would already be installed.");
        }

        // Generate an inventory and run the ansible playbook.
        $fs = new Filesystem();

        try {
            $fs->dumpFile('/tmp/inventory-remote', $hostname);
        } catch (IOExceptionInterface $e) {
            throw new \Exception("Unable to write inventory-remote file.");
        }

        // Look for saved password
        $password_file_path = realpath("/var/aegir/.servers/$hostname-sql-password");

        if ($fs->exists($password_file_path)) {
            $mysql_password = trim(file_get_contents($password_file_path));
            $output->writeln("MySQL Password found at <comment>$password_file_path</comment>");
        }
        else {
            $mysql_password = $this->generatePassword();
            $output->writeln("MySQL Password generated as <comment>$mysql_password</comment>.");
        }
        $output->writeln("");

        $extra_vars = json_encode(array(
            'aegir_ssh_key' => $pubkey,
            'mysql_root_password' => $mysql_password,
            'server_hostname' => $hostname,
            'server_ip' => $ip,
            'mysql_client_ip' => $mysql_client_ip,
        ));

        $playbook_path = realpath(__DIR__ . '/../../../playbook-remote.yml');
        $command = "ansible-playbook -i /tmp/inventory-remote $playbook_path --extra-vars '$extra_vars'";

        $output->writeln("<info>Provision Server:</info> Run Ansible Playbook");
        $output->writeln("Run the following command? You may cancel and run the command manually now if you wish.");

        $confirmationQuestion = new ConfirmationQuestion(
            "<comment>$command</comment> [y/N] ", true
        );

        if ($helper->ask($input, $output, $confirmationQuestion)) {
            $process = new Process($command, null, array(
                'ANSIBLE_FORCE_COLOR' => 'true',
                'HOME' => $_SERVER['HOME'],
            ));
            $process->setTimeout(null);
            $process->run(
                function ($type, $buffer) {
                    echo $buffer;
                }
            );

            $output->writeln('');
        } else {
            $output->writeln("");
        }

        // Find remote apache control
        $process = new Process( "ssh $root_username@$hostname -o 'PasswordAuthentication no' -C 'cat /var/aegir/.apache-restart-command'");
        $process->setTimeout(null);
        $process->run();
        $apache_restart = $process->getOutput();

        if (empty($apache_restart)) {
            $apache_restart = 'Unable to determine. Run the provisioner to find the restart command.';
        }
        else {
            $apache_restart = "sudo $apache_restart graceful";
        }

        // Save a file with the server's mysql root password in case one runs the command again.
        if (!$fs->exists('/var/aegir/.servers')) {
            $fs->mkdir('/var/aegir/.servers', 0700);
        }
        $fs->dumpFile("/var/aegir/.servers/$hostname-sql-password", $mysql_password);

        $something_failed = FALSE;

        // Test MySQL Access
        $output->writeln('');
        $output->writeln('<info>MySQL:</info> Testing MySQL Access...');

        $cmd = "mysql -h $hostname -u aegir_root -p$mysql_password -e'CREATE DATABASE test_access; DROP DATABASE test_access;'";
        $process = new Process($cmd);
        $process->setTimeout(null);

        try {
            $process->mustRun();
            $output->writeln('<info>Access Granted!</info> MySQL on remote server is accessible.');

        } catch (ProcessFailedException $e) {
            $errorMessages = array_filter(explode("\n", $e->getMessage()));
            $formattedBlock = $formatter->formatBlock($errorMessages, 'error');
            $output->writeln($formattedBlock);
            $something_failed = TRUE;
        }

        // Test Apache Access
        $output->writeln('');
        $output->writeln('<info>Apache:</info> Testing Apache restart access...');

        $cmd = "ssh aegir@$hostname -t -o 'PasswordAuthentication no' -C '$apache_restart'";
        $process = new Process($cmd);
        $process->setTimeout(null);

        try {
            $process->mustRun();
            $output->writeln('<info>Access Granted!</info> Apache was restarted successfully.');

        } catch (ProcessFailedException $e) {
            $errorMessages = array_filter(explode("\n", $e->getMessage()));
            $formattedBlock = $formatter->formatBlock($errorMessages, 'error');
            $output->writeln($formattedBlock);
            $something_failed = TRUE;
        }

        $output->writeln('');

        if ($something_failed) {
            $output->writeln('<error>Aegir Remote Server did not complete!</error>');
        }
        else {
            $output->writeln('<info>Aegir Remote Server Setup Complete</info>');
        }

        $output->writeln("<comment>Hostname:</comment> $hostname");
        $output->writeln("<comment>MySQL username:</comment> aegir_root");
        $output->writeln("<comment>MySQL password:</comment> $mysql_password");
        $output->writeln("<comment>Apache Restart Command:</comment> $apache_restart");
        $output->writeln('');

        $output->writeln("<comment>WARNING:</comment> You must revoke access to  <comment>root@$hostname</comment> to secure your system.");

        $output->writeln('');
        $url = "http://" . php_uname('n') . '/node/add/server';
;
        $output->writeln("You must now add the server to the Aegir front-end.");
        $output->writeln("Run `devshop login` to login to the front-end, and visit $url to create a new server.");
    }

    /**
     * Generates a random password.
     *
     * Stolen from aegir provision_password.
     *
     * @param int $length
     * @return string
     */
    private function generatePassword($length = 16) {
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

            // Each iteration, pick a random character from the
            // allowable string and append it to the password:
            $pass .= $allowable_characters[mt_rand(0, $len)];
        }

        return $pass;
    }
}