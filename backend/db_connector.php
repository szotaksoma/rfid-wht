<?php

$connection = mysqli_connect($DB_HOST, $DB_USER, $DB_PASSWORD, $DB_NAME);

if(mysqli_connect_errno()) {
    die(error_msg("Database connection failed"));
}

// Close connection and die
function die_safe($message) {
    global $connection;
    $connection->close();
    die($message);
}

?>