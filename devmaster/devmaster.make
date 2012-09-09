core = 6.x
api = 2

projects[drupal][type] = "core"
projects[drupal][version] = "6.26"

; DevShop modules
projects[devshop_hosting][version] = "1.9-alpha1"

; Aegir Hostmaster modules
; This includes all hostmaster contrib and libraries!
projects[hostmaster][version] = "1.9"

; Patch: Issue #1760962: Any Hosting Task can call provision save.
projects[hostmaster][patch][] = "http://drupalcode.org/project/devshop_hosting.git/blob_plain/HEAD:/1760962-hosting-task-provision-save-1.9.patch"

; Patch: Issue #1513678: Hosting Task Names cannot use dashes and use Hosting Task Forms hook.
projects[hostmaster][patch][] = "http://drupalcode.org/project/devshop_hosting.git/blob_plain/HEAD:/1513678-hosting-task-names-1.9.patch"

; Patch: Issue #1778400: Hosting Task Names cannot use dashes and use pre and post hooks
projects[hostmaster][patch][] = "http://drupalcode.org/project/devshop_hosting.git/blob_plain/HEAD:/1778400-hosting-tasks-names-hooks-1.9.patch"


