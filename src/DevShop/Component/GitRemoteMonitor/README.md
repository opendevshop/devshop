Git Remote Monitor
=====================

The `devshop/git-remote-monitor` component serves 3 purposes:

1. Maintain an up to date list of git references from a list of git repositories.
2. Run additional commands when any changes are detected, such as `git pull` or `docker build`.
3. Run as a background service so that data is always up to date and commands are always run as fast as possible, using [wrep/daemonizable-command](https://github.com/mac-cain13/daemonizable-command).

It does this by:

1. Reading a list of git remote URLs from configuration.
2. Running `git ls-remote $REMOTE_URL` to retrieve the current list of references.
3. Saves the list of references to configuration, yml files, etc.
4. Triggers an event that other tools can respond to.
5. Doing all of this continuously, as a daemon.

This tool will *not*:

1. Run any deployment commands or modify any local or remote git repos. It will only monitor and kick off to other systems.
2. Depend or lock into any other hosting platform or system package beyond PHP.


Background
----------

From the author:

This tool is the result of years of struggle to do one thing: Update code and deploy things as fast as possible after code is "pushed".

Existing solutions in Aegir and DevShop all had limitations. Webhooks were often unavailable due to firewalls, queues only happen "once per minute", git host specific API-based solutions created more maintenance.

I have posted numerous github issues, jira tickets, and now clubhouse stories over the years talking about a folder-specific git watching component.

As recently as a 4 days ago I was creating "platform-specific" git_remote_references storage in the hosting_platfomr module itself. This was storing data for each *path*.

Once 2021 came around, I had a revelation. DevShop (and users of aegir git) can have a million clones of a single repo. Checking the remote status of every one is totally unnecessary!

DevShop has used the `git ls-remote` command to load the available branches and tags for a repo since the beginning, but it is only doing that when a "project verify" task is run. 

All we need to do is read and save the "references list" for each git remote url, read the SHA of each branch, and then, only if there are changes, run a second command to trigger any action desired by the implementor. 

In Hosting.module's case, this can be `drush @hostmaster hosting-task deploy $ALIAS`. 

In addition, this tool is a great way to retrieve the list of available branches, which is a requirement in a DevOps system that provides users the ability to create environments from a list of branches.

Resources
---------

  * [Documentation](https://github.com/opendevshop/devshop/blob/src/DevShop/Component/GitRemoteMontirREADME.md)
  * [Report issues](https://github.com/opendevshop/devshop/issues) and
    [send Pull Requests](https://github.com/opendevshop/devshop/pulls)
    in the [main DevShop repository](https://github.com/opendevshop/devshop)


