import scala.collection.Seq
import scala.math.Ordering.BigIntOrdering

sealed trait Expr

case class UnaryMinusExpr(value: Expr) extends Expr {
  override def toString: String = s"-($value)"
}

case class NotExpr(value: Expr) extends Expr {
  override def toString: String = s"!($value)"
}

sealed trait BinaryExpr(op: String) extends Expr {
  def lhs: Expr
  def rhs: Expr
  override def toString: String = s"($lhs) $op ($rhs)"
}

case class EqExpr(lhs: Expr, rhs: Expr) extends BinaryExpr("==")
case class NeExpr(lhs: Expr, rhs: Expr) extends BinaryExpr("!=")

case class LtExpr(lhs: Expr, rhs: Expr) extends BinaryExpr("<")
case class GtExpr(lhs: Expr, rhs: Expr) extends BinaryExpr(">")
case class LeExpr(lhs: Expr, rhs: Expr) extends BinaryExpr("<=")
case class GeExpr(lhs: Expr, rhs: Expr) extends BinaryExpr(">=")

case class AddExpr(lhs: Expr, rhs: Expr) extends BinaryExpr("+")
case class SubExpr(lhs: Expr, rhs: Expr) extends BinaryExpr("-")
case class MulExpr(lhs: Expr, rhs: Expr) extends BinaryExpr("*")
case class DivExpr(lhs: Expr, rhs: Expr) extends BinaryExpr("/")

case class OrExpr(lhs: Expr, rhs: Expr)  extends BinaryExpr("||")
case class AndExpr(lhs: Expr, rhs: Expr) extends BinaryExpr("&&")

case class NumLitExpr(value: Int) extends Expr {
  override def toString: String = value.toString
}
case class BoolLitExpr(value: Boolean) extends Expr {
  override def toString: String = value.toString
}

case object NullLitExpr extends Expr {
  override def toString: String = "null"
}
case class VarRefExpr(name: String) extends Expr {
  override def toString: String = name
}
case class MemberRefExpr(target: Expr, member: String) extends Expr {
  override def toString: String = s"($target).$member"
}
case class CallExpr(callee: Expr, args: Seq[Expr]) extends Expr {
  override def toString: String = s"($callee)" ++ args.mkString("(", ", ", ")")
}
case class StructInitExpr(ty: StructTy, fields: Seq[(String, Expr)]) extends Expr {
  override def toString: String =
    s"${ty.name}" ++ fields.map { (name, value) => s".$name = $value" }.mkString("{", ",\n", "}")
}
