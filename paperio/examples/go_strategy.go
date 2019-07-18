package main

import (
	"encoding/json"
	"fmt"
	"math/rand"
	"bufio"
	"os"
)


func main() {
	rand.Seed(42)
	commands := [4]string{"left", "right", "up", "down"}
	reader := bufio.NewReader(os.Stdin)
	for {
		text, _ := reader.ReadString('\n')
	    mapD := map[string]string{"command": commands[rand.Intn(4)], "debug": text}
	    mapB, _ := json.Marshal(mapD)
	    fmt.Println(string(mapB))
	}
}