import scala.util.Random

object Strategy {
    def main(args: Array[String]): Unit = {
        while(true){
            val input = scala.io.StdIn.readLine()
            val commands = Array("left", "right", "up", "down")
            val command = Random.shuffle(commands.toList).head
            println(String.format("{\"command\": \"%s\"}", command))
        }
    }
}