Ansible Role: Aegir Apache
=========

Extends the geerlingguy.apache role to allow usage as an aegir remote server.

Tasks
=====

This role does very little: 

- Symlink from /etc/apache/conf.d to Aegir's Apache.conf
- Add to sudoers to allow aegir to reload apache.
