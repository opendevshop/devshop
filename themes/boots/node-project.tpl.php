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

<nav class="navbar navbar-default" role="navigation">
  <div class="container-fluid">
    <!-- Brand and toggle get grouped for better mobile display -->
    <div class="navbar-header">
      <a class="navbar-brand" href="<?php print $node_url ?>"><?php print $title ?></a>
    </div>
    <div class="navbar-form navbar-right form-group">
      <div class="input-group">
        <div class="input-group-addon"><i class="fa fa-code-fork"></i></div>
        <input type="text" class="form-control" size="40" value="<?php print $node->project->git_url; ?>" onclick="this.select()">
      </div>
    </div>
  </div>
</nav>
<div class="row placeholders">
<?php foreach ($node->project->environments as $environment_name => $environment): ?>
<!--    {% for id, environment in project.environments %}-->
  <div class="col-xs-6 col-sm-4 col-md-3 col-lg-3">

    <div class="list-group">
      <a href="<?php print $environment->url ?>" target="_blank" class="list-group-item list-group-item-info">
        <strong><?php print $environment->name; ?></strong><br />
        <small class="text-muted"><?php print $environment->url ?></small>
      </a>
      <div class="list-group-item">

        <div class="input-group">
          <div class="input-group-btn">
            <button type="button" class="btn btn-default dropdown-toggle btn-git-ref" data-toggle="dropdown"><i class="fa fa-<?php print  $environment->git_ref_type == 'branch'? 'code-fork': 'tag' ?>"></i> <span class="caret"></span></button>
            <ul class="dropdown-menu btn-git-ref" role="menu">
              <li><p class="text-muted">Deploy a tag or branch:</p></li>
              <li class="divider"></li>
              <?php foreach ($node->project->settings->git['branches'] as $branch): ?>
                <li><a href="/node/<?php print $node->nid ?>/project_devshop-deploy/ref/<?php print $branch ?>/?environment=<?php print $environment->name ?>"><?php print $branch; ?></a></li>
              <?php endforeach; ?>
              <li class="divider"></li>
              <?php foreach ($node->project->settings->git['tags'] as $tag): ?>
                <li><a href="#"><i class="fa fa-tag"></i> <?php print $tag; ?></a></li>
              <?php endforeach; ?>
            </ul>
          </div>
          <input type="text" class="form-control" size="40" value="<?php print $environment->git_ref; ?>" onclick="this.select()">
        </div>
      </div>
      <div class="list-group-item">
        <p>
          Last Deploy: <a href="logs">2 min ago</a>
        </p>
      </div>
      <ul class="list-group-item nav nav-pills nav-justified">
        <li class="dropdown">
          <a class="dropdown-toggle" data-toggle="dropdown" href="#">
            Logs <span class="caret"></span>
          </a>
          <ul class="dropdown-menu" role="menu">
            <li class="active"><a href="#">Commits</a></li>
            <li><a href="#">Errors</a></li>
            <li><a href="#">Files</a></li>
          </ul>
        </li>
        <li class="dropdown">
          <a class="dropdown-toggle" data-toggle="dropdown" href="#">
            Actions <span class="caret"></span>
          </a>
          <ul class="dropdown-menu" role="menu">
            <li><a href="#">Clone</a></li>
            <li><a href="#">Fork</a></li>
          </ul>
        </li>
      </ul>
    </div>
  </div>
<?php endforeach; ?>

  <div class="col-xs-6 col-sm-4 col-md-3 col-lg-3 placeholder add-project-button">
    <a href="/node/<?php print $node->nid; ?>/project_devshop-create" class="btn btn-lg btn-success"><i class="glyphicon glyphicon-plus"></i> <?php print t('Create New Environment'); ?></a>
  </div>
</div>

<div class="well">
  <div id="node-<?php print $node->nid; ?>" class="node<?php if ($sticky) { print ' sticky'; } ?><?php if (!$status) { print ' node-unpublished'; } ?> clear-block">

    <?php print $picture ?>

    <?php if (!$page): ?>
      <h2><a href="<?php print $node_url ?>" title="<?php print $title ?>"><?php print $title ?></a></h2>
    <?php endif; ?>

    <div class="meta">
      <?php if ($submitted): ?>
        <span class="submitted"><?php print $submitted ?></span>
      <?php endif; ?>

      <?php if ($terms): ?>
        <div class="terms terms-inline"><?php print $terms ?></div>
      <?php endif;?>
    </div>

    <div class="content">
      <?php print $content ?>
    </div>

    <?php print $links; ?>
  </div>
</div>