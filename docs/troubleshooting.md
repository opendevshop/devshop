Troubleshooting
=================

Here are some tips to overcome issues you may run into while using
DevShop.


queued tasks are not completing
-------------------------------

If tasks are being queued up, but not running you may have to restart
your queue runner. 

Check your tasks list to see if anything is actually running:

1. Click the Gear icon in the header.
2. Click Task Logs link.
3. If you see Queued tasks (gray background) and none of them are running (you would see a spinner icon.) then your supervisor queue may have stopped.  

To restart supervisor run the following command on
your web server as a user that can sudo:

```
sudo service supervisor stop
sudo service supervisor start
```

Your queued tasks should start running again.


## "Existing sites were found on this platform. These sites will need to be deleted before this platform can be deleted."

This means that a settings.php file was found in a platform's codebase at sites/DOMAIN/settings.php. 

This can happen if a site fails on clone, or cannot import the data for some reason.
  
  To fix:
  
  1. SSH into the server as aegir.
  2. cd into the folder of the environment (platform) you are trying to delete.
  3. cd into the "sites" folder.  Run "ls" to see what sites might still be there.
  4. These sites might still exist (have a database created, etc.) so you should first try to run:
  
  ```
  $ drush @DOMAIN provision-delete
  ```
  
  If this doesn't work, then force-remove the folder:
  
  ```
  $ rm -rf sites/DOMAIN
  ```
  
  5. Retry the platform deletion task.