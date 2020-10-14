# Developing DevShop

## The Monorepo
## Secondary Repos

## Components

DevShop is a DevOps framework. It contains many components that work independently.

These components are pushed out to other git repos as Composer Packages.

### Adding new Components

To add a new component follow these steps:

1. Pick a name for the component. Try to use two words, CamelCase. A component 
 can include many classes, so group them logically.
2. Create a new directory in [`./src/DevShop/Component`](../src/DevShop/Component) using the component name.
3. Copy the [`./src/DevShop/Templates/ComponentTemplate`](../src/DevShop/Templates/ComponentTemplate) folderinto . `src/DevShop/Component` and rename it.
4. Add a the component code. This folder will become a standalone Composer package.
5. Follow the direction of the [symfony/symfony](https://github.com/symfony/symfony) repository. See the [Process Component](https://github.com/symfony/symfony/tree/5.0/src/Symfony/Component/Process) for an example of best practices.
6. Create a git repository on GitHub: https://github.com/new
  1. Select `devshop-packages` as the owner.
  2. Add the same purpose statement from the `README.md` to the **Description** field.
  2. Format the repository name by converting CamelCase ComponentName to `component-name`.
  3. Set it to Public.
  4. **DO NOT SELECT** *Initialize this repository with a README*, *Add .gitigire* or *Add a license*. The GitSplit tool will populate this repo.
  5. Press "Create Repository" to create the new git repo.
6. Add the package name to the `replace` section of [`devshop/composer.json`](../composer.json). If the component is required by the `devshop` CLI, add it to `require` instead as `"devshop/component-name": "*"`.

7. Add the Secondary Repository information to the `config.git-split.repos` section of the main [`devshop/composer.json`](../composer.json) file.
 
    Use the path within the `opendevshop/devshop` repo to the component as the key of the array element, and the git repo URL as the value. 
 
    It should look something like this:

    ```json
    "extra": {
        "git-split": {
            "repos": {
                "src/DevShop/Component/GitSplit": "git@github.com:devshop-packages/git-split.git",
                "devmaster": "git@github.com:opendevshop/devmaster.git",
                "roles/opendevshop.apache": "git@github.com:opendevshop/ansible-role-apache.git",
                "roles/opendevshop.devmaster": "git@github.com:opendevshop/ansible-role-devmaster.git",
                "roles/opendevshop.users": "git@github.com:opendevshop/ansible-role-user.git"
            }
        }
    },
    ```    

9. Create a new branch from `develop` and push it:

        $ git checkout -b component/php/component-name
        $ git push -u origin component/php/component-name
    
    Use the branch naming convention `component/php/git-split`.
    
    *NOTE:* GitHub now provides a direct link to the "Create Pull Request" page after you push a new branch. Look for this in the output of the first `git push` command on a new branch.

## GitHub Actions: Automated Git Split
    
As soon as you push this new branch, (even before you submit a pull request), 
GitHub Actions will run our workflows on your branch. Check the [GitHub Actions 
page on the DevShop Repo](https://github.com/opendevshop/devshop/actions) for the *Git Management* workflow results.

9. Once you feel your component is read for review by the DevShop Team, submit a Pull Request!

### Merging in external repos

If a component repo exists already, you can merge it in with the `git subtree` command. 

Create a new branch from `develop` first, then pull in the tree: 

        git checkout -b component/TYPE/NAME develop
        git subtree add --prefix=$PATH_IN_REPO https://$GIT_URL $BRANCH

Then submit a PR against `develop` branch.

See https://www.jvt.me/posts/2018/06/01/git-subtree-monorepo/ for a good explanation.

### List of Components

This list is changing rapidly. We'll try to keep it up to date with status.

#### Component: GitSplit

1. GitSplit - Composer command and bin script to run splitsh-lite. This is used to split the monorepo.

## Follow Symfony

Symfony solved how to build a big repo with many components.

Refer to Symfony/symfony codebase for examples.

Useful examples include:

- Commit to add a new Component: https://github.com/symfony/symfony/commit/053de25edffaf39a6d7e16d0badbedf79f89a8e3#diff-b5d0ee8c97c7abd7e3fa29b9a27d1780
- 