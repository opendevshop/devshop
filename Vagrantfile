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
  # Uncomment to test with other types of boxes.
  # config.vm.box = "hashicorp/precise64"
  # config.vm.box = "ubuntu/trusty64"
  # config.vm.box = "chef/centos-6.5"
   config.vm.box = "chef/centos-7.0"

  config.vm.hostname = attributes["vagrant"]["hostname"]
  config.vm.network "private_network", ip: attributes["vagrant"]["private_network_ip"]

  # Set SH as our provisioner
  config.vm.provision "shell",
    path: attributes['vagrant']['install_script'],
    args: "/vagrant/installers/ansible"

end

class NoSettingsException < Vagrant::Errors::VagrantError
  error_message('Project settings file not found. Create attributes.json file then try again.')
end
