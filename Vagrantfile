# -*- mode: ruby -*-
# vi: set ft=ruby :

VAGRANTFILE_API_VERSION = "2"
Vagrant.require_version ">= 1.5"
PROVISION_SCRIPT_PATH = "http://drupalcode.org/project/devshop.git/blob_plain/HEAD:/install.debian.sh"
PATH_TO_ATTRIBUTES = File.dirname(__FILE__) + "/attributes.json"

# For Development, uncomment
# PROVISION_SCRIPT_PATH = "repos/devshop/install.debian.sh"

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|

  # Base Box
  config.vm.box = "hashicorp/precise64"

  # Attributes are loaded from attributes.json

  if !(File.exists?(PATH_TO_ATTRIBUTES))
    warn "Make sure you have an attributes.json file and try again."
    exit
  end

  # Get attributes from attributes.json
  attributes = JSON.parse(IO.read(PATH_TO_ATTRIBUTES))

  # Networking & hostname
  config.vm.hostname = attributes["vagrant"]["hostname"]

  # Connect to your internet
  config.vm.network "public_network"

  # Connect to your computer at the IP in the attributes file.
  config.vm.network "private_network", ip: attributes["vagrant"]["private_network_ip"]

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
  config.vm.synced_folder "repos/", "/repos", owner: "aegir", group: "aegir"

end
