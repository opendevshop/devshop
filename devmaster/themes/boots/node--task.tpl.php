<?php
/**
 * @file node.tpl.php
 *
 * Theme implementation to display a node.
 *
 * Available variables:
 * - $title: the (sanitized) title of the node.
 * - $content: Node body or teaser depending on $teaser flag.
 * - $picture: The authors picture of the node output from
 *   theme_user_picture().
 * - $date: Formatted creation date (use $created to reformat with
 *   format_date()).
 * - $links: Themed links like "Read more", "Add new comment", etc. output
 *   from theme_links().
 * - $name: Themed username of node author output from theme_username().
 * - $node_url: Direct URL of the current node.
 * - $terms: the themed list of taxonomy term links output from theme_links().
 * - $submitted: themed submission information output from
 *   theme_node_submitted().
 *
 * Other variables:
 * - $node: Full node object. Contains data that may not be safe.
 * - $type: Node type, i.e. story, page, blog, etc.
 * - $comment_count: Number of comments attached to the node.
 * - $uid: User ID of the node author.
 * - $created: Time the node was published formatted in Unix timestamp.
 * - $zebra: Outputs either "even" or "odd". Useful for zebra striping in
 *   teaser listings.
 * - $id: Position of the node. Increments each time it's output.
 *
 * Node status variables:
 * - $teaser: Flag for the teaser state.
 * - $page: Flag for the full page state.
 * - $promote: Flag for front page promotion state.
 * - $sticky: Flags for sticky post setting.
 * - $status: Flag for published status.
 * - $comment: State of comment settings for the node.
 * - $readmore: Flags true if the teaser content of the node cannot hold the
 *   main body content.
 * - $is_front: Flags true when presented in the front page.
 * - $logged_in: Flags true when the current user is a logged-in member.
 * - $is_admin: Flags true when the current user is an administrator.
 *
 * @see template_preprocess()
 * @see template_preprocess_node()
 */
?>
<div id="node-<?php print $node->nid; ?>" class="node<?php if ($sticky) { print ' sticky'; } ?><?php if (!$status) { print ' node-unpublished'; } ?> clear-block">

  <?php print $user_picture ?>

  <div class="list-group">
    <div id="task-info" class="task-info list-group-item">
      <div class="btn-group pull-right" role="group" aria-label="Actions">

        <!-- Terminal Tasks modal -->
        <button type="button" class="btn btn-text" data-toggle="modal" data-target="#exampleModal">
          <small><i class="fa fa-terminal"></i>
            <?php print t('Run from Terminal'); ?>
          </small>
        </button>

        <?php if (isset($retry)): ?>
              <?php print render($retry); ?>
        <?php endif; ?>
        <?php if (isset($cancel_button)): ?>
          <?php print render($cancel_button); ?>
        <?php endif; ?>
        <?php if ($node->task_status != HOSTING_TASK_QUEUED && $node->task_status != HOSTING_TASK_PROCESSING && isset($run_again)): ?>
          <?php print render($run_again); ?>
        <?php endif; ?>
      </div>

    <h4>

      <div class="task-badge pull-left">
        <span class="label label-default label-<?php print $task_label_class ?> task-status"><?php print $task_label ?></span>
      </div>

       <a href="<?php print $node_url ?>" title="<?php print $title ?>"><?php print $type_name ?></a>
    </h4>

    <div class="pull-right">
      <?php  if (isset($follow_checkbox)): ?>
        <?php print $follow_checkbox; ?>
      <?php endif; ?>
    </div>

    <p>
      <span class="duration">
          <i class="fa fa-clock-o"></i>
          <span class="duration-text">
            <?php print $duration; ?>
          </span>
      </span>
      <span>&nbsp;</span>
      <span class="executed inline">
          <i class="fa fa-calendar-o"></i>
          <?php print $date; ?>
          <small><time class="timeago" datetime="<?php print $node->task_timestamp ?>"></time></small>
      </span>
    </p>
    <div class="task-urls btn-group-xs">
      <?php foreach ($environment->domains as $domain): ?>
          <a class="btn btn-text btn-xs" href="<?php print 'http://' . $domain; ?>" target="_blank">
              <i class="fa fa-globe"></i> <?php print $domain; ?>
          </a>
      <?php endforeach; ?>

    <?php if (isset($task_well)): ?>
      <?php print $task_well; ?>
    <?php endif; ?>
    </div>

    <?php if (count($task_args)): ?>
    <div class="task-args">
    <?php foreach (array_filter($task_args) as $arg => $value): ?>
      <?php
      if ($value === '1') {
        $value = '';
        $arg = '<i class="fa fa-check"></i>' . $arg;
      }
      ?>
      <span class="task-arg small text-muted">
        <strong><?php print $arg; ?></strong>
        <span>
          <?php print $value; ?>
        </span>
      </span>
    <?php endforeach; ?>
    </div>
    </div>
  <?php endif; ?>

  </div>

    <!-- Modal -->
    <div class="modal fade" id="exampleModal" tabindex="-1" role="dialog" aria-labelledby="exampleModalLabel" aria-hidden="true">
        <div class="modal-dialog" role="document">
            <div class="modal-content">
                <div class="modal-header">
                    <h5 class="modal-title" id="exampleModalLabel">Run Task</h5>
                    <button type="button" class="close" data-dismiss="modal" aria-label="Close">
                        <span aria-hidden="true">&times;</span>
                    </button>
                </div>
                <div class="modal-body">
                  <?php if ($node->task_status == HOSTING_TASK_QUEUED): ?>
                    <?php print t('To run this task, run the following command on the DevShop server:'); ?>
                    <kbd class="alert">
                        drush @hm hosting-task <?php print $node->nid; ?>
                    </kbd>
                  <?php elseif ($node->task_status != HOSTING_TASK_QUEUED): ?>
                    <?php foreach ($node->task_args as $i => $v){
                      $name = escapeshellarg($i);
                      $value = escapeshellarg($v);
                      $args[] = "$name=$value";
                    } ?>
                    <?php print t('This task has already started. To run this task again, run the following command on the DevShop server.'); ?>
                    <kbd class="alert">
                        drush @hm hosting-task @<?php print $node->ref->hosting_name; ?> <?php print $node->task_type;  ?> <?php print implode(' ', $args); ?>
                    </kbd>
                  <?php endif; ?>
                </div>
                <div class="modal-footer">
                    <button type="button" class="btn btn-secondary" data-dismiss="modal">Close</button>
                </div>
            </div>
        </div>
    </div>

    <div id='task-logs'>
        <?php print $messages; ?>
    </div>

    <div class="running-indicator <?php print isset($is_active) ? $is_active : ''; ?>  text-muted small">
      <?php if (isset($is_running)): ?>
        <i class="fa fa-gear <?php print $is_running; ?>"></i> <span class="running-label"><?php print $running_label; ?></span>
      <?php endif; ?>
    </div>

    <div class="task-details">
        <button class="btn btn-default" type="button" data-toggle="collapse" data-target="#collapseLogs" aria-expanded="false" aria-controls="collapseLogs">
            <i class="fa fa-list"></i> <?php print t('Details'); ?>
        </button>
        <div class="collapse" id="collapseLogs">
            <div class="well">
                <?php print render($content['hosting_log']); ?>
            </div>
        </div>
    </div>

  <?php print isset($links) ? $links : ''; ?>
</div>
