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
 * -
 * $date: Formatted creation date (use $created to reformat with
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

<!-- STATUS/INFO -->
<div id="project-info">
  <ul class="list-inline">
    <?php if ($status != NODE_PUBLISHED): ?>
        <li>
            <strong>Status</strong>
            <small><?php print t('Disabled') ; ?></small>
        </li>
    <?php endif; ?>
    <?php if ($project->settings->live['live_domain']): ?>
    <li>
      <strong>Live Site</strong>
      <small><a href="http://<?php print $project->settings->live['live_domain']; ?>" target="_blank">http://<?php print $project->settings->live['live_domain']; ?></a></small>
    </li>
    <?php endif; ?>
    <li>
      <strong>Install Profile</strong>
      <small><?php print $project->install_profile ?></small>
    </li>
    <li data-toggle="tooltip" data-placement="bottom" title="<?php print t('The source code for each environment will be placed into subfolders of this folder.');?>">
      <strong>Base Path</strong>
      <small><?php print $project->code_path ?></small>
    </li>

    <!-- Drush Info -->
    <li class="pull-right">
      <button type="button" class="btn btn-xs btn-link text-muted" data-toggle="modal" data-target="#drush-alias-modal" title="Drush Aliases">
        <i class="fa fa-drupal"></i>
        <?php print t('Drush'); ?>
      </button>

      <!-- Modal -->
      <div class="modal fade" id="drush-alias-modal" tabindex="-1" role="dialog" aria-labelledby="drush-alias-modal" aria-hidden="true">
        <div class="modal-dialog">
          <div class="modal-content">
            <div class="modal-header">
              <button type="button" class="close" data-dismiss="modal"><span aria-hidden="true">&times;</span><span class="sr-only">Close</span></button>
              <h4 class="modal-title" id="drush-alias-modal">Project Drush Aliases</h4>
            </div>
            <div class="modal-body">

              <!-- Download button -->
              <p>
                <a href="<?php print $aliases_url; ?>" class="btn btn-primary"><?php print t('Download Alias File'); ?></a> or copy to <code>~/.drush/<?php print $project->name; ?>.aliases.drushrc.php</code>.
              </p>

              <textarea cols="40" rows="10" class='form-control' onlick="this.select()"><?php print $drush_aliases; ?></textarea>

              <p>
                <?php print $access_note; ?>
              </p>

            </div>
            <div class="modal-footer">
              <button type="button" class="btn btn-default" data-dismiss="modal">Close</button>
            </div>
          </div>
        </div>
      </div>
    </li>
    <li>
        <!-- Webhook -->
        <?php if ($project->settings->deploy['method'] == 'webhook'): ?>


      <strong><?php print t('Last Commit'); ?></strong>
          <small>
          <?php if (empty($project->settings->deploy['last_webhook'])): ?>
            <!-- Not Received -->
            <span class="text-danger"><i class="fa fa-warning"></i> <?php print t('Not Received'); ?></span>
          <?php elseif ($project->settings->deploy['last_webhook_status'] == DEVSHOP_PULL_STATUS_ACCESS_DENIED): ?>
            <!-- Last Received -->
            <span class="text-danger">
              <i class="fa fa-warning"></i> <?php print t('Access Denied'); ?>
            </span>
            <a href="<?php print url('admin/hosting/git')?>">
              <?php print t('Configure Webhook Access'); ?>
            </a>
          <?php else: ?>
            <!-- Last Received -->
            <span title="<?php print t('Last webhook receieved.'); ?>"><?php print $webhook_ago; ?></span>
          <?php endif; ?>
          </small>

        <?php elseif ($project->settings->deploy['method'] == 'queue'): ?>
        <!-- Queue -->
        <strong><?php print t('Queue'); ?>:</strong>
        <small>
          <?php print $queued_ago; ?>
        </small>
          <?php if (user_access('administer hosting queues')): ?>
              <?php print $hosting_queue_admin_link; ?>
          <?php endif; ?>
        <?php endif; ?>
    </li>

    <!-- Webhook -->
    <?php if ($project->settings->deploy['method'] == 'webhook'):

        $float = empty($project->settings->deploy['last_webhook'])? 'inline': 'pull-right';
      ?>
      <li class="<?php print $float; ?>"><?php print $webhook_url; ?></li>
    <?php endif; ?>
    
    <?php // Extra items to allow modules to add things. ?>
    <?php foreach ($project_extra_items as $item): ?>
      <li><?php print $item; ?></li>
    <?php endforeach; ?>

  </ul>
</div>

<?php if (isset($project_messages) && count($project_messages)): ?>
  <?php foreach ($project_messages as $message): ?>
    <div class="alert alert-<?php print $message['type']; ?>">
      <?php print $message['icon']; ?>
      <?php print $message['message']; ?>
    </div>
  <?php endforeach; ?>
<?php endif; ?>

<!-- ENVIRONMENTS-->
<div class="row">
    <?php foreach ($environments as $environment): ?>

        <div class="environment-wrapper col-xs-12 col-sm-6 col-md-4 col-lg-3">
            <?php print $environment; ?>
        </div>

    <?php endforeach; ?>

  <?php if (drupal_valid_path("node/add/site/$project->name")): ?>
  <div class="environment-wrapper placeholder add-project-button col-xs-12 col-sm-6 col-md-4 col-lg-3">
    <a href="<?php print url("node/add/site/$project->name"); ?>" class="btn btn-lg btn-success">
      <i class="fa fa-plus-square"></i><br />
      <?php print t('Create New Environment'); ?></a>
  </div>
  <?php endif; ?>
</div>

<div class="drupal-content">
  <?php
    print render($content);
  ?>
</div>
