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

  # Uncomment to test with other types of boxes.
  # config.vm.box = "ubuntu/trusty64"
  # config.vm.box = "chef/centos-6.5"
  # config.vm.box = "chef/centos-7.0"

  config.vm.hostname = attributes["vagrant"]["hostname"]
  config.vm.network "private_network", ip: attributes["vagrant"]["private_network_ip"]

  # Set SH as our provisioner
  config.vm.provision "shell",
    path: attributes['vagrant']['install_script'],
    args: "/vagrant/installers/ansible"

  # Prepare development environment
  if (attributes['vagrant']['development'])
      config.vm.synced_folder "devshop-6.x-1.x", "/var/aegir/devshop-6.x-1.x",
          mount_options: ["uid=12345,gid=12345"]

      config.vm.synced_folder "drush", "/var/aegir/.drush/commands",
          mount_options: ["uid=12345,gid=12345"]

      system('bash ' + File.dirname(__FILE__) + '/prepare-development.sh ' + File.dirname(__FILE__))

      config.vm.provision "shell",
        path: 'prepare-development-vagrant.sh'
  end

end

class NoSettingsException < Vagrant::Errors::VagrantError
  error_message('Project settings file not found. Create attributes.json file then try again.')
end
