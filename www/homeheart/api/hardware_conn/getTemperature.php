<?php
//include_once 'system_locations.php';

function getTemperature($id){
	$temp_serial_numbers = [
		"1" => "28-0000017815da",//Inside
		"2" => "28-0000032f098e",//Outside
	];
	$temp_file = "/sys/bus/w1/devices/$temp_serial_numbers[$id]/w1_slave";
	$myFile = file_get_contents($temp_file);
    if ( $myFile != False ){
        $lines = explode(PHP_EOL, $myFile);
		if (preg_match("/t=(-?\d+)/", $lines[1], $match)) {
			return $match[1]/1000;
		} else {
			return "Match was not found.";
		}
    } else {
		return "File not found";
	} 
}
?>