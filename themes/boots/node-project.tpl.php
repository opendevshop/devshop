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
    <li>
    <?php if ($project->settings->deploy['method'] == 'manual'): ?>
      <strong><?php print t('Manual Deployment Only'); ?></strong>
    <?php else: ?>
    <li>
        <!-- Webhook -->
        <?php if ($project->settings->deploy['method'] == 'webhook'): ?>


      <strong><?php print t('Webhook'); ?></strong>
          <small>
          <?php if (empty($project->settings->deploy['last_webhook'])): ?>
            <!-- Not Received -->
            <span class="text-danger"><i class="fa fa-warning"></i> <?php print t('Not Received'); ?></span>
          <?php elseif ($project->settings->deploy['last_webhook_status'] == DEVSHOP_PULL_STATUS_ACCESS_DENIED): ?>
            <!-- Last Received -->
            <span class="text-danger">
              <i class="fa fa-warning"></i> <?php print t('Access Denied'); ?>
            </span>
            <a href="<?php print url('admin/hosting/devshop/pull')?>">
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
    <?php endif; ?>
  </ul>
</div>

<!-- ENVIRONMENTS-->
<div class="row">
<?php foreach ($node->project->environments as $environment_name => $environment): ?>

  <?php
  if ($environment->site_status == HOSTING_SITE_DISABLED){
    $environment_class = 'disabled';
    $list_item_class = 'disabled';
  }
  elseif ($environment->name == $project->settings->live['live_environment']){
    $environment_class = ' live-environment';
    $list_item_class = 'info active';
  }
  else {
    $environment_class = '';
    $list_item_class = 'info';
  }

  // Active?
  if ($environment->active_tasks > 0) {
    $environment_class .= ' active';
    $list_item_class = 'warning';
  }
  ?>

  <div class="environment-wrapper col-xs-12 col-sm-6 col-md-4 col-lg-3">

    <div class="list-group environment <?php print $environment_class ?>">
      <div class="environment-header list-group-item list-group-item-<?php print $list_item_class ?>">


        <!-- Environment Tasks -->
        <div class="environment-tasks pull-right">
          <?php print $environment->tasks_list; ?>
        </div>

        <!-- Environment Status Indicators -->
        <div class="environment-indicators pull-right">
          <?php if ($environment->settings->locked): ?>
            <i class="fa fa-lock" title="Locked"></i>
          <?php endif; ?>

          <?php if ($environment->name == $project->settings->live['live_environment']): ?>
            <i class="fa fa-bolt" title="Live Environment"></i>
          <?php endif; ?>
        </div>

        <!-- Environment Links -->
        <a href="<?php print $environment->site? url("node/$environment->site"): url("node/$environment->platform"); ?>" class="environment-link">
          <?php print $environment->name; ?></a>

        <a href="<?php print url("node/$environment->site/logs/commits"); ?>" class="environment-meta-data btn btn-text">
          <i class='fa fa-<?php print $environment->git_ref_type == 'tag'? 'tag': 'code-fork'; ?>'></i><?php print $environment->git_ref; ?>
        </a>

        <?php if ($environment->version): ?>
          <a href="<?php print url("node/$environment->platform"); ?>"  title="Drupal version <?php print $environment->version; ?>" class="environment-meta-data btn btn-text">
          <i class="fa fa-drupal"></i><?php print $environment->version; ?>
        </a>

        <?php endif; ?>

        <?php if ($environment->site_status == HOSTING_SITE_DISABLED): ?>
          <span class="environment-meta-data">Disabled</span>
        <?php endif; ?>

        <div class="progress">
          <div class="progress-bar <?php print $environment->progress_classes ?>"  role="progressbar" aria-valuenow="100" aria-valuemin="0" aria-valuemax="100" style="width: 100%">
            <span class="sr-only"><?php print $environment->progress_output ?></span>
          </div>
        </div>
      </div>
      <!-- URLs -->
      <div class="environment-domains list-group-item btn-group btn-group-justified">
        <div class="btn-group">
          <?php if (count($environment->domains) > 1): ?>

            <a type="button" class="btn btn-xs" href="<?php print $environment->url ?>" target="_blank">
              <i class="fa fa-globe"></i> <?php print $environment->url ?>
            </a>
            <button type="button" class="btn btn-xs dropdown-toggle pull-right" data-toggle="dropdown" aria-expanded="false">
              <i class="fa fa-globe"></i>
              <?php print count($environment->domains); ?>
              <span class="caret"></span>
              <span class="sr-only">Domains</span>
            </button>
          <?php else: ?>
            <?php if (!empty($environment->url)): ?>
              <a type="button" class="btn btn-xs" href="<?php print $environment->url ?>" target="_blank">
                <i class="fa fa-globe"></i>
                <?php print $environment->url ?>
              </a>
            <?php else: ?>
              <button class="btn btn-xs">
                <i class="fa fa-globe"></i>
                <em>&nbsp;</em>
              </button>
            <?php endif;?>
            <a type="button" class="btn btn-xs pull-right" href="<?php print url('node/' . $node->nid . '/edit/' . $environment->name, array('query'=> drupal_get_destination())); ?>" title="<?php print t("Add Domain"); ?>">
              <i class="fa fa-plus"></i>
            </a>
          <?php endif;?>

          <?php if (count($environment->domains) > 1): ?>
          <ul class="dropdown-menu pull-right" role="menu">
            <?php foreach ($environment->domains as $domain): ?>
            <li><a href="<?php print 'http://' . $domain; ?>" target="_blank"><?php print 'http://' . $domain; ?></a></li>
            <?php endforeach; ?>
            <li class="divider">&nbsp;</li>
            <li><?php print l(t('Edit Domains'), 'node/' . $node->nid . '/edit/' . $environment->name, array('query'=> drupal_get_destination())); ?></li>
          </ul>
          <?php endif; ?>
        </div>
      </div>


      <div class="environment-deploy list-group-item">

        <!-- Deploy: Git Select -->
        <label><?php print t('Deploy'); ?></label>
        <div class="btn-group btn-toolbar" role="toolbar">
          <div class="btn-group btn-deploy-code" role="group">
            <button type="button" class="btn btn-default dropdown-toggle btn-git-ref" data-toggle="dropdown"><i class="fa fa-code"></i>
              <?php print t('Code'); ?>
              <span class="caret"></span>
            </button>
            <ul class="dropdown-menu btn-git-ref" role="menu">
              <li><label><?php print t('Deploy a branch or tag'); ?></label></li>
              <?php if (count($git_refs)): ?>
              <?php foreach ($git_refs as $ref => $item): ?>
                <li>
                  <?php print str_replace('ENV_NID', $environment->site, $item); ?>
                </li>
              <?php endforeach; ?>
              <?php endif; ?>
            </ul>
          </div>
          <div class="btn-group btn-deploy-database" role="group">

            <button type="button" class="btn btn-default dropdown-toggle" data-toggle="dropdown"><i class="fa fa-database"></i>
              <?php print t('Data'); ?>
              <span class="caret"></span>
            </button>
            <ul class="dropdown-menu" role="menu">
              <li><label><?php print t('Deploy data from'); ?></label></li>
              <?php if (count($project->environments) == 1): ?>
              <li><p><?php print t('No other environments to deploy data from.'); ?></p></li>
              <?php endif; ?>
                <?php foreach ($project->environments as $source): ?>
                  <?php if ($env->settings->locked || $source->name == $environment->name) continue; ?>
                  <li><a href="/node/<?php print $environment->site ?>/site_sync/?source=<?php print $source->name ?>&dest=<?php print $source->name ?>"><?php print $source->name ?> <small><?php print $source->url; ?></small></a></li>
                <?php endforeach; ?>
            </ul>
          </div>
          <div class="btn-group btn-deploy-servers" role="group">

            <button type="button" class="btn btn-default dropdown-toggle" data-toggle="dropdown"><i class="fa fa-bars"></i>
              <?php print t('Stack'); ?>
              <span class="caret"></span>
            </button>
            <ul class="dropdown-menu" role="menu">
                <?php foreach ($environment->servers as $type => $server):
                  if ($type == 'db') {
                    $icon = 'database';
                  }
                  elseif ($type == 'http') {
                    $icon = 'cube';
                  }
                  elseif ($type == 'solr') {
                    $icon = 'sun';
                  }
                  ?>
                  <li>
                    <a href="/node/<?php print $server['nid'] ?>" title="<?php print $type .' '. t('server') .' '. $server['name']; ?>">
                      <i class="fa fa-<?php print $icon; ?>"></i>
                      <?php print $type; ?>
                      <small><?php print $server['name']; ?></small>
                    </a>
                  </li>
                <?php endforeach; ?>
            </ul>
          </div>
        </div>
      </div>
        <div class="list-group-item">
          <!-- Settings -->
          <a href="<?php print url('node/' . $node->nid . '/edit/' . $environment->name, array('query'=> drupal_get_destination())); ?>" class="btn btn-default pull-right settings">
            <i class="fa fa-sliders" ?></i> Settings
          </a>

          <a href="<?php print url("node/$environment->site/logs/commits"); ?>" class="last-commit">
            <?php print $environment->git_current; ?>
          </a>
        </div>

      </div>
    </div>
<?php endforeach; ?>

  <div class="placeholder add-project-button col-xs-12 col-sm-6 col-md-4 col-lg-3">
    <a href="/node/<?php print $node->nid; ?>/project_devshop-create" class="btn btn-lg btn-success">
      <i class="fa fa-plus-square"></i><br />
      <?php print t('Create New Environment'); ?></a>
  </div>
</div>
