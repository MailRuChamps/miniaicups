using System;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
//using System.Web.Helpers.Json;
using System.IO;

public class Strategy
{
    public static void Main(String[] args)
    {
        
        Random random = new Random();
        while (true)
        {
            var config = Console.ReadLine();
            
            JObject cmd_left = new JObject();
            cmd_left["command"] = "left";
            cmd_left["debug"] = "left";

            JObject cmd_right = new JObject();
            cmd_right["command"] = "right";
            cmd_right["debug"] = "right";

            JObject cmd_stop = new JObject();
            cmd_stop["command"] = "stop";
            cmd_stop["debug"] = "stop";

            JArray commands = new JArray();
            commands.Add(cmd_left);
            commands.Add(cmd_right);
            commands.Add(cmd_stop);



            Console.WriteLine(commands[random.Next(0,commands.Count-1)].ToString(Formatting.None));
        
        }
    }

   
}