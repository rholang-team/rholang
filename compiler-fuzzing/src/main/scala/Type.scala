import scala.collection.Seq

sealed trait Type:
  def isComparable: Boolean = isOrdered
  def isOrdered: Boolean    = false

case object VoidTy extends Type

case object BoolTy extends Type:
  override def isComparable: Boolean = true

case object IntTy extends Type:
  override def isOrdered: Boolean = true

case class FnTy(params: Seq[Type], rettype: Type) extends Type

case class StructTy(name: String) extends Type:
  override def isComparable: Boolean = true
