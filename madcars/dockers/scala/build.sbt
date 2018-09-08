name := "Strategy"

version := "0.1.0"

scalaVersion := "2.12.6"

libraryDependencies += "com.rojoma" %% "rojoma-json-v3" % "3.8.0"
libraryDependencies += "ch.qos.logback" % "logback-classic" % "1.2.3"
libraryDependencies += "com.typesafe.scala-logging" %% "scala-logging" % "3.8.0"

scalacOptions in Test ++= Seq("-Yrangepos")
