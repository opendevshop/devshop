<?php
/**
 * Copyright (c) 2009 - 2010, SoftLayer Technologies, Inc. All rights reserved.
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
 *    specific prior written permission.
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
 * Support for asynchronous SoftLayer SOAP API calls
 *
 * PHP SOAP calls operate in a send -> receive style of transmission. Response
 * time for a call is dependent on the latency between the SOAP client and SOAP
 * server and the time required by the server to process the SOAP call. Sending
 * multiple SOAP calls in serial over the public Internet can be a time
 * consuming process.
 *
 * Asynchronous calls allow you to send multiple SOAP calls in parallel to the
 * SoftLayer API. Parallel calls reduce the latency involved handling multiple
 * calls to the time it takes for the longest SOAP call to execute, dramatically
 * reducing the time it takes to send multiple SOAP calls in most cases.
 *
 * Asynchronous calls are handled identically to standard API calls with two
 * differences:
 *
 * 1) The SoftLayer_SoapClient class knows to make an asynchronous call when the
 * method called ends with "Async". For example, to make a standard call to the
 * method getObject() you would execute $client->geObject(). It's asynchronous
 * counterpart is execued with the code $client->getObjectAsync(). Once the
 * asynchronous call is made the results of your API command are sent to this
 * classes' socket property.
 *
 * 2) The results of an asynchronous method call are stored in a
 * SoftLayer_SoapClient_AsynchronousAction object. Use the wait() method to
 * retrieve data off the internal socket and return the result back to the
 * SoftLayer_SoapClient for processing. For example if you wish to retrieve the
 * results of the method getObject() execute the following statements:
 *
 * $result = $client->getObjectAsync(); // Make the call and start geting data back.
 * $result = $result->wait(); // Return the results of the API call
 *
 * To chain multiple asynchronous requests together call multiple Async requests
 * in succession then call their associated wait() methods in succession.
 *
 * Here's a simple usage example that retrieves account information, a PDF of an
 * account's next invoice and enables VLAN spanning on that same account by
 * calling three methods in the SoftLayer_Account service in parallel:
 *
 * ----------
 *
 * // Initialize an API client for the SoftLayer_Account service.
 * $client = SoftLayer_SoapClient::getClient('SoftLayer_Account');
 *
 * try {
 *     // Request our account information.
 *     $account = $client->getObjectAsync();
 *
 *     // Request a PDF of our next invoice. This can take much longer than
 *     // getting simple account information.
 *     $nextInvoicePdf = $client->getNextInvoicePdfAsync();
 *
 *     // While we're at it we'll enable VLAN spanning on our account.
 *     $vlanSpanResult = $client->setVlanSpanAsync(true);
 *
 *     // The three requests are now processing in parallel. Use the wait()
 *     // method to retrieve the resuls of our requests. The wait time involved
 *     // is roughly the same time as the longest API call.
 *     $account = $account->wait();
 *     $nextInvoicePdf = $nextInvoicePdf->wait();
 *     $vlanSpanResult = $vlanSpanResult->wait();
 *
 *     // Finally, display our results.
 *     var_dump($account);
 *     var_dump($nextInvoicePdf);
 *     var_dump($vlanSpanResult);
 * } catch (Exception $e) {
 *     die('Unable to retrieve account information: ' . $e->getMessage());
 * }
 *
 * ----------
 *
 * The most up to date version of this library can be found on the SoftLayer
 * github public repositories: http://github.com/softlayer/ . Please post to
 * the SoftLayer forums <http://forums.softlayer.com/> or open a support ticket
 * in the SoftLayer customer portal if you have any questions regarding use of
 * this library.
 *
 * @author      SoftLayer Technologies, Inc. <sldn@softlayer.com>
 * @copyright   Copyright (c) 2009 - 2010, Softlayer Technologies, Inc
 * @license     http://sldn.softlayer.com/article/License
 * @see         SoftLayer_SoapClient
 */
class SoftLayer_SoapClient_AsynchronousAction
{
    /**
     * The SoftLayer SOAP client making an asynchronous call
     *
     * @var SoftLayer_SoapClient
     */
    protected $_soapClient;

    /**
     * The name of the function we're calling
     *
     * @var string
     */
    protected $_functionName;

    /**
     * A socket connection to the SoftLayer SOAP API
     *
     * @var resource
     */
    protected $_socket;

    /**
     * Perform an asynchgronous SoftLayer SOAP call
     *
     * Create a raw socket connection to the URL specified by the
     * SoftLayer_SoapClient class and send SOAP HTTP headers and request XML to
     * that socket. Throw exceptions if we're unable to make the socket
     * connection or send data to that socket.
     *
     * @param SoftLayer_SoapClient $soapClient The SoftLayer SOAP client making the asynchronous call.
     * @param string $functionName The name of the function we're calling.
     * @param string $request The full XML SOAP request we wish to make.
     * @param string $location The URL of the web service we wish to call.
     * @param string $action The value of the HTTP SOAPAction header in our SOAP call.
     */
    public function __construct($soapClient, $functionName, $request, $location, $action)
    {
        preg_match('%^(http(?:s)?)://(.*?)(/.*?)$%', $location, $matches);

        $this->_soapClient = $soapClient;
        $this->_functionName = $functionName;

        $protocol   = $matches[1];
        $host       = $matches[2];
        $endpoint   = $matches[3];

        $headers = array(
            'POST ' . $endpoint . ' HTTP/1.1',
            'Host: ' . $host,
            'User-Agent: PHP-SOAP/' . phpversion(),
            'Content-Type: text/xml; charset=utf-8',
            'SOAPAction: "' . $action . '"',
            'Content-Length: ' . strlen($request),
            'Connection: close',
        );

        if ($protocol == 'https') {
            $host = 'ssl://' . $host;
            $port = 443;
        } else {
            $port = 80;
        }

        $data = implode("\r\n", $headers) . "\r\n\r\n" . $request . "\r\n";
        $this->_socket = fsockopen($host, $port, $errorNumber, $errorMessage);

        if ($this->_socket === false) {
            $this->_socket = null;
            throw new Exception('Unable to make an asynchronous SoftLayer API call: ' . $errorNumber . ': ' . $errorMessage);
        }

        if (fwrite($this->_socket, $data) === false) {
            throw new Exception('Unable to write data to an asynchronous SoftLayer API call.');
        }
    }

    /**
     * Process and return the results of an asyncrhonous SoftLayer API call
     *
     * Read data from our socket and process the raw SOAP result from the
     * SoftLayer_SoapClient instance that made the asynchronous call. wait()
     * *must* be called in order to recieve the results from your API call.
     *
     * @return object
     */
    public function wait()
    {
        $soapResult = '';

        while (!feof($this->_socket)) {
            $soapResult .= fread($this->_socket, 8192);
        }

        // separate the SOAP result into headers and data.
        list($headers, $data) = explode("\r\n\r\n", $soapResult);

        return $this->_soapClient->handleAsyncResult($this->_functionName, $data);
    }

    /**
     * Close the socket created when the SOAP request was created.
     */
    public function __destruct()
    {
        if ($this->_socket != null) {
            fclose($this->_socket);
        }
    }
}
