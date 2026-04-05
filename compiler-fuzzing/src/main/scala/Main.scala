import scala.util.Random
import scala.sys.process.*

import java.nio.file.Files
import java.nio.file.Paths
import java.nio.charset.StandardCharsets

val policy = GenPolicy(
  identifierLength = Range(1, 16),
  globalsCount = Range(0, 0),
  structCount = Range(0, 7),
  functionCount = Range(0, 16),
  paramsPerFunction = Range(0, 10),
  fieldsPerStruct = Range(0, 10),
  methodsPerStruct = Range(0, 10),
  statementBlockSize = Range(0, 30),
  statementDepth = Range(0, 4),
  expressionDepth = Range(0, 5),
)

@main def main(cmd: String, tmpFile: String, repetitions: Int): Unit = {
  var failures = 0

  for i <- 1 to repetitions do {
    var res: Option[String] = None
    while res.isEmpty do {
      res =
        try Some(Generator(policy)(using Random()).genFile())
        catch case e: Exception => None
    }

    Files.write(Paths.get(tmpFile), res.get.getBytes(StandardCharsets.US_ASCII))
    assert(res.get == Files.readString(Paths.get(tmpFile)))

    try s"$cmd $tmpFile".!!
    catch
      case e: RuntimeException => {
        println(s"run $i failed")
        failures += 1
      }
  }

  val success = repetitions - failures
  println(
    s"$success/$repetitions (${success.toDouble / repetitions.toDouble * 100.0}%) runs were successful"
  )
}
