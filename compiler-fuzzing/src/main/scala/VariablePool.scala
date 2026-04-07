import scala.util.Random
import scala.collection.mutable.HashMap
import scala.collection.mutable.Stack
import scala.collection.mutable.ArrayBuffer

class VariablePool {
  private val scopes: Stack[HashMap[Type, ArrayBuffer[String]]] = Stack(HashMap.empty)

  def enterScope(): Unit = scopes push HashMap.empty
  def leaveScope(): Unit = scopes.pop()

  def addVar(name: String, ty: Type): Unit = {
    val scope = scopes.top
    if !scope.contains(ty) then scope.addOne(ty, ArrayBuffer.empty)
    scope(ty).addOne(name)
  }

  def contains(ty: Type): Boolean         = scopes.exists { _.contains(ty) }
  def containsName(name: String): Boolean = scopes.exists { _.values.exists { _.contains(name) } }

  def randomVarWithType(ty: Type)(using rng: Random): Option[VarRefExpr] = {
    val totalSize = scopes.map { _.get(ty).map { _.size }.getOrElse(0) }.sum
    Option.when(totalSize != 0) {
      val idx = rng.nextInt(totalSize)
      VarRefExpr(
        scopes.iterator
          .flatMap { _.getOrElse(ty, ArrayBuffer.empty).iterator }
          .drop(idx)
          .next
      )
    }
  }
}
