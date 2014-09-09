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

  <ul class="nav nav-pills project-stuff">
    <!-- Drush Aliases -->
    <li class="dropdown">
      <a href="#" class="dropdown-toggle" data-toggle="dropdown" title="<?php print t('Drush Aliases'); ?>">
        <i class="fa fa-drupal"></i>
        <span class="caret"></span>
      </a>
      <div class="dropdown-menu dropdown-menu-right">
        <textarea>
          <?php print $drush_aliases; ?>
        </textarea>
      </div>
    </li>
    <!-- TASKS -->
    <li class="dropdown">
      <a href="#" class="dropdown-toggle" data-toggle="dropdown" title="<?php print t('Tasks'); ?>">
        <i class="fa fa-cog <?php print $spinner; ?>"></i> <?php print "0"; ?>
        <span class="caret"></span>
      </a>
      <div class="dropdown-menu dropdown-menu-right">
        TASKS (coming soon.)
      </div>
    </li>
  </ul>

<nav class="navbar navbar-default navbar-project" role="navigation">
  <div class="container-fluid">
    <!-- First Links -->
    <div class="nav navbar-text main-project-nav">
      <ul class="nav nav-pills">

        <!-- Dashboard -->
        <li><a href='<?php print url("node/$node->nid"); ?>' class="active"><i class="fa fa-cubes"></i> <?php print t('Dashboard'); ?></a></li>

        <!-- Settings -->
        <?php if (node_access('update', $node)): ?>
          <li><a href='<?php print url("node/$node->nid/edit"); ?>'><i class="fa fa-sliders"></i> <?php print t('Settings'); ?></a></li>
        <?php endif; ?>

        <!-- Logs-->
        <?php if (user_access('access project logs')): ?>
          <li><a href='<?php print url("node/$node->nid/logs"); ?>'><i class="fa fa-list-alt"></i> <?php print t('Logs'); ?></a></li>
        <?php endif; ?>

      </ul>
    </div>

    <!-- Git Info -->
    <div class="navbar-form navbar-right form-group">
      <div class="input-group">

        <!-- Link to github or an icon -->
        <?php if (isset($github_url)): ?>
        <a class="input-group-addon" href="<?php print $github_url; ?>" title="<?php print t('View on GitHub'); ?>" target="_blank"><i class="fa fa-github-alt"></i></a>
        <?php else: ?>
          <div class="input-group-addon"><i class="fa fa-git"></i></div>
        <?php endif; ?>


        <!-- Git URL -->
        <input type="text" class="form-control" size="26" value="<?php print $node->project->git_url; ?>" onclick="this.select()">

        <!-- Branch & Tag List -->
        <div class="input-group-btn">
          <button type="button" class="btn btn-default dropdown-toggle <?php print $branches_class ?>" data-toggle="dropdown" title="<?php print $branches_label; ?>">

            <?php if ($branches_show_label): ?>
              <i class="fa fa-<?php print $branches_icon; ?>"></i>
              <?php print $branches_label; ?>
            <?php else: ?>
              <small>
                <i class="fa fa-code-fork"></i> <?php print $branches_count; ?>
              </small>

              &nbsp;
              <?php if ($tags_count): ?>
                <small>
                  <i class="fa fa-tag"></i> <?php print $tags_count; ?>
                </small>
              <?php endif; ?>

            <?php endif; // branches_show label ?>

            <span class="caret"></span></button>
          <ul class="dropdown-menu dropdown-menu-right" role="menu">
            <?php foreach ($branches_items as $item): ?>
              <li><?php print $item; ?></li>
            <?php endforeach; ?>
          </ul>
        </div>
      </div>
    </div>
  </div>
</div>
</nav>

<!-- STATUS/INFO -->
<div id="project-info" class="col-md-12">
  <ul class="list-inline">
    <li>
      <strong>Install Profile</strong>
      <small><?php print $project->install_profile ?></small>
    </li>
    <li>
      <strong>Last Commit</strong>
      <?php if ($node->pull_status != DEVSHOP_PULL_STATUS_OK): ?>
        <small class="text-danger">
          <?php print $node->pull_message; ?>
        </small>
      <?php else: ?>
        <small><?php print hosting_format_interval($project->settings->pull['last_pull']); ?></small>
      <?php endif; ?>
    </li>
  </ul>
</div>

<!-- ENVIRONMENTS-->
<div class="row placeholders col-md-12">
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

  <div class="col-xs-12 col-sm-6 col-md-4 col-lg-3">

    <div class="list-group devshop-environment <?php print $environment_class ?>">
      <a href="<?php print $environment->url ?>" target="_blank" class="site-link list-group-item list-group-item-<?php print $list_item_class ?>"  data-toggle="tooltip" data-placement="bottom" title="<?php print t('Visit !url', array('!url' => $environment->url)); ?>">



        <?php if ($environment->settings->production_mode): ?>
        <i class="fa fa-lock pull-right" title="Production Mode"></i>
        <?php endif; ?>

        <?php if ($environment->name == $project->settings->live['live_environment']): ?>
        <i class="fa fa-bolt pull-right" title="Live Environment"></i>
        <?php endif; ?>


        <strong><?php print $environment->name; ?></strong>

        <small class="environment-meta-data">
          <i class='fa fa-<?php print $environment->git_ref_type == 'tag'? 'tag': 'code-fork'; ?>'></i> <?php print $environment->git_ref; ?>
        </small>

        <?php if ($environment->version): ?>
        <small class="environment-meta-data" title="Drupal version <?php print $environment->version; ?>">
          <i class="fa fa-drupal"></i>
          <?php print $environment->version; ?>
        </small>
        <?php endif; ?>

        <?php if ($environment->site_status == HOSTING_SITE_DISABLED): ?>
          <span class="environment-meta-data">Disabled</span>
        <?php endif; ?>

         <br />
        <small class="text-muted"><?php print $environment->url ?></small>


        <div class="progress">
          <div class="progress-bar <?php print $environment->progress_classes ?>"  role="progressbar" aria-valuenow="100" aria-valuemin="0" aria-valuemax="100" style="width: 100%">
            <span class="sr-only">45% Complete</span>
          </div>
        </div>
      </a>
      <div class="environment-tasks list-group-item btn-group btn-group-justified">

        <!-- Git Select -->
        <div class="btn-group btn-git">
          <button type="button" class="btn btn-default dropdown-toggle btn-git-ref" data-toggle="dropdown"><i class="fa fa-code"></i>

            <?php print t('Deploy'); ?>

            <span class="caret"></span>
          </button>
          <ul class="dropdown-menu btn-git-ref" role="menu">
            <li><p class="text-muted"><?php print $deploy_label; ?></p></li>

            <?php if (count($git_refs)): ?>
            <li class="divider"></li>

            <?php foreach ($git_refs as $item): ?>
              <li>
                <?php print str_replace('ENV', $environment->name, $item); ?>
              </li>
            <?php endforeach; ?>
            <?php endif; ?>
          </ul>
        </div>

        <!-- Tasks -->
        <div class="btn-group btn-tasks">
          <button type="button" class="btn btn-default dropdown-toggle btn-git-ref" data-toggle="dropdown">
            <i class="fa fa-tasks" ></i>
              <?php print t('Tasks'); ?>
            <span class="caret"></span>
          </button>
          <ul class="dropdown-menu">
            <?php foreach ($node->environment_actions[$environment->name] as $link): ?>
              <li>
                <a href="<?php print $link['url'] ?>"><?php print $link['title']; ?></a>
              </li>
            <?php endforeach; ?>
            <li class="divider"></li>
            <li class="text-muted"><?php print t('Sync Data:'); ?></li>

            <?php foreach ($project->environments as $env): ?>
              <?php if ($env->settings->production_mode || $env->name == $environment->name) continue; ?>
              <li><a href="/node/<?php print $node->nid ?>/project_devshop-sync/?source=<?php print $environment->name ?>&dest=<?php print $env->name ?>"><?php print t('Copy data to') . ' ' . $env->name; ?></a></li>
            <?php endforeach; ?>



          </ul>
        </div>


        <!-- Settings -->
        <div class="btn-group btn-settings">
          <a href="<?php print url('node/' . $node->nid . '/edit/' . $environment->name, array('query'=> drupal_get_destination())); ?>" class="btn btn-default">
            <i class="fa fa-sliders" ?></i> Settings
          </a>
        </div>
      </div>
      <!-- Logs, Errors, commits -->
      <ul class="list-group-item nav nav-pills nav-justified">
        <li><a href="<?php print url("node/$environment->site/logs/commits"); ?>">Commits</a></li>
        <li><a href="<?php print url("node/$environment->site/logs/errors"); ?>">Errors</a></li>
        <li><a href="<?php print url("node/$environment->site/files/platform"); ?>">Files</a></li>
      </ul>

      <!-- Tasks -->
      <div class="tasks-button">
        <?php print $environment->tasks_list; ?>
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
