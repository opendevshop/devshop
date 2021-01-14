<?php

namespace DevShop\Component\GitRemoteMonitor;

use Robo\Tasks;
use SebastianBergmann\Diff\Differ;
use Symfony\Component\Process\Exception\LogicException;
use Symfony\Component\Process\Exception\ProcessFailedException;
use Symfony\Component\Process\Exception\ProcessSignaledException;
use Symfony\Component\Process\Exception\ProcessTimedOutException;
use Symfony\Component\Process\Process;
use Symfony\Component\Yaml\Yaml;

class Commands extends Tasks
{

  /**
   * Print the list of remotes to monitor, one per line. Derived from config "remotes" or "remotes_callback".
   */
    public function remotes()
    {
        $remotes = $this->getRemotes();
        echo implode(PHP_EOL, $remotes);
        return 0;
    }

  /**
   * Get an array of remotes from grm config.
   * @return string[]
   */
    public function getRemotes()
    {
      /** @var Robo\Config\Config $config */
        $config = $this->getContainer()->get('config');

      // Look for remotes
      // GRM_REMOTES
        $remotes = $config->get('remotes');

      // GRM_REMOTES_CALLBACK
        if (empty($remotes)) {
          // Execute remotes.callback to return list of remotes.
            $callback = $config->get('remotes_callback');

            if (empty($callback)) {
                throw new LogicException('No "remotes" or "remotes_callback" configuration options found. At least one config value must be set.');
            }
            $remotes = trim(shell_exec($callback));
            if (empty($remotes)) {
                throw new LogicException("Remote callback ($callback) returned nothing.");
            }
        }

      // Always return an array.
        if (is_string($remotes)) {
            $remotes = explode(PHP_EOL, $remotes);
        }
        return array_unique($remotes);
    }

  /**
   * Display the current state of the GitRemoteMonitor command, such as configuration.
   * @command status
   */
    public function status()
    {
        $this->io()->section('Git Remote Monitor');
        $this->io()->title('Status');
        return 0;
    }

  /**
   * Show differences between last stored list of refs and the current list.
   *
   * @arg $git_remote The URL of the remote repository.
   * @option timeout The length of time to let the process run until timeout.
   *
   * @return int Returns 0 and prints the diff if current refs is different from the stored refs.
   */
    public function referencesDiff($git_remote, $opts = [
    'timeout' => 60,
    ])
    {

        $command = ['git', 'ls-remote', $git_remote];
        $process = new Process($command);
        $process->setTimeout($opts['timeout']);

        try {
            $process->mustRun();
        } catch (ProcessSignaledException $e) {
            $this->io()->error("Command signalled.");
            $this->io()->write($e->getMessage());
            return $process->getExitCode();
        } catch (ProcessTimedOutException $e) {
            $this->io()->error("Command timed out.");
            $this->io()->write($e->getMessage());
            return $process->getExitCode();
        } catch (ProcessFailedException $e) {
            $this->io()->error("Command failed.");
            $this->io()->write($e->getMessage());
            return $process->getExitCode();
        }

      // Save to file.
        $references = $process->getOutput();
        $config = $this->getContainer()->get('config');
        $yml_file_path = $config->get('remotes_save_path', $_SERVER['HOME'] . '/.grm/remotes/') . GitRemote::getSlug($git_remote) . '.yml';
        $yml_file_dir = dirname($yml_file_path);

        if (!file_exists($yml_file_dir)) {
            mkdir($yml_file_dir, 0744, true);
        }

      // Compare to existing file.
        if (file_exists($yml_file_path)) {
            $existing_refs = trim(file_get_contents($yml_file_path));
            $new_refs = trim($references);
            if (empty($existing_refs) || empty($new_refs)) {
                $this->io()->error('New or old references were empty:');
                $this->io()->write("Existing: $existing_refs");
                $this->io()->write("New: $new_refs");
                return 2;
            }
            if ($existing_refs == $new_refs) {
                $this->io()->warning('No new references detected.');
                return 1;
            } else {
                $differ = new Differ();
                $this->io()->section('+++ New changes loaded at ' . date('r'));
                $this->io()->writeln($differ->diff($existing_refs, $new_refs));
                file_put_contents($yml_file_path, $process->getOutput());

                // Sleep to allow subsequent calls catch up with the filesystem.
                sleep(2);
                return 0;
            }
        } else {
            $this->io()->warning('First scan. No new references.');
            file_put_contents($yml_file_path, $process->getOutput());
            return 1;
        }
    }
}
