using System;

public class Program
{
	public static void Main()
	{
		var commands = new string[4]{ "left", "right", "up", "down" };
		Random random = new Random();
		while(true) {
			var input = Console.ReadLine();
			int index = random.Next(0, commands.Length);
			Console.WriteLine("{{\"command\": \"{0}\"}}", commands[index]);
		}
	}
}