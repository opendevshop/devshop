# -*- mode: ruby -*-
# vi: set ft=ruby :

# Vagrantfile API/syntax version. Don't touch unless you know what you're doing!
VAGRANTFILE_API_VERSION = "2"

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|

  # debian.vm.network "public_network"

  require 'yaml'
  settings = YAML.load_file(File.dirname(__FILE__) + "/vars.yml")
  ssh_public_key = IO.read("#{Dir.home}/.ssh/id_rsa.pub").strip!

  config.vm.define "debian" do |debian|
    debian.vm.box = "hashicorp/precise64"
    debian.vm.network "private_network", ip: "20.20.20.20"
    debian.vm.hostname = "devshop.local"

    debian.vm.provision "ansible" do |ansible|
      ansible.playbook = settings['ansible_playbook']
      ansible.extra_vars = {
        ansible_ssh_user: 'vagrant',
        authorized_keys: ssh_public_key
      }
      ansible.sudo = true
    end
  end

  config.vm.define "redhat" do |redhat|
    redhat.vm.box = "chef/centos-7.0"
    redhat.vm.network "private_network", ip: "30.30.30.30"
    redhat.vm.hostname = "devshop.redhat"

    redhat.vm.provision "ansible" do |ansible|
      ansible.playbook = settings['ansible_playbook']
      ansible.extra_vars = {
        ansible_ssh_user: 'vagrant',
        authorized_keys: ssh_public_key
      }
      ansible.sudo = true
    end
  end
end
