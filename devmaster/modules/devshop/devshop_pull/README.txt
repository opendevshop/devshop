DevShop Pull
============

Provides a way for environments to stay up to date with the git repository.

Each project can configure to Pull on Queue or Pull on URL Callback.

Pull on Queue will trigger Pull Code tasks on a regular basis using Hosting
Queues.  Pull on URL Callback provides a URL that you can add to your git host
to ping on receiving a commit.


GitHub Setup
------------

1. Visit your repos page: http://github.com/YOURNAME/YOURREPO
2. Click "Settings".
3. Click "Service Hooks".
4. Click "WebHook URLs"
5. Copy and paste your project's Git Pull Trigger URL into the URL field of the
   WebHook URLs page.
6. Click "Test Hook" to run a test, then check your DevShop project to ensure a
   Pull Code task was triggered.
