
# DevShop Repos
if [ ! -d 'repos' ]
  then

    mkdir repos
    cd repos

    git clone http://git.drupal.org/project/devshop.git
    git clone http://git.drupal.org/project/devshop_provision.git
    git clone http://git.drupal.org/project/devshop_hosting.git
    git clone http://git.drupal.org/project/provision_git.git
fi
