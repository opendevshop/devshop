# -*- mode: ruby -*-
# vi: set ft=ruby :

# Vagrantfile API/syntax version. Don't touch unless you know what you're doing!
VAGRANTFILE_API_VERSION = "2"
Vagrant.require_version ">= 1.5.1"
PROVISION_SCRIPT_PATH = "http://drupalcode.org/project/devshop.git/blob_plain/HEAD:/install.debian.sh"

# For development...
# PROVISION_SCRIPT_PATH = "repos/devshop/install.debian.sh"

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|

  # Base Box
  config.vm.box = "hashicorp/precise64"

  # Attributes are loaded from attributes.json
  if !(File.exists?("attributes.json"))
    warn "Make sure you have an attributes.json file and try again."
    exit
  end

  # Get attributes from attributes.json
  attributes = JSON.parse(IO.read("attributes.json"))

  # Networking & hostname
  # Connect to your internet
  config.vm.network "public_network"

  # Connect to your computer at the IP in the attributes file.
  config.vm.network "private_network", ip: attributes["vagrant"]["private_network_ip"]
  config.vm.host_name = attributes["vagrant"]["hostname"]

  # Set SH as our provisioner
  # Until this is resolved, you must run the installer interactively.
  config.vm.provision "shell", path: PROVISION_SCRIPT_PATH

  # Make local source code available to the VM, if they have the repos folder.
  config.vm.synced_folder "repos", "/repos",
    owner: "www-data", group: "www-data"

  # @TODO: Clone and replace all the essential repos.
  config.vm.synced_folder "repos/devshop_hosting", "/var/aegir/devshop-6.x-1.x/profiles/devshop/modules/contrib/devshop_hosting",
    owner: "www-data", group: "www-data"

  config.vm.synced_folder "repos/devshop_provision", "/var/aegir/.drush/devshop_provision",
    owner: "www-data", group: "www-data"

end
