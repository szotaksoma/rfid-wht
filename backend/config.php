<?php

$DB_HOST = "";
$DB_NAME = "";
$DB_PASSWORD = "";
$DB_USER = "";
$API_KEY = "";

// Template for standard status message (error)
function error_msg($message) {
    return json_encode(
        array(
            "status"    => "error",
            "message"   => $message
        )
    );
}

// Template for standard status message (ok)
function ok_msg($message) {
    return json_encode(
        array(
            "status"    => "OK",
            "message"   => $message
        )
    );
}

?>