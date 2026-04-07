import scala.collection.Seq

case class VarDecl(name: String, ty: Type, value: Expr):
  override def toString: String = s"var $name $ty = $value;"

case class FnSignature(name: String, params: Seq[(String, Type)], rettype: Type) {
  def ty: FnTy                          = FnTy(params map { _._2 }, rettype)
  def toMethod(of: Struct): FnSignature = copy(params = ("self", of.ty) +: params)
}

case class FnDecl(signature: FnSignature, body: CompoundStmt) {
  def ty: FnTy = signature.ty
}
