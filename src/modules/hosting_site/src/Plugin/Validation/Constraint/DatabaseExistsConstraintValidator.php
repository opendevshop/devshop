<?php

declare(strict_types=1);


namespace Drupal\hosting_site\Plugin\Validation\Constraint;

use Drupal\Core\Database\Database;
use Drupal\Core\Database\DatabaseExceptionWrapper;
use Drupal\Core\Field\FieldDefinitionInterface;
use Drupal\Core\Messenger\Messenger;
use Symfony\Component\Validator\Constraint;
use Symfony\Component\Validator\ConstraintValidator;

/**
 * Validates the Database Exists constraint.
 */
final class DatabaseExistsConstraintValidator extends ConstraintValidator {

  /**
   * {@inheritdoc}
   */
  public function validate(mixed $field, Constraint $constraint): void {

    $database = \Drupal::database();
    $current_connection = $database->getConnectionOptions();

    $new_database_name = $field->value;
    $new_database_name = preg_replace("/[^a-zA-Z]+/", "", $new_database_name);
    $new_database_user = $current_connection['username'];
    $new_database_password = $current_connection['password'];

    // @TODO: This only gets set when "refresh data" is clicked. Editing/creating doesn't seem to alter the value that is saved.
    $field->getEntity()->set('database_name', $new_database_name);

    try {
      // Check that the database exists and can be accessed.
      $new_connection = $current_connection;
      $new_connection['database'] = $new_database_name;

      Database::addConnectionInfo($new_database_name, 'default', $new_connection);
      Database::setActiveConnection($new_database_name);
      $connection = Database::getConnection('default', $new_database_name);

      $tables = $connection->query('SHOW TABLES')->fetchAll();
      \Drupal::messenger()->addMessage(t("Successfully connected to database :database. There are :tables tables in the dB", [
        ':database' => $new_database_name,
        ':tables' => count($tables),
      ]));

      // @TODO: Should we set state to "not installed"?
      if (count($tables) == 0) {
        \Drupal::messenger()->addMessage(t("No data was found in the database :database. Try visiting the site to install data.", [
          ':database' => $new_database_name,
        ]));
      }
    }
    catch (\PDOException $e) {

      Database::setActiveConnection('creator');
      $database = Database::getConnection($new_database_name);

      // Cannot access. Try to create.
      try {
        $query = $database->query("GRANT ALL ON $new_database_name.* TO '$new_database_user'@'%' IDENTIFIED BY '$new_database_password'");
        $query->execute();
        \Drupal::messenger()->addMessage(t("User was granted access to :database.", [
          ':database' => $new_database_name,
        ]));
        $database->createDatabase($new_database_name);
        \Drupal::messenger()->addMessage(t("Database :database was created.", [
          ':database' => $new_database_name,
        ]));

        // Reset database
        $field->getEntity()->set('database_name', $new_database_name);

        return;

      }
      catch (DatabaseExceptionWrapper $e) {
        $this->context->addViolation($constraint->messageCannotCreate, [
          '%value' => $new_database_name,
          '%error' => $e->getMessage(),
        ]);
      }
    }
    catch (DatabaseExceptionWrapper $e) {
      // Cannot access. Try to create.
      $this->context->addViolation($constraint->messageCannotAccess, [
        '%value' => $new_database_name,
        '%error' => $e->getMessage(),
      ]);
    }
  }
}
