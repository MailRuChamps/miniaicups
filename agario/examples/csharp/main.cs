using System;
using Newtonsoft.Json.Linq;
using System.IO;
 
public class Strategy
{
    public static void Main (String[] args)
    {
        var config = Console.ReadLine();
    	while (true) 
    	{
			var data = Console.ReadLine();
            var parsed = JObject.Parse(data);
			var command = onTick(parsed);
			Console.WriteLine(command.ToString());
    	}
    }

    public static JObject onTick(JObject parsed) 
    {
        var mine = (JArray) parsed.GetValue("Mine");
        var result = new JObject();
        if (mine.Count > 0) 
        {
            var objects = (JArray) parsed.GetValue("Objects");
            var food = findFood(objects);
            if (food != null) 
            {
                result["X"] = food.GetValue("X");
                result["Y"] = food.GetValue("Y");
            }
            else 
            {
                result["X"] = 0;
                result["Y"] = 0;
                result["Debug"] = "No food";
            }
        }
        else 
        {
            result["X"] = 0;
            result["Y"] = 0;
            result["Debug"] = "Died";
        }
        return result;
    }

    public static JObject findFood(JArray objects) 
    {
        foreach (JObject obj in objects) 
        {
            var type = (String) obj.GetValue("T");
            if (type.Equals("F")) 
            {
                return obj;
            }
        }
        return null;
    }
}