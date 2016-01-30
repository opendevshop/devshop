;
; This makefile is by developers to test pull requests
core = 6.x
api = 2
projects[drupal][type] = "core"

;
; Before submitting a Pull Request, change this URL to your fork.
; Before we merge your pull request, change this URL back to the opendevshop/devmaster repo.
projects[devmaster][type] = "profile"
projects[devmaster][download][type] = "git"
projects[devmaster][download][branch] = "0.x-tests"
projects[devmaster][download][url] = "https://github.com/opendevshop/devmaster.git"
