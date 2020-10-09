---
description: >-
  This documentation page outlines the methods for migrating an existing site
  into DevShop.
---

# Migrating sites into DevShop

DevShop is based on a standard LAMP stack architecture, with Drush.

Drush can be used to export and import databases and copy files for your sites. 

## What you need

### Code

First and foremost, your Drupal code must be in a Git repository. Do this first. GitHub has great documentation See [https://help.github.com/en/articles/create-a-repo](https://help.github.com/en/articles/create-a-repo) for help if needed.

If your site has never been in git, we recommend putting everything on your server in git, except for:

* The sites/\*/settings.php files, if they contain mysql access information.
* Any tar.gz files that may have been left behind.
* Any Drupal site files: those are considered content, and while important, are usually not stored in git.

### Database 

MySQL databases are backed up and moved around by exporting them to SQL files. 

You can do this on the command line with `mysqldump` or a web based tool like PHPMyAdmin.

See [https://www.drupal.org/docs/7/backing-up-and-migrating-a-site](https://www.drupal.org/docs/7/backing-up-and-migrating-a-site) for more information on how Database migrations are handled.

### Files

Almost all Drupal sites have important files: If you have uploaded a logo, use account avatars, have an imagefield, your files need to be copied over.

## How to Migrate

### Step 1: Copy files to your devshop server

DevShop handles deploying the source code to your servers, but the files and database need to be copied manually.

You will have to decide how to get your database and files onto the server, but it is more secure to _push_ files into the server from source server. This way you are granting your source server access to SSH into DevShop, but not letting DevShop into your other servers.

You might not want DevShop to have access to your remote server, because DevShop makes it easy for developers to get access by uploading their SSH keys.

### Step 2: Create a DevShop Project

Create a new project, and DevShop will clone the source code to the server every time you make a new environment.

Create a project and an environment to get started. If you are using DevShop for Production use, go ahead and create your "production" environment first, so all clones will copy that one.

### Step 3: Import your Database

#### Drush Aliases

DevShop generates Drush aliases for every environment it creates. You can use these aliases to target a specific site when importing or exporting data.

SSH into the DevShop Server as `aegir`, then type `drush site-aliases` or `drush sa` to get a list of all your sites.

If you had one project called "myproject", and one environment called "dev", it would look like this when you called `drush sa`

```text
aegir@devshop:~$ drush sa
@myproject
@myproject.dev
@myproject.dev.mydevshop.com

```

Once you know the site you want to import, you can use the alias and drush commands to import the database and files.

#### Drush sql-connect

Use the `sql-connect` or `sqlc` command to import your database file.

You can use the "&lt;" character to import an SQL file into the site:

```text
drush @myproject.dev sqlc < database.sql 
```

### Step 4: Import your files

#### Drush rsync

You can use Drush's "rsync" command to easily copy files into the right location. It is just a wrapper for rsync that can translate Drush Alias information for you. 

The `drush rsync` command behaves just like the rsync command: `drush rsync SOURCE DESTINATION`.

Source and Destination can be either local or remote. If it is remote, you must have SSH access.

For Example, if your Drupal files is on another server called `server` in the folder `/var/www/html/files` , you can run the following command to sync the files from that server, as long as the user named `user` has SSH access: 

```text
aegir@devshop:~$ drush rsync user@server:/var/www/files @myproject.dev:%files
```

  
The `%files` at the end of that command is a "drush path alias", it always points to the active drupal files directory, so it's best to use that. as the path is different for each environment.![](https://ssl.gstatic.com/ui/v1/icons/mail/images/cleardot.gif)  


## Alternatives

### Create Environment: SQL Import

The "Create Environment" form has an option to import the database from an SQL file or an SQL host.

### DevShop Remote Aliases

DevShop has a feature called "Remote Aliases" that allows you to add "Sync Data" or "Clone Environment" sources via the Web UI.

At the top of every Project dashboard is a "Remote Aliases" button. This will allow you to create a Drush alias that represents your external site.

_This can only be used if you trust your DevShop users with access to the remote servers, as it requires SSH access from the DevShop server to the remote to work._

1. Visit your project's dashboard. Click "Remote Aliases".
2. Click "+ Add Remote Alias".
3. Fill in the information for your site: alias name, Site URI, Path to root, remote host and remote user.
4. When you click "Save Alias", it will be added to your Drush aliases for the project. 

For example, if you added an alias called "remotelive" to your project called "myproject", as long as SSH access is setup from the devshop aegir user to the remote, you will be able to use drush for that site:

```text
aegir@devshop:~$ drush rsync @myproject.remotelive:%files @myproject.dev:%files
```

Once your remote alias is setup, you can also use the "Sync Data" drop down to pull data from the remote site to your chosen environment.

And finally, the remote alias appears as an option under "Clone Site" on the "Create Environment" button.

