# Learning DevShop

## Training Materials for DevShop

This section of the Documentation is in more of a guide format.

We are using it for providing training on DevShop.

We are actively populating this section. Please be patient as content becomes available!

## Outline

### Unit 1

1. **Preparing for DevShop**: Domain names and DNS.
  1. Buy a domain name or choose a subdomain on an existing domain.
  2. DNS Setup.

2. ** Get DevShop**: how to install devshop on your own server.
  1. *install.sh*: [Standalone install script](https://github.com/opendevshop/devshop/blob/1.x/install.sh).
  2. *Docker*: Getting devshop running on Docker with our [Docker Container](https://hub.docker.com/r/devshop/devmaster/).
  3. *Vagrant*: Launch a vagrant image of devshop with our [built in Vagrantfile](https://github.com/opendevshop/devshop/blob/1.x/Vagrantfile).
  4. *Ansible*: Configure a devshop server with our [Devmaster Ansible Role](https://galaxy.ansible.com/opendevshop/devmaster/).

2. **Create a codebase**: How to add Drupal to a git repository.
  1. Rules for a good codebase.
  2. Create a new git repository.
  3. Download drupal into it.
  4. Push the new code up to your git repository.

3. ** Create Projects**: How to start new Drupal projects.
  1. Adding new projects.
  2. Setting up Git Access.
  3. Automated Deployment Settings.
  4. Environments Creation.
  5. Installation Profile selection.

5. **Create Environments**: How to launch new sites for your project.
  1. Create New Environment: Run install profile.
  2. Clone Environment.
  3. Fork Environment.

6. **Project Dashboard**
  1. Dashboard: List all environments.
  2. Project Settings.
  3. Logs: Environment Task Logs.
  3. Git Repository.
  4. Branches & Tags.
  2. Webhook Settings.
  2. Drush Aliases. 

7. ** Project Settings**
  1. Deployment Hooks: Clear Caches, Revert Features, etc.
  2. Deployment Automation: Immediate, Queued, Manual.
  3. Domain Name Settings: Live Domain, automated subdomains.
  4. Default Environment Settings: Default servers, default install profile.
  5. Testing: Simpletest or Behat.
  6. GitHub or BitBucket integration: 
    - Create Environments for Pull Requests
    - Delete Pull Request Environments
    - Pull Request Environment Deploy Method
8. **Environment Settings**
  1. Lock Database.
  2. Disable Deploy on Commit.
  3. Deployment Hooks.
  4. Domain Names.
  5. Backup Schedule.
  6. HTTP Basic Authentication: Password protect your site.
  6. Error logs: Making logs available.
  7. SSL.
8. **Environment Dashboard**
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
9. **Importing Sites**
  1. Site Migration primer: Database, Files, Code.
  2. Using DevShop Drush Aliases.
  3. Adding "Remote Aliases" for Deploying Data.
  4. Using the command line.
10. **Going Live.**
  1. Selecting your Primary Environment.
  2. Locking your Database.
  2. Configuring Environment Domain Names & Redirection.
  3. DNS.

   


