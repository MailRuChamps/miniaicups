import org.json.JSONArray
import org.json.JSONObject

fun main(args: Array<String>) {
    val config = JSONObject(readLine())

    while (true) {
        val tickData = JSONObject(readLine())
        val move = onTick(tickData)
        println(move)
    }
}

fun onTick(tickData: JSONObject): JSONObject {
    val mine = tickData.getJSONArray("Mine")!!
    val objects = tickData.getJSONArray("Objects")!!
    if (mine.length() != 0) {
        val first = mine[0]
        val food = findFood(objects) ?: return JSONObject(mapOf("X" to 0, "Y" to 0, "Debug" to "No food"))

        return JSONObject(mapOf<String, Any>("X" to food["X"], "Y" to food["Y"]))
    }
    return JSONObject(mapOf("X" to 0, "Y" to 0, "Debug" to "Died"))
}

fun findFood(objects: JSONArray): JSONObject? =
        objects.map { it as JSONObject }.firstOrNull { it["T"] == "F" }
