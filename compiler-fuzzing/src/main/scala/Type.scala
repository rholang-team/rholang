import scala.collection.Seq

sealed trait Type

case object VoidTy extends Type:
  override def toString: String = "void"

case object BoolTy extends Type:
  override def toString: String = "bool"

case object IntTy extends Type:
  override def toString: String = "int"

case class FnTy(params: Seq[Type], rettype: Type) extends Type

case class StructTy(name: String) extends Type:
  override def toString: String = name
