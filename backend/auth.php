<?php

$headers = getallheaders();

if(!isset($headers["X-Api-Key"]) || $headers["X-Api-Key"] !== $API_KEY) {
    die(error_msg("Invalid API key"));
}

?>