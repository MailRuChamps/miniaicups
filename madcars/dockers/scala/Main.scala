
import com.rojoma.json.v3.util.JsonUtil
import com.rojoma.json.v3.ast._


object Main {

	def onTick(parsed : Map[String, Array[Map[String, JValue]]]) : Map[String, JValue] = {

		if (parsed.get("Mine").size > 0) {

			val foods = findFood(parsed.get("Objects").get)

			if (foods.size > 0) {
				Map("X" -> foods.head.get("X").get.cast[JNumber].get, "Y" -> foods.head.get("Y").get.cast[JNumber].get)
			} else {
				Map("X" -> JNumber(300), "Y" -> JNumber(300), "Debug" -> JString("No food"))
			}

		} else {
			Map("X" -> JNumber(0), "Y" -> JNumber(0), "Debug" -> JString("Died"))
		}
	}

	def findFood(objects : Array[Map[String, JValue]]) : Array[Map[String, JValue]] = {
		objects.filter(t => t.get("T").get == JString("F"))
	}

	def main(args : Array[String]) {
		val configStr = scala.io.StdIn.readLine().trim
		val config = JsonUtil.parseJson[Map[String, JValue]](configStr)

		for(tick <- 0 to config.right.get("GAME_TICKS").cast[JNumber].get.toInt) {
			val json = scala.io.StdIn.readLine().trim
			val data = JsonUtil.parseJson[Map[String, Array[Map[String, JValue]]]](json)

			val reponse = onTick(data.right.get)
			println(JsonUtil.renderJson(reponse))
		}
	}
}
