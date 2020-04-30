# Deleting Projects & Environments

## Deleting Environments & Projects

You can delete environments & projects through the devshop web UI.

### How to Delete an Environment

Environments, by default, must be _disabled_ before being deleted. This is to help prevent accidental destruction of your environment.

To disable your environment:

1. Find your environment in the web UI.
2. Click the _Environment Settings_ icon ![Push this button to open Environment Settings.](../.gitbook/assets/settings.png) on the environment box.
3. Click _Disable_ at the bottom of the drop down.
4. On the confirmation page, click the _Disable_ button.

The environment will then run it's _Disable Task_. Once complete, it will change appearance and have the word "Disabled" displayed in the UI.

At this point, you can either _Enable_ the environment or _Delete_ the environment through the _Gear_ icon.

If you are sure you want to delete the environment:

1. Click the _Environment Settings_ icon ![Push this button to open Environment Settings.](../.gitbook/assets/settings%20%281%29.png) on your disabled environment.
2. Click _Delete_.
3. On the confirmation page, click the _Delete_ button.  _You will not be able to recover once you press this button._
4. Once the _Delete Task_ runs, the environment will disappear from the Project Dashboard.

### Allowing Direct Deletion

If, for some reason you want to be able to delete sites without having to disable them first, visit the **Admin &gt; Hosting &gt; Settings** page. Uncheck the box that says _Require site to be disabled before deletion_.

This is _not_ recommended. Please use caution when using this setting.

## Deleting Entire Projects

To delete a project and all of it's environments:

1. Visit the _Project Dashboard_ for the project you wish to delete.
2. Click _Settings_, then _Project Settings_.
3. Scroll down to the bottom of the settings page and click the _Delete this project_ link.
4. On the confirmation page, click the _Delete Project_ button. _This cannot be undone!_

Your project, and all of it's environments will be scheduled for deletion once you hit this button. It will take a few seconds for each environment to be deleted, so please be patient.

