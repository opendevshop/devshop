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
  1. Select `opendevshop` as the owner

### List of Components

This list is changing rapidly. We'll try to keep it up to date with status.

#### Component: GitTools

1. GitSplit - Composer command and bin script to run splitsh-lite. This is used to split the monorepo.
