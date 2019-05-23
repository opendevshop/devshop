# Yaml Tests

A composer plugin to make it as simple as possible to define a list of commands that get passed to GitHub Commit Status API

## Usage

1. `composer require provision-ops/yaml-tests @dev`
2. `cp vendor/provision-ops/yaml-tests/tests-example.yml tests.yml`
3. `composer yaml-tests --github-token=123456789fdasfsahfsd`


# test.yml

The `composer yaml-tests` command will look for a `tests.yml` file in your project.

Each item in your `tests.yml` file can be either a string, an object with a `command` and optionally, a `description` property, or it can be a list of commands.

See this example:

```yml
code/phpunit: phpunit --color=always
code/phpcs: phpcs --standard=PSR2 -n src
build/composer: 
  command: composer install --ansi
  description: Make sure composer doesn't fail
ACoolTest:
  - cat /etc/os-release
  - cd /var
  - ls -la
```

# Run the tests

Once you have added the plugin and created a tests.yml file, do a `dry-run`:

`composer yaml-tests --dry-run`

The output will look something like this:

![Test Run](https://github.com/provision-ops/yaml-tests/blob/master/assets/test-run.png?raw=true)

And you will get a nice summary at the end like this:

![Test Run](https://github.com/provision-ops/yaml-tests/blob/master/assets/test-result.png?raw=true)

