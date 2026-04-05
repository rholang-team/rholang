val scala3Version = "3.8.2"

lazy val root = project
  .in(file("."))
  .settings(
    name                                   := "sandbox",
    version                                := "0.1.0",
    scalaVersion                           := scala3Version,
    libraryDependencies += "org.scalameta" %% "munit" % "1.0.0" % Test,
  )
