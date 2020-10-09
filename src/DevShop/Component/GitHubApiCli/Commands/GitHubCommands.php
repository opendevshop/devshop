<?php

namespace DevShop\Component\GitHubApiCli\Commands;

use DevShop\Component\GitHubApiCli\GitHubApiCli;
use Symfony\Component\Console\Exception\InvalidOptionException;
use Symfony\Component\Yaml\Yaml;

class GitHubCommands extends \Robo\Tasks
{

    /**
     * @var \DevShop\Component\GitHubApiCli\GitHubApiCli
     */
    protected $cli;

    /**
     * GitHubCommands constructor.
     */
    public function __construct()
    {
        $this->cli = new GitHubApiCli();
    }

    /**
     * List available APIs.
     */
    public function listApis()
    {

        $this->io()->section('Available APIs');

        try {
            $apis[] = ['current_user', 'currentUser me'];
            $apis[] = ['deployment', 'deployments'];
            $apis[] = ['enterprise', 'ent'];
            $apis[] = ['git', 'git_data gitData'];
            $apis[] = ['gist', 'gists'];
            $apis[] = ['issue', 'issues'];
            $apis[] = ['markdown'];
            $apis[] = ['notification', 'notifications'];
            $apis[] = ['organization', 'organizations'];
            $apis[] = [
              'pull_request',
              'pr pullRequest pullRequests pull_requests',
            ];
            $apis[] = ['rateLimit', 'rate_limit'];
            $apis[] = ['repo', 'repos'];
            $apis[] = ['repository', 'repositories'];
            $apis[] = ['search'];
            $apis[] = ['team', 'teams'];
            $apis[] = ['user', 'users'];
            $apis[] = ['authorization', 'authorizations'];
            $apis[] = ['meta'];
            $this->io()->table(['API Name', 'Aliases'], $apis);
        } catch (\Exception $e) {
            $this->io()->error('Unable to list APIs: '.$e->getMessage());
        }
    }

    /**
     * List available API methods.
     */
    public function listMethods($apiName = null)
    {

        if (!$apiName) {
            $apiName = $this->io()->choice(
              'Which API?',
              $this->cli->getApis(),
              0
            );
        }

        $this->io()->section('Available Methods for API '.$apiName);

        try {
            $api = $this->cli->api($apiName);
            $this->io()->listing($this->cli->getApiMethods($api));
        } catch (\Exception $e) {
            $this->io()->error('Unable to list methods: '.$e->getMessage());
        }
    }

    /**
     * Send an API request.
     *
     * @command api
     * @param string $apiName The name of the specific API to use. See
     *   https://github.com/KnpLabs/php-github-api/blob/master/lib/Github/Client.php#L166
     *   for available options.
     * @param string $apiMethod The API method to call. Depends on the API
     *   used.
     *   Common methods include show, create, update, remove. See the available
     *   AbstractAPI classes at
     *   https://github.com/KnpLabs/php-github-api/tree/master/lib/Github/Api.
     * @param string $apiMethodArgs All additional arguments are passed to the
     *   apiMethod.
     * @option param Add a parameter to pass to the API method in the format NAME=VALUE.
     *
     * @see \Github\Client
     * @see \Github\Client::api()
     */
    public function api(
      $apiName = null,
      $apiMethod = null,
      array $apiMethodArgs,
      $opts = [
        'param|p' => [],
      ]
    ) {
        if (!$apiName) {
            $apiName = $this->io()->choice(
              'Which API?',
              $this->cli->getApis(),
              0
            );
        }

        if (!$apiMethod) {
            $apiMethod = $this->io()->choice(
              'Which API method?',
              $this->cli->getApiMethods($apiName),
              0
            );
        }


        // Validate the API request.
        try {
            $api = $this->cli->api($apiName);
            $apiClass = get_class($api);

            // Validate that the method exists and can be called.
            if (!is_callable([$api, $apiMethod])) {
                if (!method_exists($api, $apiMethod)) {
                    throw new \InvalidArgumentException(
                      "Method $apiMethod does not exist on Class $apiClass."
                    );
                }
                throw new \InvalidArgumentException(
                  "Method $apiMethod on Class $apiClass is private. It cannot be used."
                );
            }

            // Append $params argument to the method args.
            if (!empty($opts['param'])) {
                $params = [];
                foreach ($opts['param'] as $param) {
                    $param_pair = explode('=', $param);
                    if (count($param_pair) != 2) {
                        throw new InvalidOptionException(
                          '--param options must be in the format NAME=VALUE.'
                        );
                    } else {
                        // If the param value has a comma, turn it into an array.
                        // Some github params need to be arrays.
                        if (strpos($param_pair[1], ',') !== false) {
                            $param_pair[1] = array_filter(
                              explode(',', trim($param_pair[1]))
                            );
                        }
                        $params[$param_pair[0]] = $param_pair[1];
                    }
                }

                $apiMethodArgs[] = $params;
            }

            // Validate the number of arguments with reflection.
            $reflection = new \ReflectionMethod($apiClass, $apiMethod);
            $parameters = $reflection->getParameters();

            $apiMethodArgsConfirmed = [];

            $this->yell("Confirming parameters for: $apiClass::$apiMethod()");

            // Confirm arguments
            foreach ($parameters as $i => $arg)
            {
                $default_value = !empty($apiMethodArgs[$i])? $apiMethodArgs[$i]: '';

                // If Method parameter is expecting an array, ask for multiple params.
                $type = $arg->getType();
                if ($type == 'array') {
                    // The last value given.
                    $value = 'none';

                    // The list of all values given.
                    $params = [];

                    // Confirm existing params first
                    foreach ($default_value as $paramName => $paramValue) {
                        $value = $this->askDefault("{$arg->name} (Enter as many as needed. Leave blank to continue.)", "{$paramName}={$paramValue}");

                        // If param has =, explode.
                        if (strpos($value, '=') !== FALSE) {
                            list($key, $value) = explode('=', $value);
                            $params[$key] = $value;
                        }
                        // If not, just set to true.
                        else {
                            $params[$value] = 1;
                        }
                    }

                    // Keep asking until empty value.
                    while (!empty($value)) {
                        $value = $this->ask($arg->name . ' (Enter as many as needed. Leave blank to continue.)');
                        if (empty($value)) continue;

                        // If param has =, explode.
                        if (strpos($value, '=') !== FALSE) {
                            list($key, $value) = explode('=', $value);
                            $params[$key] = $value;
                        }
                        // If not, just set to true.
                        else {
                            $params[$value] = 1;
                        }
                    }

                    // EDGE CASE: Array parameters.
                    // Required Contexts: pass an empty array.
                    if (isset($params['required_contexts'])) {
                        if (empty($params['required_contexts'])) {
                            $params['required_contexts'] = [];
                        }
                        else {
                            // @TODO: Support JSON list of required contexts?
                            $params['required_contexts'] = explode(',', $params['required_contexts']);
                        }
                    }

                    $apiMethodArgsConfirmed[$arg->name] = $params;
                }
                else {
                    if (empty($default_value)) {
                        $apiMethodArgsConfirmed[$arg->name] = $this->ask($arg->name);
                    }
                    else {
                        $apiMethodArgsConfirmed[$arg->name] = $this->askDefault($arg->name, $default_value);
                    }
                }
            }

            $this->objectTable($apiMethodArgsConfirmed, [$apiClass, $apiMethod]);

            // Same as call_user_func_array, only faster!
            // @see https://www.php.net/manual/en/function.call-user-func-array.php#117655
            $apiMethodArgs = array_values($apiMethodArgsConfirmed);
            $results = $api->{$apiMethod}(...$apiMethodArgs);

            // Handle all variable types.
            if (is_object($results)) {
                $this->say(get_class($results));

                // Get methods on the class they chose.
                $methods = [];
                $reflectionClass = new \ReflectionClass($results);
                foreach ($reflectionClass->getMethods() as $method) {
                    $methods[] = $method->getName();
                }

                $method = $this->io()->choice("API Method?", $methods, key($methods));

                // @TODO: Ask for args, if any.
                $params= [];
                $reflectionMethod = new \ReflectionMethod($apiClass, $method);
                foreach ($reflectionMethod->getParameters() as $arg) {
                    $params[] = $this->ask($arg->name);
                }

                $results = $results->{$method}(...$params);


            }
            elseif (is_array($results)) {
                $items = $results;
            }
            else {
                $items= [$results];
            }

            $this->objectTable($results, ["API: ".$apiName, $apiMethod]);

        } catch (\ArgumentCountError $e) {

            $this->io()->error('Command failed: '.$e->getMessage());

            return 1;

        } catch (\Exception $e) {
            $this->io()->error('GitHub API Request failed: '.$e->getMessage());

            if ($this->io()->isDebug()) {
                $this->io()->warning($e->getTraceAsString());
            }

            return 1;
        }

        return 0;
    }

    /**
     * Show the data for the currently authenticated user. (The owner of the
     * token.)
     *
     * @command whoami
     */
    public function whoami()
    {

        /**
         * @var \Github\Api\CurrentUser
         */
        $user = $this->cli->api('me')->show();

        // @TODO: Add a "format" option to return json, yml, or pretty
        $this->objectTable($user);

        return 0;
    }

    /**
     * Prepare an object for display in the CLI.
     *
     * @param $obj
     *
     * @return array
     */
    private function objectTable($items, $headers = [])
    {
        $rows = [];
        foreach ($items as $name => $value) {
            if (is_scalar($value)) {
                $rows[] = [
                  $name,
                  $value
                ];
            } else {
                $rows[] = [
                    $name,
                    Yaml::dump($value, 4, 4, Yaml::DUMP_OBJECT_AS_MAP),
                ];
            }
        }

        return $this->io()->table($headers, $rows);
    }

}

