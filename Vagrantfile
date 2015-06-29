# -*- mode: ruby -*-
# vi: set ft=ruby :

VAGRANTFILE_API_VERSION = "2"
Vagrant.require_version ">= 1.5"
PATH_TO_ATTRIBUTES = File.dirname(__FILE__) + "/attributes.json"

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|

  # Load Variables
  require 'yaml'
  settings = YAML.load_file(File.dirname(__FILE__) + "/vars.yml")

  # Base Box & Config
  config.vm.box = "ubuntu/trusty64"

  # Uncomment to test with other types of boxes.
  # config.vm.box = "ubuntu/trusty64"
  # config.vm.box = "chef/centos-6.5"
  # config.vm.box = "chef/centos-7.0"

  config.vm.hostname = settings["server_hostname"]
  config.vm.network "private_network", ip: settings["vagrant_private_network_ip"]

  config.vm.provider "virtualbox" do |v|
    v.memory = 2048
  end

  # Set SH as our provisioner
  config.vm.provision "shell",
    path: settings["vagrant_install_script"],
    args: settings["vagrant_install_script_args"]

  # Prepare development environment
  if (settings["vagrant_development"])

      system('bash ' + File.dirname(__FILE__) + '/vagrant-prepare-host.sh ' + File.dirname(__FILE__) + ' ' + settings["devmaster_version"])

      config.vm.synced_folder "source/devmaster-" + settings["devmaster_version"], "/var/aegir/devmaster-" + settings["devmaster_version"],
          mount_options: ["uid=12345,gid=12345"]

      config.vm.synced_folder "source/drush", "/var/aegir/.drush/commands",
          mount_options: ["uid=12345,gid=12345"]

      # config.vm.synced_folder "source/projects", "/var/aegir/projects",
      #    mount_options: ["uid=12345,gid=12345"]

      config.vm.provision "shell",
        path: 'vagrant-prepare-guest.sh'
  end

end

class NoSettingsException < Vagrant::Errors::VagrantError
  error_message('Project settings file not found. Create attributes.json file then try again.')
end
