# -*- mode: ruby -*-
# vi: set ft=ruby :
Vagrant::Config.run do |config|

  # Attributes are loaded from attributes.json
  if !(File.exists?("attributes.json"))
    warn "Make sure you have an attributes.json file and try again."
    exit
  end

  # Check for repos folder.
  if !(File.exists?("repos"))
    warn "Run `sh init-repos.sh` to clone the needed source code to the host."
    exit
  end

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

  # Make local source code available to the VM
  config.vm.share_folder "repos", "/repos",  "repos", :owner => "www-data", :group => "www-data"

  config.vm.share_folder "devshop_hosting", "/var/aegir/devshop-6.x-1.x/profiles/devshop/modules/contrib/devshop_hosting",  "repos/devshop_hosting", :owner => "aegir", :group => "aegir"
  config.vm.share_folder "devshop_provision", "/var/aegir/.drush/devshop_provision",  "repos/devshop_provision", :owner => "aegir", :group => "aegir"

end
