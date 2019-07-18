import Foundation

while true {
    let commands: [String] = ["left", "right", "up", "down"]
    let command = commands.randomElement()!
    let dictionary = ["command": command]
    let jsonData = try? JSONSerialization.data(withJSONObject: dictionary, options: [])
    let jsonString = String(data: jsonData!, encoding: .utf8)
    print(jsonString!)
}