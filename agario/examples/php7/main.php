<?php
    class Strategy {
    	public function run() {
            $line = trim(fgets(STDIN));
            $config = json_decode($line);
    		while (1) {
    			$line = trim(fgets(STDIN));
    			$parsed = json_decode($line);
    			$command = $this->on_tick($parsed, $config);
				print json_encode($command)."\n";
    		}
    	}

        public function on_tick($parsed, $config) {
            if (!empty($parsed->Mine)) {
            	$food = $this->find_food($parsed->Objects);
            	if (!is_null($food)) {
            		return array('X' => $food->X, 'Y' => $food->Y);
            	}
            	return array('X' => 0, 'Y' => 0, 'Debug' => 'No food');
            }
            return array('X' => 0, 'Y' => 0, 'Debug' => 'Died');
        }

        public function find_food($objects) {
        	for ($i = 0, $size = count($objects); $i < $size; ++$i) {
        		if ($objects[$i]->T == 'F') {
        			return $objects[$i];
        		}
        	}
        	return null;
        }
    }

    $strategy = new Strategy();
    $strategy->run();
?>
