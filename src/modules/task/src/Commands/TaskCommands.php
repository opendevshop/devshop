<?php

namespace Drupal\task\Commands;

use Consolidation\OutputFormatters\StructuredData\RowsOfFields;
use Drupal\Component\Datetime\Time;
use Drupal\Core\Cache\Cache;
use Drupal\node\Entity\Node;
use Drupal\task\Entity\Task;
use Drush\Commands\DrushCommands;
use Drush\Drush;
use Drush\Exceptions\CommandFailedException;
use Symfony\Component\Process\Exception\ProcessFailedException;
use Symfony\Component\Process\Process;

/**
 * A Drush commandfile.
 *
 * In addition to this file, you need a drush.services.yml
 * in root of your module, and a composer.json file that provides the name
 * of the services file to use.
 *
 * See these files for an example of injecting Drupal services:
 *   - http://cgit.drupalcode.org/devel/tree/src/Commands/DevelCommands.php
 *   - http://cgit.drupalcode.org/devel/tree/drush.services.yml
 */
class TaskCommands extends DrushCommands {


  /**
   * Run Task
   *
   * @param $id
   *   The ID of the task to run.
   * @param array $options
   *   An associative array of options whose values come from cli, aliases, config, etc.
   * @option force
   *   Force running the task again, regardless of exit state. (Not recommended. Will be replaced with "run-again".)
   * @usage task:run 123
   *   Enter the node ID of the task to run.
   *
   * @command task:run
   * @aliases t
   */
  public function taskRun($id, $options = [
    'force' => false,
  ]) {

    $task = Task::load($id);
    if (empty($task)) {
      throw new CommandFailedException(dt('No task found with that ID.'));
    }

    if ($task->state->value == Task::TASK_QUEUED || $options['force']) {
      $this->logger()->success(dt('Found Task in state:!state [!summary](!link) !command', [
        '!summary' => $task->label(),
        '!link' => $task->toUrl()->setAbsolute(true)->toString(),
        '!command' => $task->command->value,
        '!state' => $task->state->value,
      ]));

      if ($options['force']) {
        $this->logger()->warning(dt('The command is being run again because the "force" option was used.'));
      }

      $this->taskRunExecute($task);
    }
    elseif ($task->state->value == Task::TASK_PROCESSING) {
      $this->logger()->warning(dt('Task already running: !summary [!link].', [
        '!summary' => $task->label(),
        '!link' => $task->toUrl()->setAbsolute(true)->toString(),
      ]));
    }
    elseif ($task->state->value == Task::TASK_ERROR) {
      $this->logger()->warning(dt('Task already failed: !summary [!link].', [
        '!summary' => $task->label(),
        '!link' => $task->toUrl()->setAbsolute(true)->toString(),
      ]));
    }
    elseif ($task->state->value == Task::TASK_OK) {
      $this->logger()->warning(dt('Task already succeeded: !summary [!link].', [
        '!summary' => $task->label(),
        '!link' => $task->toUrl()->setAbsolute(true)->toString(),
      ]));
    }
    else {
      # @TODO: (maybe?) Print constant names too.
      throw new CommandFailedException(dt('Task state value is not found in DevShopTaskCommands. Possible states are: 0,1,2,3'));
    }
  }

  private function taskRunExecute(Task $task) {

    $this->logger()->info(dt('Task starting. Updated state. Command: @command', [
      '@command' => $task->command->value,
    ]));

    $task->set('state', Task::TASK_PROCESSING);
    $task->set('output', null);
    $task->set('executed', \Drupal::time()->getCurrentTime());

    // Unset values in case this task is being retried
    $task->set('finished', NULL);
    $task->set('duration', NULL);

    $task->setNewRevision();
    $task->save();

    // Turn off revisions for process run.
    $task->setNewRevision(FALSE);

    $args = explode(' ', $task->command->value);
    $process = Drush::process($args);

    $process->setWorkingDirectory($task->working_directory->value ?? getcwd());

    try {
      $process->mustRun(function ($type, $buffer) use ($task){

        // Echo output to the same stream the process returned.
        if (Process::ERR === $type) {
          $this->stderr()->write($buffer);
        } else {
          $this->output()->write($buffer);
        }

        // @TODO: Use insert queries directly? Test performance.
        $task->get('output')->appendItem([
          'output' => $buffer,
          'stream' => $type == Process::OUT ? Process::STDOUT: Process::STDERR,
        ]);
        $task->save();
      });
      $task->set('state', Task::TASK_OK);
      $task->setNewRevision();
      $task->set('finished', \Drupal::time()->getCurrentTime());
      $task->save();
      $this->logger()->info(dt('Task ended in Success. Updated state.'));
    }
    catch (ProcessFailedException $exception) {
      $task->set('state', Task::TASK_ERROR);
      $task->setNewRevision();
      $task->set('finished', \Drupal::time()->getCurrentTime());
      $task->save();
      $this->logger()->info(dt('Task ended in Failure. Updated state.'));
      throw $exception;
    }
  }

  /**
   * An example of the table output format.
   *
   * @param array $options An associative array of options whose values come from cli, aliases, config, etc.
   *
   * @field-labels
   *   group: Group
   *   token: Token
   *   name: Name
   * @default-fields group,token,name
   *
   * @command devshop_task:token
   * @aliases token
   *
   * @filter-default-field name
   * @return \Consolidation\OutputFormatters\StructuredData\RowsOfFields
   */
  public function token($options = ['format' => 'table']) {
    $all = \Drupal::token()->getInfo();
    foreach ($all['tokens'] as $group => $tokens) {
      foreach ($tokens as $key => $token) {
        $rows[] = [
          'group' => $group,
          'token' => $key,
          'name' => $token['name'],
        ];
      }
    }
    return new RowsOfFields($rows);
  }
}
