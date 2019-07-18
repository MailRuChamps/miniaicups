<?php

while (1) {
    $line = trim(fgets(STDIN));
    $parsed = json_decode($line);
    $commands = array('right', 'left', 'up', 'down');
    $rand_key = array_rand($commands);
    $command = array('command' => $commands[$rand_key], 'debug' => 'debug_msg');
    print json_encode($command)."\n";
}

?>