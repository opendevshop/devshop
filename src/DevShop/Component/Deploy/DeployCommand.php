<?php

/*
 * This file is part of the DevShop package.
 *
 * (c) Jon Pugh <jon@thinkdrop.net>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

namespace DevShop\Component\Deploy;

use Composer\Composer;
use Composer\Config;
use DevShop\Component\Common\ComposerRepositoryAwareTrait;
use DevShop\Component\Common\GitRepository;
use Robo\Common\OutputAwareTrait;
use Symfony\Component\Console\Command\Command as BaseCommand;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Input\InputOption;
use Symfony\Component\Console\Output\OutputInterface;
use Symfony\Component\Console\Style\SymfonyStyle;


/**
 * GitSplitConsoleCommand runs splitsh-lite script.
 *
 * @author Jon Pugh <jon@thinkdrop.net>
 */
class DeployCommand extends BaseCommand
{

    use ComposerRepositoryAwareTrait;

    /**
     * @var SymfonyStyle
     */
    private $io;


    /**
     * {@inheritdoc}
     */
    protected function configure()
    {
        $this
          ->setName('deploy')
          ->setDescription(
            'Run the deploy stages for this application, contingent on environment configuration and .'
          )
          ->setHelp(
            <<<'EOF'
The <info>%command.name%</info> runs the commands defined in a project's composer.json:extras.deploy.stages
EOF
          );

        $this
          ->addOption(
            'option',
            'o',
            InputOption::VALUE_OPTIONAL | InputOption::VALUE_IS_ARRAY,
            "Set deploy options in the format NAME=VALUE."
          );

        // Add CLI options for default stages.
        foreach ([false, true] as $skip) {
            $i = 0;
            foreach (DeployStages::getStages() as $stage_name => $description) {
                $i++;
                $this
                  ->addOption(
                    $skip
                      ? "skip-$stage_name"
                      : $stage_name
                    ,
                    null,
                    InputOption::VALUE_NONE,
                    $skip
                      ? "Skip Stage $i: $stage_name: $description"
                      : "Stage $i: $stage_name: $description"
                  );
            }
        }
    }

    /**
     * {@inheritdoc}
     */
    public function initialize(InputInterface $input, OutputInterface $output)
    {
        // Prepare object.
        $this->io = new SymfonyStyle($input, $output);
        $this->setComposerConfig();

        // Load the stages from the current composer project.
        if ($this->io->isVerbose()) {
            $this->io->section('Initializing Deploy Command');
            $verbose_rows[] = ['PWD', getenv('PWD')];
            $verbose_rows[] = [
              'Repository Root',
              $this->getRepository()->getRepositoryPath(),
            ];
            $verbose_rows[] = [
              'Composer Project',
              $this->getComposerConfig()->name(),
            ];
            $this->io->table(['Debug Information'], $verbose_rows);
        }
    }

    /**
     * {@inheritdoc}
     */
    protected function execute(InputInterface $input, OutputInterface $output)
    {
        $this->io->title('Deploy');

        // @TODO: Convert to use the Deploy class to load config from anywhere.
        // @TODO: Create common DeployTemplates so every project does not need extras.deploy.stages
        if (empty($this->getComposerConfig()->extra()->deploy)) {
            // @TODO: Allow Fail or Warn
            $this->io->warning(
              "No 'extra.deploy' section found in composer.json in directory " . $this->composerPath
            );
        }
        else {
            // @TODO: Create a simple config class so stage commands can be loaded from many places.
            $deploy_extra_config = $this->getComposerConfig()->extra()->deploy;

            // Pass all options as deploy options.
            foreach ($input->getOption('option') as $option) {
                [$name, $value] = explode('=', $option);
                $deploy->setOption($name, $value);
            }

            // Load all possible deploy stages
            $deploy_plan = [];
            $skipped_stages = [];

            // Load built in stage "git", but allow extra config to override it.
            $command = !empty($deploy_extra_config->stages->git)? $deploy_extra_config->stages->git: null;

            // @TODO: Move this logic to the Deploy Class.
            foreach (DeployStages::getStages() as $stage_name => $description) {

                // Check if --stage is set and --skip-stage is not and stage exists in "$deploy_extra_config";
                if ((Deploy::isDefaultStage($stage_name) || $input->getOption($stage_name)) && !$input->getOption("skip-{$stage_name}") && (!empty($deploy_extra_config->stages->{$stage_name}) || $stage_name == 'git')) {

                    // If this is the git state, use the DeployStageGit class.
                    if ($stage_name == 'git') {
                        $deploy->stages['git'] = new DeployStageGit(null, $command, $this->getRepository(), $deploy);
                        $deploy_plan['git'] = ['git', $deploy->stages['git']->getCommand()];
                    }
                    else {
                        $deploy->stages[$stage_name] = new DeployStage($stage_name, $deploy_extra_config->stages->{$stage_name}, $this->getRepository(), $deploy, $this->composerPath);
                        $deploy_plan[$stage_name] = [$stage_name, $deploy->stages[$stage_name]->getCommand()];
                    }
                }
                // Messages on why stage was skipped.
                // Because: --skip-stage was used
                elseif (Deploy::isDefaultStage($stage_name) && $input->getOption("skip-{$stage_name}")) {
                    $deploy_plan[$stage_name] = ["<fg=black>$stage_name</>", "<fg=black>Stage skipped: --skip-{$stage_name} option was used.</>"];
                    $skipped_stages[$stage_name] = $stage_name;
                }
                // Because: Stage is not default and --stage option was not set.
                elseif (!Deploy::isDefaultStage($stage_name) && !$input->getOption($stage_name)) {
                    $deploy_plan[$stage_name] = ["<fg=black>$stage_name</>", "<fg=black>Stage skipped: '{$stage_name}' is not a default stage and --{$stage_name} option was not used.</>"];
                }
                // Because: Specific stage was not found in config.
                elseif (empty($deploy_extra_config->stages->{$stage_name})){
                    $deploy_plan[$stage_name] = ["<fg=black>$stage_name</>", "<fg=black>Stage skipped: '{$stage_name}' stage command was not found in extras.deploy.stages.{$stage_name}.</>"];
                }
            }

            $remote = $this->getRepository()->getCurrentRemote();

            $this->io->section('Deploy Plan');
            $this->io->table([], [
              ['Remote', $remote['origin']['fetch']],
              ['Composer Project Path', $this->composerPath],
              ['Git Repo Path', $this->getRepository()->getRepositoryPath()],
              ['Git Branch', $this->getRepository()->getCurrentBranch()],
              ['Local Commit', $this->getRepository()->getLocalSha()],
              ['Remote Commit', $this->getRepository()->getRemoteSha()],
              ['Merge Base', $this->getRepository()->getMergeSha()],
            ]);

            switch ($this->getRepository()->remoteTrackingState()) {
              case GitRepository::REMOTE_TRACKING_STATE_BEHIND:
                $this->io->note('Remote git repo has new commits.');
                break;
              case GitRepository::REMOTE_TRACKING_STATE_AHEAD:
                if ($deploy->getOption('git_reset')) {
                  $this->io->warning('Local git repo has commits not pushed to the remote and the git_reset option was passed, so the working copy will be reset to the origin. Unpushed commits may be lost! Run "git push" in the repo path or "git reflog" to view all commits.');
                }
                else {
                  $this->io->warning('Local git repo has commits not pushed to the remote. The deployment will abort. Run "git push" or use deploy command option "--option=git_reset=1" to force the repo back to the remote SHA or "--skip-git" to skip the git stage and run the rest of the deploy.');
                }
                break;
            }

            $this->io->table(['Stages'], $deploy_plan);

            if (!empty($skipped_stages)) {
                $count = count($skipped_stages);
                $skipped_stages = implode(', ', $skipped_stages);
                $plural = $count == 0 || $count > 1? 'stages': 'stage';
                $this->io->note("Skipping $count default $plural: $skipped_stages");
            }


            if (empty($deploy->stages)) {
                $this->io->warning("There were no stages to run.");
            }
            else {

                // Confirm plan before executing.
                if ($input->isInteractive()) {
                    if (!$this->io->confirm('Execute Deploy Plan? Press [enter] to continue, [n] or [CTRL+C] to cancel.', true)) {
                        $this->io->text('Deploy Cancelled');
                        return 1;
                    }
                }

                $deploy->runStages();
            }
            return 0;
        }

        return 1;
    }
}
