<?php

// Holds valid API key and DB credentials
include("config.php");

// Validates API key in request headers (access headers via $headers)
include("auth.php");

// Connects to DB (access DB via $connection)
include("db_connector.php");

// Validate body structure
if(!isset($_POST["wht-uid"])) {
    die_safe(error_msg("incomplete request body"));
}

// Sanitize input field(s)
$uid = $connection->real_escape_string($_POST["wht-uid"]);
$time = strval(time());

// Get last known state for the specified uid
$query = "SELECT state FROM activity_log WHERE uid='$uid' ORDER BY TIME DESC LIMIT 1";
$result = $connection->query($query);

if(!$result) {
    die_safe(error_msg("query failed"));
}

// Default to "1" in case of a previously unseen uid being provided
$state = "1";

// Flip state if there is a row with the specified uid
$row = $result->fetch_assoc();

if(!is_null($row)) {
    $state = $row["state"] == 1 ? "0" : "1";
}

// Insert new row with flipped state
$query = "INSERT INTO activity_log (uid, state, time) VALUES ('$uid', $state, $time)";
$result = $connection->query($query);

if($result) {
    die_safe(json_encode(array(
        "status"    => "ok",
        "message"   => "entry updated",
        "state"     => $state
    )));
} else {
    die_safe(error_msg("query failed"));
}

?>