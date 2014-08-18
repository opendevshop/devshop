# -*- mode: ruby -*-
# vi: set ft=ruby :

VAGRANTFILE_API_VERSION = "2"
Vagrant.require_version ">= 1.5"
PATH_TO_ATTRIBUTES = File.dirname(__FILE__) + "/attributes.json"

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|

  # Load Attributes
  if !(File.exists?(PATH_TO_ATTRIBUTES))
    raise NoSettingsException
  end
  attributes = JSON.parse(IO.read(PATH_TO_ATTRIBUTES))

  # Base Box & Config
  config.vm.box = "hashicorp/precise64"
  config.vm.hostname = attributes["vagrant"]["hostname"]
  config.vm.network "public_network"
  config.vm.network "private_network", ip: attributes["vagrant"]["private_network_ip"]

  # Clone project repos if the folder doesn't exist yet.
  if !(File.directory?("#{File.dirname(__FILE__)}/repos/devshop"))

    $clone_script = <<SCRIPT
      git clone git@git.drupal.org:project/devshop.git #{File.dirname(__FILE__)}/repos/devshop
      git clone git@git.drupal.org:project/devshop_provision.git #{File.dirname(__FILE__)}/repos/devshop_provision
      git clone git@git.drupal.org:project/devshop_hosting.git #{File.dirname(__FILE__)}/repos/devshop_hosting
      git clone git@git.drupal.org:project/provision_git.git #{File.dirname(__FILE__)}/repos/provision_git
SCRIPT

    system($clone_script)
  end

  # Set SH as our provisioner
  config.vm.provision "shell", path: attributes['vagrant']['install_script']

  # To develop DevShop
  #   1. `vagrant up` with the synced folder commented out.
  #   2. Uncomment this line, and run `vagrant reload`.
  #   3. Upon reload, mysql and apache may stop for some reason. Vagrant ssh in and
  #      sudo service mysql restart and sudo service apache2 restart
  # @TODO: Figure out how to make this work without this workaround.
  config.vm.synced_folder "repos/", "/repos", owner: "www-data", group: "www-data"

  # Remove code and replace with links to our sources.
  $script = <<SCRIPT
  sudo su - aegir -c "
  rm -rf /var/aegir/.drush/devshop_provision
  ln -s /repos/devshop_provision /var/aegir/.drush/devshop_provision

  rm -rf /var/aegir/devshop-6.x-1.x/profiles/devshop/modules/devshop/devshop_hosting
  ln -s /repos/devshop_hosting /var/aegir/devshop-6.x-1.x/profiles/devshop/modules/devshop/devshop_hosting

  rm -rf /var/aegir/.drush/provision_git
  ln -s /repos/provision_git /var/aegir/.drush/provision_git
  "
SCRIPT
  config.vm.provision "shell",
    inline: $script

end

class NoSettingsException < Vagrant::Errors::VagrantError
  error_message('Project settings file not found. Create attributes.json file then try again.')
end
