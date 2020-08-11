GitSplit Component
==================

The GitSplit component provides commands to split a monorepo using the [splitsh-lite](https://github.com/splitsh/lite) 
script and other tools.

Usage
-----

The first implementation of GitSplit is DevShop itself. Look to OpenDevShop's code for a usage example:

1. Add the package to your project's `composer.json`: 

    ```
    composer require devshop/git-split --dev  
    ```
   
   See https://github.com/opendevshop/devshop/blob/1.x/composer.json#L72

2. Import a legacy repo into a subfolder: 

    If a component repo already exists, you can merge it in with the git subtree command:
    
    ```
    git subtree add --prefix=$PATH_IN_REPO $GIT_URL $BRANCH
    ```
    
    For example, when we merged in the "devmaster" install profile from Drupal.org:
    
    ```
    git subtree add --prefix=devmaster https://git.drupalcode.org/project/devmaster.git 7.x-1.x
    ```
   
   See https://github.com/opendevshop/devshop/tree/1.x/devmaster

3. Add `extra.git-splits` 

    Add the Secondary Repository information to the config.git-split.repos section of the main devshop/composer.json file.

    ```json
    "extra": {
        "git-split": {
            "repos": {
                "devmaster": "git@github.com:opendevshop/devmaster.git"
            }
        }
    }
    ```

4. Run the command `composer git:split` every time there is new code.

    See the DevShop GitHub Action "git.yml" file for an example on running `composer git:split` to push code to multiple remotes, including different github organizations and drupal.org using SSH keys:
    
    https://github.com/opendevshop/devshop/blob/1.x/.github/workflows/git.yml#L64

Resources
---------

  * [Documentation](https://github.com/devshop-packages/git-split/blob/develop/README.md)
  * [Contributing](https://github.com/opendevshop/devshop/blob/develop/docs/DEVELOPING.md)
  * [Report issues](https://github.com/opendevshop/devshop/issues) and
    [send Pull Requests](https://github.com/opendevshop/devshop/pulls)
    in the [main DevShop repository](https://github.com/opendevshop/devshop)

Credits
-------

[splitsh-lite](https://github.com/splitsh/lite) is a third party shell script installed when this component is used.
 
 Find sources and license at https://github.com/splitsh/lite.
