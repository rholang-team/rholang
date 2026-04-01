import scala.collection.Seq

sealed trait Expr:
  def ty: Type

case class UnaryMinusExpr(value: Expr) extends Expr:
  assert(value.ty == IntTy)
  override def ty: Type = IntTy

case class NotExpr(value: Expr) extends Expr:
  assert(value.ty == BoolTy)
  override def ty: Type = BoolTy

sealed trait BinaryExpr extends Expr:
  assert(lhs.ty == rhs.ty)
  def lhs: Expr
  def rhs: Expr
  override def ty: Type = lhs.ty

sealed trait CmpExpr extends BinaryExpr:
  assert(lhs.ty.isComparable)
  assert(rhs.ty.isComparable)
  override def ty: Type = BoolTy

case class EqExpr(lhs: Expr, rhs: Expr) extends CmpExpr
case class NeExpr(lhs: Expr, rhs: Expr) extends CmpExpr

sealed trait OrdExpr extends CmpExpr:
  assert(lhs.ty.isOrdered)
  assert(rhs.ty.isOrdered)
  override def ty: Type = BoolTy

case class LtExpr(lhs: Expr, rhs: Expr) extends OrdExpr
case class GtExpr(lhs: Expr, rhs: Expr) extends OrdExpr
case class LeExpr(lhs: Expr, rhs: Expr) extends OrdExpr
case class GeExpr(lhs: Expr, rhs: Expr) extends OrdExpr

sealed trait ArithmeticExpr extends BinaryExpr:
  assert(lhs.ty == IntTy)
  assert(rhs.ty == IntTy)
  override def ty: Type = IntTy

case class AddExpr(lhs: Expr, rhs: Expr)  extends ArithmeticExpr
case class MinusExpr(lhs: Expr, rhs: Expr) extends ArithmeticExpr
case class MulExpr(lhs: Expr, rhs: Expr)   extends ArithmeticExpr

sealed trait LogicExpr extends BinaryExpr:
  assert(lhs.ty == BoolTy)
  assert(rhs.ty == BoolTy)
  override def ty: Type = BoolTy

case class OrExpr(lhs: Expr, rhs: Expr)  extends LogicExpr
case class AndExpr(lhs: Expr, rhs: Expr) extends LogicExpr

case class NumLitExpr(value: Int) extends Expr:
  override def ty: Type = IntTy

case class BoolLitExpr(value: Boolean) extends Expr:
  override def ty: Type = BoolTy

case class NullLitExpr(ty: Type)                                     extends Expr
case class VarRefExpr(ty: Type, name: String)                        extends Expr
case class MemberRefExpr(ty: Type, target: Expr, member: String)     extends Expr
case class CallExpr(ty: FnTy, callee: Expr, args: Seq[Expr])         extends Expr
case class StructInitExpr(ty: StructTy, fields: Seq[(String, Expr)]) extends Expr
