# Yaml Tests

Yaml Tests is a simple composer plugin tthat make it as simple as possible to define and run a set of tests.

The plugin provides a composer command that simply reads a YML file and runs the lines as a process.

The output is rendered in a way to be easier to read, and proper exit code returns if a single process fails. 

It can be installed "locally" (included in your composer.json file) or "globally" (installed into the global "composer" command.

## Installation

Keeping `yaml-tests` in your `composer.json` (local install) is the most stable way to operate, since the version is pinned.

### Local Install

1. `cd my-composer-project`
2. `composer require provision-ops/yaml-tests`

### Global Install

`composer global require provision-ops/yaml-tests`

To confirm the command is installed, ask for help:

```bash
composer yaml-tests --help
```

## Writing Tests

### Create tests.yml file

By default the `composer yaml-tests` command looks for a tests.yml file in the project root. You can also pass a path using the `--tests-file` option.

The `tests.yml` file is read as a simple collection of commands. The key can be any string, as long as it is unique in the test suite.

```yml
test/dir: pwd
test/environment: env
```

You can also include commands in a list:

```yml
lint:
  - find src -name '*.php' -print0 | xargs -0 -n1 php -l
  - find web/modules/custom -name '*.php' -print0 | xargs -0 -n1 php -l
  - find tests/src -name '*.php' -print0 | xargs -0 -n1 php -l
```

You can include a description for each test like:

```yml
debug: 
  command: env
  description: Current Environment
```

#### Commands in tests.yml

Yaml Tests work like [Composer Scripts](https://getcomposer.org/doc/articles/scripts.md#writing-custom-commands): If your project has the `config.bin-dir` set in `composer.json`, Composer will automatically add that directory to the PATH when scripts or other commands are run.

For example, you can include PHPUnit and call it without specifying the full path in composer scripts or `tests.yml`

`composer.json`:
```json|composer.json
{
    "config": {
        "bin-dir": "bin/"
    },
    "require": {
        "provision-ops/yaml-tests": "^1.1",
        "phpunit/phpunit": "^8.1"
    },
    "scripts": {
        "test": [
            "which phpunit",
            "phpunit --version"
        ]
    }
}
```

Having the `scripts.test` section in `composer.json` creates a composer command called `composer test`.

`tests.yml`:
```yml
test/debug: 
  - which phpunit
  - phpunit --version
```

If you want to only maintain one set of scripts, you can reference composer scripts in `tests.yml`:

`tests.yml`:
```yml
test/debug: composer test 
```

## Running tests

Once the `tests.yml` file is in place, and the `composer yaml-tests` command is available, you can trigger test runs.

### Dry Runs vs Normal

This plugin was also designed to pass these tests as "Commit Statuses" on GitHub. This allows us to tag the results to the specific commit, pass or fail.
 
If the environment variable `GITHUB_TOKEN` or the command line option `--github-token` is NOT set, the `--dry-run` option will be forced.
 
Use the `--dry-run` option if you have a token set but do not want to post test results to GitHub.
 

Run `composer yaml-tests` or, just like all composer commands, you can use shortcuts like `compose y`.

```bash
composer yaml-tests
```

The output will look something like this:

![Test Run](https://github.com/provision-ops/yaml-tests/blob/master/assets/test-run.png?raw=true)

And you will get a nice summary at the end like this:

![Test Run](https://github.com/provision-ops/yaml-tests/blob/master/assets/test-result.png?raw=true)
