Hosting Tasks: Jenkins Queue
============================

This module is a proof of concept for using Jenkins as a Hosting Task Runner.

Setup
-----

1. Get jenkins running. Jenkins now has an official docker image, making running jenkins really easy:

  $ docker run --name aegir-jenkins -p 9000:8080 -v /var/jenkins_home jenkins
  
2. Enable SSH plugin.
4. Create an SSH keypair.  Put the private key in the docker container or in the jenkins home folder you have mounted. Put the public key in your server's ~/.ssh/authorized_keys file (or if you are using devshop, you must add the public key via the front-end: My Account > Edit > SSH Keys) 
4. Add SSH Host target using the path to private key, aegir user.
5. Create a Jenkins Job called "hosting-task".  
  1. Select "Run on remote SSH server", pre-build:

    drush @hostmaster hosting-task $TASK_NID
   
  2. Select "Build is parameterized". Add a parameter called "TASK_NID".
  3. Select "Allow concurrent builds".
6. Turn off hosting queue runner and supervisor so tasks don't run that way.
7. Trigger a task to be run in the hostmaster front-end.
8. Create a build in Jenkins with the task NID as a parameter and watch it run.


9. Help build this module to create new jenkins builds when a new task is added!


The config.xml for the jenkins job is available in this repo at jobs/hosting-task/config.xml
