# -*- mode: ruby -*-
# vi: set ft=ruby :

VAGRANTFILE_API_VERSION = "2"
Vagrant.require_version ">= 1.5"
PROVISION_SCRIPT_PATH = "http://drupalcode.org/project/devshop.git/blob_plain/HEAD:/install.debian.sh"
PATH_TO_ATTRIBUTES = File.dirname(__FILE__) + "/attributes.json"

# For Development, uncomment
# PROVISION_SCRIPT_PATH = "repos/devshop/install.debian.sh"

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|

  # Base Box & Config
  config.vm.box = "hashicorp/precise64"
  config.vm.hostname = attributes["vagrant"]["hostname"]
  config.vm.network "public_network"
  config.vm.network "private_network", ip: attributes["vagrant"]["private_network_ip"]

  # Attributes are loaded from attributes.json
  if !(File.exists?(PATH_TO_ATTRIBUTES)
    raise NoSettingsException
  end
  attributes = JSON.parse(IO.read(PATH_TO_ATTRIBUTES))


  # Set SH as our provisioner
  config.vm.provision "shell", path: PROVISION_SCRIPT_PATH

  # To develop DevShop
  #   1. `vagrant up` with the synced folder commented out.
  #   2. Uncomment this line, and run `vagrant reload`.
  #   3. Change directory to `repos` and run the `init-repos.sh` script to
  #      prepare the repositories and place files in the guest.
  #   4. Apparently mysql and apache stop for some reason. Vagrant ssh in and
  #      sudo service mysql restart and sudo service apache2 restart
  # @TODO: Figure out how to make this work without this workaround.
  config.vm.synced_folder "repos/", "/repos", owner: "www-data", group: "www-data"

end

class NoSettingsException < Vagrant::Errors::VagrantError
  error_message('Project settings file not found. Create attributes.json file then try again.')
end
