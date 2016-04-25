#!/bin/sh
#
# Altered from https://github.com/acquia/cloud-hooks/tree/master/samples/slack
# Requires a /var/aegir/slack_settings file that contains:
#
# SLACK_WEBHOOK_URL=https://example.slack.com/services/hooks/incoming-webhook?token=TOKEN
#
# Cloud Hook: post-code-deploy
#
# The post-code-deploy hook is run whenever you use the Workflow page to
# deploy new code to an environment, either via drag-drop or by selecting
# an existing branch or tag from the Code drop-down list. See
# ../README.md for details.
#
# Usage: post-code-deploy site target-env source-branch deployed-tag repo-url
#                         repo-type

site="$1"
target_env="$2"
source_branch="$3"
deployed_tag="$4"
repo_url="$5"
repo_type="$6"
source="$7"
site_url="$8"

# Load the Slack webhook URL (which is not stored in this repo).
. $HOME/slack_settings

if [ $source = 'devshop' ]; then
  source="DevShop"
  image="https://www.drupal.org/files/project-images/devshop-icon.png"
else
  source="Acquia Cloud"
  image="https://pbs.twimg.com/profile_images/1901642489/cloud_icon_150.png"
fi

# Post deployment notice to Slack
curl -X POST --data-urlencode "payload={\"channel\": \"#dev-engageny\", \"username\": \"$source\", \"text\": \"Git branch \`$deployed_tag\` updated on *$target_env*: $site_url.\", \"icon_url\": \"$image\"}" $SLACK_WEBHOOK_URL
