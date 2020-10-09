# DevShop & Ansible

Open DevShop is a full stack system: it requires the server to be configured
 a certain way to work. DevShop contains Ansible roles to do this.

## Source Code
 
DevShop development is done in a single git repository (monorepo).

The Ansible roles are installed in the [roles directory](../../roles):

  1. `opendevshop.devmaster` - Installs a full DevShop Server.
  2. `opendevshop.users` - Installs the special "application" (`aegir`) user. 
  3. `opendevshop.apache` - Installs some extra configuration on top of
   `geerlingguy.apache` to allow full automation of the server config.

The roles are separated so that they can be used for additional remote server
 configuration as well.
 
## Development & Testing

DevShop uses Ansible Molecule to test the roles.

### Installation

The "easiest" way to install Ansible & Molecule accurately is `pip`, the
 Python package system.
 
Try to install with your system's default `pip`. If that doesn't work, get
 `virtualenv`.
 
 #### Option #1: PIP install
 
 ```sh
pip install molecule docker
```
