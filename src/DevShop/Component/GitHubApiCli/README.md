GitHub API CLI Component
=================

The GitHubApiCli component a simple console wrapper for the GitHub API to allow 
abstract command line interaction.

Built on knp-labs/github-api.

## Usage

```
composer require devshop/github-api-cli
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

### `github api`

Passes directly to `GitHub\Api\AbstractAPI classes.

```sh
 ./github api me
 --------------------------- ---------------------------------------------------------------- 
  Name                        Value                                                           
 --------------------------- ---------------------------------------------------------------- 
  login                       devshopbot                                                      
  id                          11931385                                                        
  node_id                     MDQ6VXNlcjExOTMxMzg1                                            
  avatar_url                  https://avatars2.githubusercontent.com/u/11931385?v=4       
```

Only "show" works right now, but it should successfully return any object.

```
/github api user show jonpugh
 --------------------- ------------------------------------------------------------- 
  Name                  Value                                                        
 --------------------- ------------------------------------------------------------- 
  login                 jonpugh                                                      
  id                    106420                                                       
  node_id               MDQ6VXNlcjEwNjQyMA==                                         
  avatar_url            https://avatars2.githubusercontent.com/u/106420?v=4          
  gravatar_id                                                                        
  url                   https://api.github.com/users/jonpugh                         
  html_url              https://github.com/jonpugh                                   
  followers_url         https://api.github.com/users/jonpugh/followers               
```

Load info about an organization:

```
./github api organization show department-of-veterans-affairs
 --------------------------- ------------------------------------------------------------------------------------ 
  Name                        Value                                                                               
 --------------------------- ------------------------------------------------------------------------------------ 
  login                       department-of-veterans-affairs                                                      
  id                          5421563                                                                             
  node_id                     MDEyOk9yZ2FuaXphdGlvbjU0MjE1NjM=                                                    
  url                         https://api.github.com/orgs/department-of-veterans-affairs                          
  repos_url                   https://api.github.com/orgs/department-of-veterans-affairs/repos                    
```

Show info about a repo:
```
./github api repo show department-of-veterans-affairs va.gov-cms
 ------------------- ----------------------------------------------------------------------------------------------------------------- 
  Name                Value                                                                                                            
 ------------------- ----------------------------------------------------------------------------------------------------------------- 
  id                  154174777                                                                                                        
  node_id             MDEwOlJlcG9zaXRvcnkxNTQxNzQ3Nzc=                                                                                 
  name                va.gov-cms                                                                                                       
  full_name           department-of-veterans-affairs/va.gov-cms                                                                        
  private                                                                                                                              
  html_url            https://github.com/department-of-veterans-affairs/va.gov-cms                                                     
```

### `github deployment:start`

Creates a new deployment and saves the ID as a git config item.

    bin/github deployment:start --environment=dev --description="Manual deployment triggered as an example."

Then use `deployment:update` to change the status.

### `github deployment:update`

Updates the status of a deployment.

    bin/github deployment:update --state=in_progress --description="Building our thing..." --log_url=$OUR_BUILD_LOGS_URL

You can use bash operators to set different state based on exit code of a script.

Use `&&` after a command to designate a command to run on success.
Use `||` after a command to designate a command to run on failure.

    deploy.sh \
       && bin/github deployment:update --state=success --description="This deployment update will only happen if deploy.sh returns exit code 0" \
       || ( bin/github deployment:update --state=failure --description="This deployment update will only happen if deploy.sh returns exit 1." && exit 1 )

See The DevShop Build tests as an example https://github.com/opendevshop/devshop/blob/09fbf10286994c378fa24cf6b9f91e9df33928dd/.github/workflows/build.yml#L110

If you are using a CI runner, be sure to `exit 1` after the failure command, to
ensure the overall script exits with a failure as well.


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
