import scala.collection.Seq

sealed trait Stmt

case class AssignmentStmt(dest: Expr, src: Expr) extends Stmt:
  assert(dest.ty == src.ty)

case class ExprStmt(expr: Expr) extends Stmt

case class RetStmt(expr: Expr) extends Stmt

case object VoidRetStmt extends Stmt

case class CompoundStmt(stmts: Seq[Stmt]) extends Stmt

case class CondStmt(cond: Expr, onTrue: CompoundStmt, onFalse: Option[CompoundStmt | CondStmt])
    extends Stmt

case class WhileStmt(cond: Expr, body: CompoundStmt) extends Stmt
