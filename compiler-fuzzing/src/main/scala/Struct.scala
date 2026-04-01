import scala.collection.mutable.ArrayBuffer
import scala.collection.mutable.HashMap

object Struct {
  def empty(name: String): Struct =
    Struct(name, ArrayBuffer.empty, HashMap.empty, HashMap.empty)
}

case class Struct(
    name: String,
    fields: ArrayBuffer[(String, Type)],
    methodDecls: HashMap[String, FnSignature],
    methodDefs: HashMap[String, FnDecl],
) {
  def ty: StructTy = StructTy(name)
  
  def getMemberType(member: String): Type =
    fields
      .find { (n, _) => n == member }
      .map { _._2 }
      .orElse {
        methodDecls
          .get(name)
          .map { (method: FnSignature) =>
            val FnTy(params, rettype) = method.ty
            FnTy(StructTy(name) +: params, rettype)
          }
      }
      .get
}
