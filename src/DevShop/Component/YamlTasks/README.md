# Yaml Tasks

**Formerly [Yaml Tests](https://github.com/provision-ops/yaml-tests)**

Yaml Tasks is a simple composer plugin that make it as simple as possible to define and run a set of commands using YML.

It provides a composer command and an executable script that simply reads a YML file and runs each line as a process.

In addition, it integrates with GitHub's Commit Status API. If `GITHUB_TOKEN` is specified, the status of each command will be posted back to GitHub.com, to be displayed on the pull request.

The output is rendered in a way to be easier to read, and proper exit code returns if a single process fails. 

## Get Started

1. Add YamlTasks to your project.

        cd my-composer-project
        composer require devshop/yaml-tasks

2. Create a `tests.yml` file that looks something like this:

        myproject/php/lint: find src -name '*.php' -print0 | xargs -0 -n1 php -l
        myproject/php/cs: 
          description: CodeSniffer
          command: bin/phpcs --standard=PSR2 -n  --colors
        myproject/debug/environment: |
          env

3. Run the `yaml-tasks` composer command or the `yaml-tasks` bin script:

        composer yaml-tasks
        bin/yaml-tasks

      or
      
        composer y
     

### Global Install

```bash
composer global require devshop/yaml-tasks
```

To confirm the command is installed, ask for help:

```bash
composer yaml-tests --help
```

Or run the `bin/yaml-tasks` command:

```bash
bin/yaml-tasks
```

## GitHub Integration

If you pass `yaml-tasks` a GitHub Token, it will send the test results as
"commit status" indicators.

There are 3 ways to pass the GitHub Token to YamlTasks:

1. Use the `--github-token` command line option. Don't use this in CI, or you might expose your GitHub token in logs.
2. Set a GITHUB_TOKEN environment variable. This is pretty simple in Docker, but can be a challenge if your tasks get 
run in different environments.
3. **Recommended:** Create a `.env` file in your repo, or in your user's home directory:

    ```
    GITHUB_TOKEN=abcdefg
    ``` 
   
    There is a `.env.example` file in this directory you can use as an example.

## Writing tasks

### Create tasks.yml file

By default the `composer yaml-tasks` command looks for a tasks.yml file in the project root. You can also pass a path using the `--tasks-file` option.

The `tasks.yml` file is read as a simple collection of commands. The key can be any string, as long as it is unique in the test suite.

```yml
test/dir: pwd
test/environment: env
```

You can also include commands in a list:

```yml
lint:
  - find src -name '*.php' -print0 | xargs -0 -n1 php -l
  - find web/modules/custom -name '*.php' -print0 | xargs -0 -n1 php -l
  - find tasks/src -name '*.php' -print0 | xargs -0 -n1 php -l
```

You can include a description for each test like:

```yml
debug: 
  command: env
  description: Current Environment
```

### Commands in tasks.yml

Yaml Tasks work like [Composer Scripts](https://getcomposer.org/doc/articles/scripts.md#writing-custom-commands): If your project has the `config.bin-dir` set in `composer.json`, Composer will automatically add that directory to the PATH when scripts or other commands are run.

For example, you can include PHPUnit and call it without specifying the full path in composer scripts or `tasks.yml`

`composer.json`:
```json|composer.json
{
    "config": {
        "bin-dir": "bin/"
    },
    "require": {
        "devshop/yaml-tasks": "^1.5",
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

`tasks.yml`:
```yml
test/debug: 
  - which phpunit
  - phpunit --version
```

If you want to only maintain one set of scripts, you can reference composer scripts in `tasks.yml`:

`tasks.yml`:
```yml
test/debug: composer test 
```

## Running tasks

Once the `tasks.yml` file is in place, and the `composer yaml-tasks` command is available, you can trigger test runs.

### Dry Runs vs Normal

This plugin was also designed to pass these tasks as "Commit Statuses" on GitHub. This allows us to tag the results to the specific commit, pass or fail.
 
If the environment variable `GITHUB_TOKEN` or the command line option `--github-token` is NOT set, the `--dry-run` option will be forced.
 
Use the `--dry-run` option if you have a token set but do not want to post test results to GitHub.
 

Run `composer yaml-tasks` or, just like all composer commands, you can use shortcuts like `compose y`.

```bash
composer yaml-tasks
```

The output will look something like this:

![Test Run](https://github.com/devshop-packages/yaml-tasks/blob/develop/assets/test-run.png?raw=true)

And you will get a nice summary at the end like this:

![Test Run](https://github.com/devshop-packages/yaml-tasks/blob/develop/assets/test-result.png?raw=true)

## Yaml-Tasks executable

There is now a "bin" for yaml-tasks, allowing the command to be run by itself. 

If you require `devshop/yaml-tasks`, you will see a symlink to `yaml-tasks` in your `bin-dir`.