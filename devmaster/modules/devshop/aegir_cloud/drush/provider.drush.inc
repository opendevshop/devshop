<?php
///**
// * @file
// * Drush commands for DevShop Cloud.
// */
//
///**
// * Implements hook_drush_init()
// * - Ensures that provision is loaded before devshop_provision
// */
//function provider_drush_init(){
//  provider_register_autoload();
//}
//
///**
// * Register our directory as a place to find provision classes.
// */
//function provider_register_autoload() {
//  static $loaded = FALSE;
//  if (!$loaded) {
//    $loaded = TRUE;
//    provision_autoload_register_prefix('Provision_', dirname(__FILE__));
//  }
//}
//
///**
// * Implementation of hook_drush_command().
// */
//function provider_drush_command() {
//  $items = array();
//  return $items;
//}
//
///**
// * Implementation of hook_provision_services()
// *
// *  Declares a new service type and a basic implementation of it.
// *  It matches the same service definition in the hosting front end.
// */
//function provider_provision_services() {
//  provider_register_autoload();
//  return array('provider' => NULL);
//}
