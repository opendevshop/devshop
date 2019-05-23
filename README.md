# Provision Ops: Yaml GitHub Tests

A composer plugin to make it as simple as possible to define a list of commands that get passed to GitHub Commit Status API

## Usage

1. `composer require provision-ops/yaml-tests @dev`
2. `cp vendor/provision-ops/yaml-tests/tests-example.yml tests.yml`
3. `composer yaml-tests`


# test.yml

```yml
test-group/composer: 
  command: composer install --ansi
test-group/unit: phpunit --color=always
test-group/cs: phpcs --standard=PSR2 -n src
```
