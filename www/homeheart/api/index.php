<?php
include_once "hardware_conn/getTemperature.php";
include_once "hardware_conn/getHumidity.php";

$app = new Phalcon\Mvc\Micro();

$app->get('/temp/{id}', function ($id) {
	$file = file_get_contents('/home/pi/rpi-scripts/measurements.json');
    $measurements = json_decode($file, true)["measurements"];
    echo $measurements["temperature_$id"];
});

$app->get('/hum', function () {
    $file = file_get_contents('/home/pi/rpi-scripts/measurements.json');
    $measurements = json_decode($file, true)["measurements"];
    echo $measurements["humidity"];
});

$app->get('/pressure', function () {
    $file = file_get_contents('/home/pi/rpi-scripts/measurements.json');
    $measurements = json_decode($file, true)["measurements"];
    echo $measurements["pressure"];
});

$app->get('/all', function () {
    $file = file_get_contents('/home/pi/rpi-scripts/measurements.json');
    echo $file;
});
$app->handle();


//Retrieves all robots
// $app->get('/api/robots', function() {

// });

//Searches for robots with $name in their name
// $app->get('/api/robots/search/{name}', function($name) {

// });

//Retrieves robots based on primary key
// $app->get('/api/robots/{id:[0-9]+}', function($id) {

// });

//Adds a new robot
// $app->post('/api/robots', function() {

// });

//Updates robots based on primary key
// $app->put('/api/robots/{id:[0-9]+}', function() {

// });

//Deletes robots based on primary key
// $app->delete('/api/robots/{id:[0-9]+}', function() {

// });

 //$app->handle();