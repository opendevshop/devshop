<?php
/**
 * Copyright (c) 2010, SoftLayer Technologies, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither SoftLayer Technologies, Inc. nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Start by including the API client class. This example assumes that the
 * SoftLayer API classes are in the directory "SoftLayer" relative to this
 * script's path.
 *
 * If you wish to use the XML-RPC API then replace mentions of
 * SoapClient.class.php with XmlrpcClient.class.php and SoftLayer_SoapClient
 * with SoftLayer_XmlrpcClient.
 */
require_once dirname(__FILE__) . '/SoftLayer/SoapClient.class.php';

/**
 * It's possible to define your SoftLayer API username and key diredtly in the
 * class file, but it's far easier to define them before creating your API
 * client.
 */
$apiUsername = 'set me';
$apiKey = 'set me too';

/**
 * Usage:
 * SoftLayer_SoapClient::getClient([API Service], <object id>, [username], [API key]);
 *
 * API Service: The name of the API service you wish to connect to.
 * id:          An optional id to initialize your API service with, if you're
 *              interacting with a specific object. If you don't need to specify
 *              an id then pass null to the client.
 * username:    Your SoftLayer API username.
 * API key:     Your SoftLayer API key,
 */
$client = SoftLayer_SoapClient::getClient('SoftLayer_Account', null, $apiUsername, $apiKey);

/**
 * Once your client object is created you can call API methods for that service
 * directly against your client object. A call may throw an exception on error,
 * so it's best to try your call and catch exceptions.
 *
 * This example calls the getObject() method in the SoftLayer_Account API
 * service. <http://sldn.softlayer.com/reference/services/SoftLayer_Account/getObject>
 * It retrieves basic account information, and is a great way to test your API
 * account and connectivity.
 */
try {
    print_r($client->getObject());
} catch (Exception $e) {
    die($e->getMessage());
}

/**
 * For a more complex example we’ll retrieve a support ticket with id 123456
 * along with the ticket’s updates, the user it’s assigned to, the servers
 * attached to it, and the datacenter those servers are in. We’ll retrieve our
 * extra information using a nested object mask. After we have the ticket we’ll
 * update it with the text ‘Hello!’.
 */

// Declare an API client to connect to the SoftLayer_Ticket API service.
$client = SoftLayer_SoapClient::getClient('SoftLayer_Ticket', 123456, $apiUsername, $apiKey);

// Assign an object mask to our API client:
$objectMask = new SoftLayer_ObjectMask();
$objectMask->updates;
$objectMask->assignedUser;
$objectMask->attachedHardware->datacenter;
$client->setObjectMask($objectMask);

// Retrieve the ticket record.
try {
    $ticket = $client->getObject();
    print_r($ticket);
} catch (Exception $e) {
    die('Unable to retrieve ticket record: ' . $e->getMessage());
}

// Now update the ticket.
$update = new stdClass();
$update->entry = 'Hello!';

try {
    $update = $client->addUpdate($update);
    echo "Updated ticket 123456. The new update's id is " . $update[0]->id . '.';
} catch (Exception $e) {
    die('Unable to update ticket: ' . $e->getMessage());
}
