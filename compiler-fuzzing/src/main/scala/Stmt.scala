import scala.collection.Seq

sealed trait Stmt

case class AssignmentStmt(dest: Expr, src: Expr) extends Stmt {
  override def toString: String =
    s"$dest = $src;"
}

case class ExprStmt(expr: Expr) extends Stmt {
  override def toString: String = expr.toString + ';'
}

case class RetStmt(expr: Expr) extends Stmt {
  override def toString: String = s"return $expr;"
}

case object VoidRetStmt extends Stmt {
  override def toString: String = "return;"
}

case class VarDeclStmt(decl: VarDecl) extends Stmt {
  override def toString: String = decl.toString
}

case class CompoundStmt(stmts: Seq[Stmt]) extends Stmt {
  override def toString: String = "{" ++ stmts.mkString("\n", "\n", "\n") + "}"
}

case class CondStmt(
    cond: Expr,
    onTrue: CompoundStmt,
    onFalse: Option[CompoundStmt | CondStmt],
) extends Stmt {
  override def toString: String =
    val res = s"if ($cond) $onTrue"
    onFalse match
      case None          => res
      case Some(onFalse) => res ++ "\n" ++ onFalse.toString
}

case class WhileStmt(cond: Expr, body: CompoundStmt) extends Stmt {
  override def toString: String = s"while ($cond) $body"
}
