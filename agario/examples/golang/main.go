package main

import (
	"bufio"
	"os"
	"encoding/json"
)


type Strategy struct {}

type Player struct {
	X float64 `json:"X"`
	Y float64 `json:"Y"`
}
type Object struct {
	X float64 `json:"X"`
	Y float64 `json:"Y"`
	Type string `json:"T"`
}
type State struct {
	Mine []Player `json:"Mine"`
	Objects []Object `json:"Objects"`
}
type Command struct {
	X float64 `json:"X"`
	Y float64 `json:"Y"`
	Debug string `json:"Debug"`
}

func run() {
	reader := bufio.NewReader(os.Stdin)
	writer := bufio.NewWriter(os.Stdout)
	_, err := reader.ReadString('\n')
	if err == nil {
		for {
			data, err := reader.ReadString('\n')
			if err == nil {
				var parsed State
				err := json.Unmarshal([]byte(data), &parsed)
				if err == nil {
					command := onTick(parsed)
					result, err := json.Marshal(command)
					if err == nil {
						writer.WriteString(string(result))
						writer.WriteString("\n")
					} else {
						writer.WriteString("Can't Marshal\n")
					}
				} else {
					writer.WriteString("Can't Unmarshal\n")
				}
			} else {
				writer.WriteString("Can't read string\n")
			}
			writer.Flush()
		}
	} else {
		writer.WriteString("Can't read config\n")
	}
}

func onTick(parsed State) (result Command) {
	result.X = 0
	result.Y = 0

	if len(parsed.Mine) > 0 {
		food, err := findFood(parsed.Objects)
		if err != true {
			result.X = food.X
			result.Y = food.Y
		} else {
			result.Debug = "No food"
		}
	} else {
		result.Debug = "Died"
	}
	return
}

func findFood(objects []Object) (food Object, err bool) {
	for _, object := range objects {
		if object.Type == "F" {
			food = object
			err = false
			return
		}
	}
	err = true
	return
}

func main() {
	run()
}