# -*- mode: ruby -*-
# vi: set ft=ruby :
Vagrant::Config.run do |config|
  # Get attributes from attributes.json
  attributes = JSON.parse(IO.read("attributes.json"))

  # Base Box
  config.vm.box = "precise64"
  config.vm.box_url = "http://files.vagrantup.com/precise64.box"

  # Networking & hostname
  config.vm.network :bridged, :bridge => attributes["vagrant"]["adapter"]
  config.vm.network :hostonly, attributes["vagrant"]["hostonly_ip"]
  config.vm.host_name = attributes["vagrant"]["hostname"]

  # Set SH as our provisioner
  # Until this is resolved, you must run the installer interactively.
  config.vm.provision "shell", path: "repos/devshop/install.debian.sh"

  # @TODO: Release chef recipes for devshop servers.
  # Set Chef as our provisioner
  # config.vm.provision :chef_solo do |chef|

    # Cookbooks folder is expected to be in the same folder as this file.
    # chef.cookbooks_path = "cookbooks"

    # Add "devshop" recipe.
    # chef.add_recipe "devshop"

    # Pass attributes from json to Chef.
    # chef.json = attributes
  # end

  # Make local source code available to the VM
  config.vm.share_folder "repos", "/repos",  "repos", :owner => "www-data", :group => "www-data"
end
