<?php

// Holds valid API key and DB credentials
include("config.php");

// Validates API key in request headers (access headers as $headers)
include("auth.php");

// Connects to DB (access DB as $connection)
include("db_connector.php");

if(!isset($_POST["wht-uid"]) || !isset($_POST["wht-from-time"]) || !isset($_POST["wht-to-time"])) {
    die(error_msg("incomplete request body"));
}

$uid = $connection->real_escape_string($_POST["wht-uid"]);
$from_time = strval($connection->real_escape_string($_POST["wht-from-time"]));
$to_time = strval($connection->real_escape_string($_POST["wht-to-time"]));

$query = "SELECT state, time FROM activity_log WHERE uid='$uid' AND time >= $from_time AND time <= $to_time ORDER BY time ASC";
$result = $connection->query($query);

if(!$result) {
    $connection->close();
    die(error_msg("query failed"));
}

$rows = $result->fetch_all(1);

$csv = array();
$interval_id = 0;
$row_from_time = 0;
$row_to_time = 0;

$row_date = "";
$row_from_date = "";
$row_to_date = "";

$csv[] = "date,interval_id,interval_start,interval_end,delta";

foreach($rows as $row) {
    if($row["state"] === "1") {
        $row_from_time = intval($row["time"]);
    } else if($row["state"] === "0") {
        $row_date = date("Y. m. d.", $row_from_time);
        $row_to_time = intval($row["time"]);
        $row_from_date = date("G:i:s", $row_from_time);
        $row_to_date = date("G:i:s", $row_to_time);
        $delta = $row_to_time - $row_from_time;
        $csv[] = "$row_date,$interval_id,$row_from_date,$row_to_date,$delta";
        $interval_id += 1;
    }
}

$filename = "$uid-$from_time-$to_time.csv";
file_put_contents($filename, implode($csv, "\n"));
readfile($filename);
unlink($filename);

die();

?>