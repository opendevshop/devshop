# The Complete DevShop Training

## Training Materials for DevShop

This section of the Documentation is in more of a guide format.

We are using it for providing training on DevShop.

We are actively populating this section. Please be patient as content becomes available!

This material will be rolled into the general documentation as we fill out the details.

## Outline

### Unit 1: Getting DevShop

1. **Preparing for DevShop**: Domain names and DNS. 1. Buy a domain name or choose a subdomain on an existing domain. 2. DNS Setup.
2.  **Get DevShop**: how to install devshop on your own server. 1. _install.sh_: [Standalone install script](https://github.com/opendevshop/devshop/blob/1.x/install.sh). 2. _Docker_: Getting devshop running on Docker with our [Docker Container](https://hub.docker.com/r/devshop/devmaster/). 3. _Vagrant_: Launch a vagrant image of devshop with our [built in Vagrantfile](https://github.com/opendevshop/devshop/blob/1.x/Vagrantfile). 4. _Ansible_: Configure a devshop server with our [Devmaster Ansible Role](https://galaxy.ansible.com/opendevshop/devmaster/).

### Unit 2: Using DevShop

1. **Create a codebase**: How to add Drupal to a git repository. 1. Rules for a good codebase. 2. Create a new git repository. 3. Download drupal into it. 4. Push the new code up to your git repository.
2.  **Create Projects**: How to start new Drupal projects. 1. Adding new projects. 2. Setting up Git Access. 3. Automated Deployment Settings. 4. Environments Creation. 5. Installation Profile selection.
3. **Create Environments**: How to launch new sites for your project. 1. Create New Environment: Run install profile. 2. Clone Environment. 3. Fork Environment.
4. **Project Dashboard** 1. Dashboard: List all environments. 2. Project Settings. 3. Logs: Environment Task Logs. 3. Git Repository. 4. Branches & Tags. 2. Webhook Settings. 2. Drush Aliases.
5.  **Project Settings** 1. Deployment Hooks: Clear Caches, Revert Features, etc. 2. Deployment Automation: Immediate, Queued, Manual. 3. Domain Name Settings: Live Domain, automated subdomains. 4. Default Environment Settings: Default servers, default install profile. 5. Testing: Simpletest or Behat. 6. GitHub or BitBucket integration:
   * Create Environments for Pull Requests
   * Delete Pull Request Environments
   * Pull Request Environment Creation Method
6. **Environment Settings**
   1. Lock Database.
   2. Disable Deploy on Commit.
   3. Deployment Hooks.
   4. Domain Names.
   5. Backup Schedule.
   6. HTTP Basic Authentication: Password protect your site.
   7. Error logs: Making logs available.
   8. SSL.
7. **Environment Dashboard**
   1. Environment name, git version, Drupal version, environment status indicators.
   2. Domains list.
   3. Log in button.
   4. Task Logs.
   5. Error logs.
   6. Backups.
   7. Deploy Controls:
      * Deploy Code: Change the branch or tag, and pull code.
      * Deploy Data: Copy databases and files from other environments.
      * Deploy: Stack. Choose the servers for this site.
   8. Git Status Display.
   9. Last Task Display.
   10. Task Logs.
   11. Environment Tasks:
       1. Run Tests.
       2. Commit Code.
       3. Download Modules.
       4. Clone/Fork/Disable/Destroy Environment.
       5. Export & Import Config.
       6. Verify.
       7. Flush all caches.
       8. Rebuild Registry.
       9. Run Cron.
       10. Run DB Updates.
       11. Backup.
8. **Importing Sites**
   1. Site Migration primer: Database, Files, Code.
   2. Using DevShop Drush Aliases.
   3. Adding "Remote Aliases" for Deploying Data.
   4. Using the command line.
9. **Connecting to Devshop**
   1. My Account &gt; SSH Keys.
   2. Always SSH as aegir@server\_master.
   3. Drush Aliases are available on the project dashboard.
10. **Going Live.**
    1. Selecting your Primary Environment.
    2. Locking your Database.
    3. Configuring Environment Domain Names & Redirection.
    4. DNS. 
11. **Deployment & Environment Management Strategies**
    1. Dev + Test + Live. 
    2. Live + Pull Request Environments.
    3. Stage + Tagged Releases: Git Tag &gt; Manual Deploy on Live.
    4. Continuous Deployment: Merge to `master` &gt; Automatic Deploy to Live.
    5. Release Environments + Pull Request Environments. Create an environment with a tagged release for testing, then deploy to live.
12. **Remote Servers**
    1. Web Servers: Apache or NGINX. Requires SSH access and sudo access to reload web server.
    2. Database Servers: Any MySQL-compatible server. Requires a database root user access.

## Unit 3: Test Driven Development

1. **What is CI**: Breakdown of CI terms and topics.
2. **Branch Driven Development**: Using git & branching to improve code quality and developer efficiency.
3. **Branch Environments**: Creating copies of your site on different branches to isolate bug fixes and new features  until it’s ready to merge.
4. **Configuring Automated Testing**: Check the box to run Behat tests. 
5. **GitHub API Integration**: Pull Request and Commit Status integration.
6. **Custom Tests with hooks.yml**: Create a file in your Drupal site to run any command you wish.
7. **Advanced Topics**: Selenium, Docker, Screenshots, Visual Regression, & more.

## Unit 4: Cloud Management

1. **DevShop Cloud**: How to use DevShop to create new servers using just your API key.
2. **DevShop Ansible**: How to use DevShop to configure and manage your servers automatically using Ansible. 
3. **Cloud Examples**: Using Aegir Cluster & DevShop Ansible to launch multiple web servers and a load balancer.

## Unit 5: Deeper Dive

1. **Web Interface**: Devmaster: Drupal site with Aegir modules.
   1. Servers & Services.
   2. Platforms.
   3. Sites.
   4. Tasks.
   5. Queues: Cron, Tasks, Backups, Deploys.
   6. Hosting Settings & DevShop Settings
2. **Command Line Interface**
   1. Aegir User: No sudo except for webserver reload.
   2. Drush Aliases & "Contexts": 
      * Use @hostmaster for web interface.
   3. Provision Commands.
   4. Separation between Web & Command Line interface.
3. **Hosting Task Management** 1. `drush @hostmaster hosting-queued` 2. Supervisor. 3. `drush @hostmaster hosting-tasks` / `drush @hostmaster hosting-task 123` 4. Using Jenkins for Task Running with [github.com/opendevshop/hosting\_task\_jenkins](https://github.com/opendevshop/hosting_task_jenkins)
4. **Customizations** 1. Customizing settings.php

