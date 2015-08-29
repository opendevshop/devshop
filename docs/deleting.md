Deleting Environments & Projects
================================

You can delete environments & projects through the devshop web UI.

How to Delete an Environment
----------------------------

Environments, by default, must be *disabled* before being deleted.  This is to help prevent accidental destruction of your environment.

To disable your environment:

1. Find your environment in the web UI.
2. Click the *Gear* icon on the environment box.
3. Click *Disable* at the bottom of the drop down.
4. On the confirmation page, click the *Disable* button.

The environment will then run it's *Disable Task*.  Once complete, it will change appearance and have the word "Disabled" displayed in the UI.

At this point, you can either *Enable* the environment or *Delete* the environment through the *Gear* icon.

If you are sure you want to delete the environment:

1. Click the *Gear* icon on your disabled environment.
2. Click *Delete*.
3. On the confirmation page, click the *Delete* button.  *You will not be able to recover once you press this button.*
4. Once the *Delete Task* runs, the environment will disappear from the Project Dashboard.

Allowing Direct Deletion
------------------------

If, for some reason you want to be able to delete sites without having to disable them first, visit the **Admin > Hosting > Settings** page.  Uncheck the box that says *Require site to be disabled before deletion*.

This is *not* recommended.  Please use caution when using this setting.  

Deleting Entire Projects
========================

To delete a project and all of it's environments:

1. Visit the *Project Dashboard* for the project you wish to delete.
2. Click *Settings*, then *Project Settings*.
3. Scroll down to the bottom of the settings page and click the *Delete this project* link.
4. On the confirmation page, click the *Delete Project* button. *This cannot be undone!*

Your project, and all of it's environments will be scheduled for deletion once you hit this button.  It will take a few dozen seconds for each environment to be deleted, so please be patient.