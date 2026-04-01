import scala.collection.mutable.HashMap

sealed trait Ctx extends PartialFunction[String, Type] {
  def lookup(name: String): Option[Type]

  override final def apply(x: String): Type          = lookup(x).get
  override final def lift: String => Option[Type]    = lookup
  override final def isDefinedAt(x: String): Boolean = lookup(x).isDefined

  def newScope: LocalCtx = LocalCtx(this, HashMap.empty)
  def prevScope: Ctx
}

object GlobalCtx {
  def empty: GlobalCtx = GlobalCtx(HashMap.empty, HashMap.empty, HashMap.empty, HashMap.empty)
}

case class GlobalCtx(
    structs: HashMap[String, Struct],
    functionDecls: HashMap[String, FnSignature],
    functionDefs: HashMap[String, FnDecl],
    globals: HashMap[String, VarDecl],
) extends Ctx {
  override def lookup(name: String): Option[Type] =
    functionDecls.get(name).map { _.ty } orElse
      globals.get(name).map { _.ty }
  override def prevScope: Ctx = this
}

case class LocalCtx(parent: Ctx, variables: HashMap[String, Type]) extends Ctx {
  override def lookup(name: String): Option[Type] = variables.get(name) orElse parent.lookup(name)
  override def prevScope: Ctx                     = parent
}
