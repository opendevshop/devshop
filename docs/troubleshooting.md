Troubleshooting
=================

Here are some tips to overcome issues you may run into while using
DevShop.


queued tasks are not completing
-------------------------------

If tasks are being queued up, but not running you may have to restart
your queue runner. To do so run the following from the command line on
your web server:

```
sudo service supervisor stop
sudo service supervisor start
```

Your queued tasks should start running again.
