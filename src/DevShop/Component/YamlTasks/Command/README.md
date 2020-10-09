## Command Classes

This plugin provides classes to allow including the `git:split` command as a composer 
plugin or as a `bin` script. 

The difference between a Console Command and Composer Command classes?

### "Composer command" classes

```php
Composer\Command\BaseCommand;
```

1. Can be used in Composer plugins. 
2. Adds a `composer` command.
3. Must run with the `composer` CLI or in project's that require `composer/composer`.

### "Console command" classes
```php
DevShop\Component\GitSplit\GitSplitConsoleCommand;
Symfony\Component\Console\Command\Command;
```

1. Can be loaded into other CLI tools as a command.
2. Can be loaded into a `bin` script.
3. Once installed, can be run without Composer CLI or packages present.

