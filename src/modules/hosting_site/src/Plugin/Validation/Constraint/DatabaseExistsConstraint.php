<?php

declare(strict_types=1);

namespace Drupal\hosting_site\Plugin\Validation\Constraint;

use Symfony\Component\Validator\Constraint;

/**
 * Provides a Database Exists constraint.
 *
 * @Constraint(
 *   id = "HostingSiteDatabaseExists",
 *   label = @Translation("Database Exists", context = "Validation"),
 * )
 */
final class DatabaseExistsConstraint extends Constraint {

  public string $messageCannotCreate = 'Cannot create database %value. The message was: %error';
  public string $messageNotFound = 'Database %value does not exist. The message was: %error';
  public string $messageCannotAccess = 'Cannot access database %value. The message was: %error';

}
