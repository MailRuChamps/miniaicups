import org.json.JSONArray
import org.json.JSONObject
import scala.collection.JavaConverters._
import scala.io.StdIn

object Main {
	def main(args: Array[String]): Unit = {
		while (true) {
			val tickData = new JSONObject(StdIn.readLine())
			val move = onTick(tickData)
			println(move.get)
		}
	}

	def onTick(tickData: JSONObject) = {
		val mine = tickData.getJSONArray("Mine")
		val objects = tickData.getJSONArray("Objects")
		if (mine.length() != 0) {
			findFood(objects).map(v ⇒ new JSONObject(Map(("X", v.get("X")), ("Y", v.get("Y"))).asJava))
				.orElse(Some(new JSONObject(Map(("X", 0), ("Y", 0), ("Debug", "No food")).asJava)))
		}
		else Some(new JSONObject(Map(("X", 0), ("Y", 0), ("Debug", "Died")).asJava))
	}

	def findFood(objects: JSONArray) =
		objects
			.asScala
			.filter(_.isInstanceOf[JSONObject])
			.map(_.asInstanceOf[JSONObject])
			.collectFirst { case jsObj if jsObj.get("T") == "F" ⇒ jsObj }
}