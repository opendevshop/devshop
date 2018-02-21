<?php if ($project->name): ?>
<div id="project-add-status" class="panel panel-default">
    <div class="panel-heading">
        <p class="pull-right">
            <a href="<?php print url('projects/add/settings'); ?>"><i class="fa fa-sliders"></i> <?php print t('Edit') ?></a>
        </p>
        <?php print t('Project Information'); ?>
    </div>

    <ul class="list-group">
        <li class="list-group-item">
            <label><?php print t('Name'); ?></label><?php print $project->name; ?>
        </li>
        <li class="list-group-item" data-toggle="tooltip" data-placement="right" title="<?php print t('The git repository to use to build the environments.');?>">
            <label><?php print t('Git URL'); ?></label>
            <span class="git-url">
                <?php print $project->git_url; ?>
            </span>
        </li>
        <li class="list-group-item"  data-toggle="tooltip" data-placement="right" title="<?php print t('The source code for each environment will be placed into subfolders of this folder.');?>">
            <label><?php print t('Base Path'); ?></label>
            <p><?php print $project->code_path; ?></p>
        </li>

        <?php if ($project->drupal_path): ?>
        <li class="list-group-item" data-toggle="tooltip" data-placement="right" title="<?php print t('The relative path to the Drupal code within your repository.');?>">
            <label><?php print t('Path to Drupal'); ?></label>
            <p><?php print $project->drupal_path; ?></p>
        </li>
        <?php endif; ?>

        <?php if (isset($project->settings->live) && !empty($project->settings->live['live_domain'])): ?>
        <li class="list-group-item" data-toggle="tooltip" data-placement="right" title="<?php print t('The primary URL of the live website for this project.');?>">
            <label><?php print t('Live Domain'); ?></label>
            <p><?php print $project->settings->live['live_domain']; ?></p>
        </li>
        <?php endif; ?>

    <?php if (isset($project->settings->default_environment) && isset($project->settings->default_environment['web_server'])): ?>
        <li class="list-group-item" data-toggle="tooltip" data-placement="right" title="<?php print t('The collection of servers powering this website.');?>">
            <label>
                <?php print t("Default Stack"); ?>
            </label>
            <a href="<?php print url('node/' . $web_server_node->nid); ?>" title="<?php print t('Database Server')?>" class="btn btn-link" target="_blank">
                <i class="fa fa-cube"></i>
                <?php print $web_server_node->title ?>
            </a>
            <a href="<?php print url('node/' . $db_server_node->nid); ?>" title="<?php print t('Web Server')?>" class="btn btn-link" target="_blank">
                <i class="fa fa-cube"></i>
                <?php print $db_server_node->title ?>
            </a>
        </li>
    <?php endif; ?>
    </ul>
</div>
<?php endif; ?>
