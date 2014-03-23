# -*- mode: ruby -*-
# vi: set ft=ruby :

VAGRANTFILE_API_VERSION = "2"
Vagrant.require_version ">= 1.5.1"
# PROVISION_SCRIPT_PATH = "http://drupalcode.org/project/devshop.git/blob_plain/HEAD:/install.debian.sh"
PROVISION_SCRIPT_PATH = "repos/devshop/install.debian.sh"

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
  config.vm.host_name = attributes["vagrant"]["hostname"]

  # Connect to your internet
  config.vm.network "public_network"

  # Connect to your computer at the IP in the attributes file.
  config.vm.network "private_network", ip: attributes["vagrant"]["private_network_ip"]

  # Set SH as our provisioner
  config.vm.provision "shell", path: PROVISION_SCRIPT_PATH

end
