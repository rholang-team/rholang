import scala.util.Random
import scala.sys.process.*

import scala.collection.mutable.ArrayBuffer

import java.util.concurrent.atomic.AtomicInteger
import java.util.concurrent.Executors
import java.util.concurrent.Future
import java.io.ByteArrayInputStream
import java.util.concurrent.Callable

val policy = GenPolicy(
  identifierLength = Range(3, 16),
  globalsCount = Range(0, 16),
  structCount = Range(0, 8),
  functionCount = Range(0, 10),
  paramsPerFunction = Range(0, 10),
  fieldsPerStruct = Range(0, 10),
  methodsPerStruct = Range(0, 10),
  statementBlockSize = Range(0, 20),
  statementDepth = Range(0, 7),
  expressionDepth = Range(0, 10),
)

val REPETITIONS = 10000

@main def main(cmd: String): Unit = {
  val executor = Executors.newFixedThreadPool(Runtime.getRuntime().availableProcessors())
  val futures = Array.fill[Future[Boolean]](REPETITIONS) {
    val task =
      new Callable[Boolean] {
        override def call(): Boolean = {
          var input: Option[String] = None
          while input.isEmpty do {
            input =
              try Some(Generator(policy)(using Random()).genFile())
              catch case e: Exception => None
          }

          (s"$cmd -" #< ByteArrayInputStream(input.get.getBytes)).! == 0
        }
      }

    executor.submit(task)
  }

  val successes = futures.count { _.get() }

  println(
    s"$successes/$REPETITIONS (${successes.toDouble / REPETITIONS.toDouble * 100.0}%) runs were successful"
  )
}
