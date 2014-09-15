# -*- mode: ruby -*-
# vi: set ft=ruby :

# Vagrantfile API/syntax version. Don't touch unless you know what you're doing!
VAGRANTFILE_API_VERSION = "2"

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|

  # config.vm.box = "chef/centos-7.0"
  config.vm.box = "hashicorp/precise64"

  config.vm.network "private_network", ip: "20.20.20.20"
  config.vm.network "public_network"

  # config.vm.provision "shell",
  #    path: "../repos/devshop/install.centos.sh"

  require 'yaml'
  settings = YAML.load_file(File.dirname(__FILE__) + "/vars.yml")
  ssh_public_key = IO.read("#{Dir.home}/.ssh/id_rsa.pub").strip!

  config.vm.hostname = settings['server_hostname']

  # ONLY WORKS if ansible is setup on the HOST machine.
  # See https://github.com/mitchellh/vagrant/issues/2103
  config.vm.provision "ansible" do |ansible|
     ansible.playbook = settings['ansible_playbook']
     ansible.extra_vars = {
       ansible_ssh_user: 'vagrant',
       authorized_keys: ssh_public_key
     }
     ansible.sudo = true
   end
end
