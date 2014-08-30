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

<nav class="navbar navbar-default navbar-project" role="navigation">
  <div class="container-fluid">
    <!-- First Links -->
    <ul class="nav navbar-nav">

      <!-- Dashboard -->
        <li class="active"><a href='<?php print url("node/$node->nid"); ?>' ><i class="fa fa-cubes"></i> <?php print t('Dashboard'); ?></a></li>

      <!-- Settings -->
      <?php if (node_access('update', $node)): ?>
        <li><a href='<?php print url("node/$node->nid/edit"); ?>'><i class="fa fa-gear"></i> <?php print t('Settings'); ?></a></li>
      <?php endif; ?>

    </ul>

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
        <input type="text" class="form-control" size="30" value="<?php print $node->project->git_url; ?>" onclick="this.select()">

        <!-- Branch & Tag List -->
        <div class="input-group-btn">
          <button type="button" class="btn btn-default dropdown-toggle" data-toggle="dropdown" title="<?php print $branches_label; ?>">
            <small>
              <i class="fa fa-code-fork"></i> <?php print $branches_count; ?>
            </small>
            &nbsp;
            <?php if ($tags_count): ?>
              <small>
                <i class="fa fa-tag"></i> <?php print $tags_count; ?>
              </small>
            <?php endif; ?>

            <span class="caret"></span></button>
          <ul class="dropdown-menu dropdown-menu-right" role="menu">
            <?php foreach ($project->settings->git['branches'] as $branch):
            $href = isset($github_url)? "$github_url/tree/$branch" :'#';
            ?>
              <li>
                <a href='<?php print $href; ?>'><i class="fa fa-code-fork"></i> <?php print $branch; ?></a>
              </li>
            <?php endforeach; ?>
            <li class="divider"></li>
            <?php foreach ($project->settings->git['tags'] as $tag): ?>
              <li><a href='#'><i class="fa fa-tag"></i> <?php print $tag; ?></a></li>
            <?php endforeach; ?>
          </ul>
        </div><!-- /btn-group -->
      </div>
    </div>
</div>
</nav>
<div class="row placeholders">
<?php foreach ($node->project->environments as $environment_name => $environment): ?>

  <?php
  if ($environment->site_status == HOSTING_SITE_DISABLED){
    $environment_class = 'disabled';
  }
  elseif ($environment->name == $project->settings->live['live_environment']){
    $environment_class = ' active';
  }
  else {
    $environment_class = 'info';
  }
  ?>

  <div class="col-xs-6 col-sm-4 col-md-3 col-lg-3">

    <div class="list-group devshop-environment">
      <a href="<?php print $environment->url ?>" target="_blank" class="site-link list-group-item list-group-item-<?php print $environment_class ?>"  data-toggle="tooltip" data-placement="bottom" title="<?php print t('Visit !url', array('!url' => $environment->url)); ?>">

        <small class="text-muted pull-right" title="Drupal version <?php print $environment->version; ?>">
          <?php print $environment->version; ?>
        </small>

        <?php if ($environment->settings->production_mode): ?>
        <i class="fa fa-lock pull-right" title="Production Mode"></i>
        <?php endif; ?>

        <?php if ($environment->name == $project->settings->live['live_environment']): ?>
        <i class="fa fa-bolt pull-right" title="Live Environment"></i>
        <?php endif; ?>

        <?php if ($environment->site_status == HOSTING_SITE_DISABLED): ?>
          <span class="pull-right text-muted">Disabled</span>
        <?php endif; ?>

        <strong><?php print $environment->name; ?></strong>
         <br />
        <small class="text-muted"><?php print $environment->url ?></small>
      </a>
      <div class="list-group-item btn-group btn-group-justified">

        <!-- Git Select -->
        <div class="btn-group btn-git">
          <button type="button" class="btn btn-default dropdown-toggle btn-git-ref" data-toggle="dropdown"><i class="fa fa-<?php print  $environment->git_ref_type == 'branch'? 'code-fork': 'tag' ?>"></i>

            <?php print $environment->git_ref; ?>

            <span class="caret"></span>
          </button>
          <ul class="dropdown-menu btn-git-ref" role="menu">
            <li><p class="text-muted">Deploy a tag or branch:</p></li>
            <li class="divider"></li>
            <?php foreach ($node->project->settings->git['branches'] as $branch): ?>
              <li><a href="/node/<?php print $node->nid ?>/project_devshop-deploy/?git_ref=<?php print $branch ?>&environment=<?php print $environment->name ?>"><?php print $branch; ?></a></li>
            <?php endforeach; ?>
            <li class="divider"></li>
            <?php foreach ($node->project->settings->git['tags'] as $tag): ?>
              <li><a href="/node/<?php print $node->nid ?>/project_devshop-deploy/?git_ref=<?php print $tag ?>&environment=<?php print $environment->name ?>"><i class="fa fa-tag"></i> <?php print $tag; ?></a></li>
            <?php endforeach; ?>
          </ul>
        </div>

        <!-- Settings -->
        <div class="btn-group btn-settings">
          <button type="button" class="btn btn-default dropdown-toggle btn-git-ref" data-toggle="dropdown">
            <i class="fa fa-gear" ?></i>
            <span class="caret"></span>
          </button>
          <ul class="dropdown-menu">
            <li>
              <a href="<?php print url('node/' . $node->nid . '/edit/' . $environment->name, array('query'=> drupal_get_destination())); ?>">
                <i class="fa fa-gear" ?></i> Environment Settings
              </a>
            </li>
            <li class="divider"></li>
            <?php foreach ($node->environment_actions[$environment->name] as $link): ?>
              <li>
                <a href="<?php print url($link['href']); ?>"><?php print $link['title']; ?></a>
              </li>
            <?php endforeach; ?>
          </ul>
        </div>

        <!-- Sync Stuff -->
        <div class="btn-group btn-sync">
          <button type="button" class="btn btn-default dropdown-toggle" data-toggle="dropdown"><i class="fa fa-exchange"></i>
            <span class="caret"></span>
          </button>
          <ul class="dropdown-menu btn-git-ref" role="menu">
            <?php foreach ($project->environments as $env): ?>
              <?php if ($env->settings->production_mode || $env->name == $environment->name) continue; ?>
              <li><a href="/node/<?php print $node->nid ?>/project_devshop-sync/?source=<?php print $environment->name ?>&dest=<?php print $env->name ?>"><?php print t('Copy data to') . ' ' . $env->name; ?></a></li>
            <?php endforeach; ?>

            <!-- Sync from Live -->
            <?php if ($environment->name != $project->settings->live['live_environment'] ): ?>
              <li class="divider"></li>
              <li><a href="/node/<?php print $node->nid ?>/project_devshop-sync/?source=<?php print $project->settings->live['live_environment'] ?>&dest=<?php print $env->name ?>"><?php print t('Copy data from') . ' ' . $project->settings->live['live_environment'] ; ?></a></li>
            <?php endif; ?>
          </ul>

        </div>
      </div>
        <!-- Logs, Errors, commits -->
        <ul class="list-group-item nav nav-pills nav-justified">
            <li><a href="<?php print url("node/$environment->site/logs/commits"); ?>">Commits</a></li>
            <li><a href="<?php print url("node/$environment->site/logs/errors"); ?>">Errors</a></li>
            <li><a href="<?php print url("node/$environment->site/files/platform"); ?>">Files</a></li>
          </ul>
    </div>
  </div>
<?php endforeach; ?>

  <div class="col-xs-6 col-sm-4 col-md-3 col-lg-3 placeholder add-project-button">
    <a href="/node/<?php print $node->nid; ?>/project_devshop-create" class="btn btn-lg btn-success"><i class="glyphicon glyphicon-plus"></i> <?php print t('Create New Environment'); ?></a>
  </div>
</div>
