<?php

namespace DevShop\Command;

use Symfony\Component\Console\Command\Command;
use Symfony\Component\Console\Input\InputArgument;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Input\InputOption;
use Symfony\Component\Console\Output\OutputInterface;
use Symfony\Component\Console\Question\Question;
use Symfony\Component\Console\Question\ChoiceQuestion;
use Symfony\Component\Console\Question\ConfirmationQuestion;

use Symfony\Component\Process\Process;
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
            '<info>Welcome to the Remote Server Installer!</info>'
        );
        $output->writeln('');

        // Ensure the command is being run on an existing devshop server, by looking for aegir user.
        $output->writeln(
            "<info>Find aegir public key:</info> Aegir uses SSH to connect to remote servers."
        );

        $users = file_get_contents('/etc/passwd');
        if (strpos($users, 'aegir') === false) {
            $output->writeln(
                '<comment>WARNING:</comment> aegir user doesn\'t exist.'
            );

            // Ask for public key file to use for remote install.
            $default_path = $_SERVER['HOME'].'/.ssh/id_rsa.pub';
            $question = new Question(
                "Would you like to use a different public key file? This key will be used to connect to 'aegir' user on the remote host. [$default_path] ",
                $default_path
            );
            $key_file = $helper->ask($input, $output, $question);

            if (empty($key_file) || !file_exists(realpath($key_file))) {
                throw new \Exception(
                    'Unable to continue: Public SSH key not found at '.$key_file
                );
            }
        } // If for some reason aegir ssh key doesn't exist...
        elseif (!file_exists("/var/aegir/.ssh/id_rsa.pub")) {
            $output->writeln(
                '<comment>WARNING:</comment> aegir user public key not found at /var/aegir/.ssh/id_rsa.pub'
            );

            // Ask for public key file to use for remote install.
            $default_path = $_SERVER['HOME'].'/.ssh/id_rsa.pub';
            $question = new Question(
                "Would you like to use a different public key file? This key will be used to connect to 'aegir' user on the remote host. [$default_path] ",
                $default_path
            );
            $key_file = $helper->ask($input, $output, $question);
        } // If aegir user exists and public key exists, load it.
        else {
            $key_file = "/var/aegir/.ssh/id_rsa.pub";
        }

        $output->writeln("Public Key found at <info>$key_file</info>");
        $output->writeln('');

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

        // Ask for root access
        $output->writeln(
            "<info>Check root access:</info> In order to provision the server, we need root access to it."
        );

        $question = new Question("Remote root user? [root] ", 'root');
        $root_username = $helper->ask($input, $output, $question);

        // Ask for private key file to use for remote install.
        $default_path = $_SERVER['HOME'].'/.ssh/id_rsa';
        $question = new Question(
            "Private SSH Key for $root_username@$hostname? [$default_path] ",
            $default_path
        );
        $private_key_file = $helper->ask($input, $output, $question);
        $output->writeln("");

        // Confirm ability to SSH in as root.
        $command = "ssh $root_username@$hostname -o 'PasswordAuthentication no' -C 'echo \$SSH_CLIENT'";
        $output->writeln(
            "Running <comment>$command</comment> to test access..."
        );

        $process = new Process($command);
        $process->setTimeout(null);
        $process->run(
            function ($type, $buffer) {
                echo $buffer;
            }
        );

        $ssh_client_output = $process->getOutput();
        $ssh_client = explode(' ', $ssh_client_output);
        $mysql_client_ip = array_shift($ssh_client);

        if ($ssh_client_output) {
            $output->writeln(
                "<info>SUCCESS</info> SSH connection was successful.  Client IP detected: <comment>$mysql_client_ip</comment>"
            );
        } else {
            throw new \Exception("Unable to access $root_username@$hostname using the private key file at $private_key_file.");
        }

        // Check for 'ansible' command.
        $process = new Process('which ansible-playbook');
        $process->run();
        if ($process->getOutput()) {
            $output->writeln(
                "<info>SUCCESS</info> Command 'ansible-playbook' found."
            );
        }
        else {
            throw new \Exception("Command 'ansible-playbook' not found.  Please install ansible and try again. If you are running on a devshop server, ansible would already be installed.");
        }

        // Generate an inventory and run the ansible playbook.
        $fs = new Filesystem();

        try {
            $fs->dumpFile('inventory-remote', $hostname);
        } catch (IOExceptionInterface $e) {
            throw new \Exception("Unable to write inventory-remote file.");
        }

        $mysql_password = $this->generatePassword();

        $extra_vars = json_encode(array(
            'aegir_ssh_key' => file_get_contents($key_file),
            'mysql_root_password' => $mysql_password,
            'server_hostname' => $hostname,
            'mysql_client_ip' => $mysql_client_ip,
        ));

        $command = "ansible-playbook -i inventory-remote playbook-remote.yml --extra-vars '$extra_vars'";

        $confirmationQuestion = new ConfirmationQuestion(
            "Run the command <comment>$command</comment> ? [y/N] ", false
        );

        if ($helper->ask($input, $output, $confirmationQuestion)) {
            $process = new Process($command);
            $process->setTimeout(null);
            $process->run(
                function ($type, $buffer) {
                    echo $buffer;
                }
            );
        } else {
            $output->writeln("<info>Generated MySQL password:</info> $mysql_password");
            $output->writeln("<warning>Aegir Remote not installed.</warning> Run the above command to provision the server, then create the server in the front-end.");
            return;
        }

        // Find remote apache control
        $process = new Process( "ssh $root_username@$hostname -o 'PasswordAuthentication no' -C 'cat /var/aegir/.apache-restart-command'");
        $process->setTimeout(null);
        $process->run();
        $apache_restart = $process->getOutput();

        $output->writeln('');
        $output->writeln('<info>Aegir Remote Server Install was successful!</info>');
        $output->writeln("You must now add the server to the Aegir front-end.");

        $output->writeln("<comment>Hostname:</comment> $hostname");
        $output->writeln("<comment>MySQL username:</comment> aegir_root");
        $output->writeln("<comment>MySQL password:</comment> $mysql_password");
        $output->writeln("<comment>Apache Restart Command:</comment> $apache_restart");

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