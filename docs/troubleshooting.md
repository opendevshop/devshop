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
