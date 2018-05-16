# Tour

## DevShop Homepage

![A screenshot of the OpenDevShop Homeage: a clear list of all of your projects and all of your environments.](https://raw.githubusercontent.com/opendevshop/documentation/master/images/devshop-projects.png)

The OpenDevShop Homepage shows you a birds eye view of all your projects. The name, install profile, git URL, Drupal Version, and a list of all the environments are visible at a glance.

Each environment indicator is updated in realtime. You can see the status of the latest task for every site in your system.

## DevShop Project Dashboard

![A screenshot of the OpenDevShop Project Dashboard](https://raw.githubusercontent.com/opendevshop/documentation/master/images/devshop.png)

The project dashboard shows you all the information you need about your website. Git URL, list of branches and tags, links to GitHub, links to the live environment, Drush aliases, and most importantly: your project's environments.

Each block is a running copy of your website. Name them whatever you want. Each one shows you the drupal version, the current git branch or tag, the URLs that are available, the last commit time, a files browser, and a backup system.

## Environments Dashboard

![A screenshot of an OpenDevShop Environment UI.](https://raw.githubusercontent.com/opendevshop/documentation/master/images/environment-settings.png)

Under the "Environment Settings" button is a list of possible tasks:

* **Download Modules** allows you to add drupal modules and themes to your project, automatically committing them to git.
* **Clone Environment** creates an exact copy of your environment with a new name.
* **Fork Environment** runs a clone, then creates a new branch with a name of your choice!
* **Disable & Destroy Environment**. A setting prevents environments from being destroyed in two clicks, use at your discretion. 
* **Flush all caches**, **Rebuild Registry**, **Run Cron**,etc.  \*These tasks are not really needed if you use Deploy hooks! Cron is always enabled, and caches can be cleared on every code deployment.
* **Backup / Restore**, as you would expect.
* **Run Tests** allows you to manually trigger test runs.

## Deploy

![A screenshot of the Deploy Code widget.](https://raw.githubusercontent.com/opendevshop/documentation/master/images/deploy-code.png)

* The **Deploy Code** control allow you to easily change what branch or tag an environment is tracking. 
* **Deploy Data** allows you to deliver new database and files to your environment. 
* **Deploy Stack** allows you to move your environment's services \(like database and files\) from one server to another.

## Tasks

![A screenshot of the Environment Task Logs.](https://raw.githubusercontent.com/opendevshop/documentation/master/images/environment-task-logs.png)

At the bottom of each environment block is a status indicator for the last task that was run on the environment.

You can click any task to view the detailed logs of any task.

## DevShop Logs

DevShop is designed for developers. We want to give them exactly the information they need. No more, no less.

Out of this came the design of our task logs. We strive to make DevShop's activities as clear and transparent as possible.

![A screenshot of Deploy Code logs.](https://raw.githubusercontent.com/opendevshop/documentation/master/images/logs-deploy-pull.png)

![A screenshot of Deploy Code logs running drush updb.](https://raw.githubusercontent.com/opendevshop/documentation/master/images/logs-deploy-pull.png)

After the code is deployed with git, any number of _Deploy Hooks_ can be run.

