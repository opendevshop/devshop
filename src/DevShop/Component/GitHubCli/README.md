GitHubCli Component
=================

The GitHubCli component a simple console wrapper for the GitHub API to allow 
abstract command line interaction.

Built on knp-labs/github-api.

## Usage

```
composer require devshop/github-cli
export GITHUB_TOKEN=aRealToken
bin/github whoami

--------------------------- ---------------------------------------------------------------- 
  Name                        Value                                                           
 --------------------------- ---------------------------------------------------------------- 
  login                       devshopbot                                                      
  id                          11931385                                                        
  node_id                     MDQ6VXNlcjExOTMxMzg1                                            
  avatar_url                  https://avatars2.githubusercontent.com/u/11931385?v=4           
  gravatar_id                                                                                 
  url                         https://api.github.com/users/devshopbot                         
  html_url                    https://github.com/devshopbot                                   
  followers_url               https://api.github.com/users/devshopbot/followers               
  following_url               https://api.github.com/users/devshopbot/following{/other_user}  
  gists_url                   https://api.github.com/users/devshopbot/gists{/gist_id}         
  starred_url                 https://api.github.com/users/devshopbot/starred{/owner}{/repo}  
  subscriptions_url           https://api.github.com/users/devshopbot/subscriptions           
  organizations_url           https://api.github.com/users/devshopbot/orgs                    
  repos_url                   https://api.github.com/users/devshopbot/repos                   
  events_url                  https://api.github.com/users/devshopbot/events{/privacy}        
  received_events_url         https://api.github.com/users/devshopbot/received_events         
  type                        User                                                            
  site_admin                                                                                  
  name                        devshopbot                                                      
  company                                                                                     
  blog                                                                                        
  location                                                                                    
  email                                                                                       
  hireable                                                                                    
  bio                                                                                         
  public_repos                2                                                               
  public_gists                0                                                               
  followers                   0                                                               
  following                   0                                                               
  created_at                  2015-04-13T20:05:05Z                                            
  updated_at                  2020-02-28T17:13:13Z                                            
  private_gists               0                                                               
  total_private_repos         0                                                               
  owned_private_repos         0                                                               
  disk_usage                  0                                                               
  collaborators               0                                                               
  two_factor_authentication                                                                   
 --------------------------- ---------------------------------------------------------------- 


```

## Commands

The CLI will abstractly pass all commands to the GitHub API in an abstract manner.

It also has custom commands like `whoami` that are contained in the GitHubCommands.php file.

Stand by for more.

Resources
---------

  * [Documentation](https://github.com/opendevshop/devshop/blob/develop/README.md)
  * [Contributing](https://github.com/opendevshop/devshop/blob/develop/docs/DEVELOPING.md)
  * [Report issues](https://github.com/opendevshop/devshop/issues) and
    [send Pull Requests](https://github.com/opendevshop/devshop/pulls)
    in the [main DevShop repository](https://github.com/opendevshop/devshop)

Credits
-------

$CREDITS 
