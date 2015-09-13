
<nav class="navbar navbar-default navbar-project" role="navigation">
  <div class="container-fluid">
    <!-- First Links -->
    <div class="nav navbar-text main-project-nav">
      <ul class="nav nav-pills">

        <!-- Dashboard -->
        <li><?php print $dashboard_link; ?></li>

        <?php if (node_access('update', $node) || user_access('edit site')): ?>
        <!-- Settings -->
        <li class="dropdown">
          <a href="#" class="dropdown-toggle <?php print $settings_active ?>" data-toggle="dropdown" role="button" aria-expanded="false">
            <i class="fa fa-sliders"></i>
            <?php print t('Settings'); ?>
            <span class="caret"></span>
          </a>
          <ul class="dropdown-menu dropdown-settings" role="menu">
            <?php if (node_access('update', $node)): ?>
            <li><?php print l(t('Project Settings'), "node/{$project->nid}/edit"); ?></li>
                <li class="divider"></li>
            <?php endif; ?>
            <?php foreach ($project->environments as $environment): $nid = empty($environment->site)? $environment->platform: $environment->site ?>
                <li><a href="<?php print url("node/{$nid}/edit");?>">
                    <?php print $environment->name; ?>
                    <?php print t('Environment Settings'); ?>
                </a></li>
            </li>
            <?php endforeach; ?>
          </ul>
        </li>
        <?php endif; ?>

        <!-- Logs-->
        <li class="dropdown">
          <a href="#" class="dropdown-toggle <?php print $logs_active ?>" data-toggle="dropdown" role="button" aria-expanded="false">
            <i class="fa fa-list-alt"></i>
            <?php print t('Logs'); ?>
            <span class="caret"></span>
          </a>
          <ul class="dropdown-menu" role="menu">
            <li><label><?php print t('Environment Logs'); ?></label></li>
            <?php foreach ($project->environments as $environment): ?>
              <li>
                <?php print l($environment->name, "node/{$project->nid}/logs/{$environment->name}"); ?>
              </li>
            <?php endforeach; ?>
          </ul>
        </li>

      </ul>
    </div>

    <!-- Git Info -->
    <div class="navbar-form navbar-right form-group">


      <div class="input-group">

        <!-- Link to github or an icon -->
        <?php if ($project->git_provider == 'github'): ?>
          <a class="input-group-addon github-button" href="<?php print $project->git_repo_url; ?>" title="<?php print t('View on GitHub'); ?>" target="_blank"><i class="fa fa-github"></i></a>
        <?php elseif ($project->git_repo_url): ?>
          <a class="input-group-addon large" href="<?php print $project->git_repo_url; ?>" title="<?php print t('View Git Repo'); ?>" target="_blank"><i class="fa fa-git"></i></a>
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
</nav>
