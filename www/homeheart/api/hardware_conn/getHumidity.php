<?php

function getHumidity(){
	for ($i = 1; $i <= 3; $i++) {
		$hum_file = "/dev/dht11";
		$myFile = file_get_contents($hum_file);
		if ( $myFile != False ){
			if (preg_match("/Values: \d+, \d+, \d+, \d+, (\d+), (OK|BAD)/", $myFile, $match)) {
				if ( $match[2] == 'OK') {
					return (int) $match[1];
				}
			} else {
				return "Match was not found.";
			}
		} else {
			return "File not found";
		} 
	}
}

?>