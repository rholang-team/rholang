import scala.util.Random

import scala.util.boundary
import scala.util.boundary.break

import scala.annotation.tailrec

import scala.collection.mutable.ArrayBuffer
import scala.collection.mutable.Buffer
import scala.collection.IndexedSeq
import scala.collection.Seq
import scala.collection.Map

import java.io.OutputStream
import scala.collection.mutable.HashMap

private val ALPHABET_LENGTH: Int = 'z' - 'a' + 1
private val IDENTIFIER_CHARS     = '_' +: (('a' to 'z') ++ ('A' to 'Z') ++ ('0' to '9'))

extension [A](seq: IndexedSeq[A]) def pick(using rng: Random) = seq(rng.nextInt(seq.length))

extension (rng: Random) {
  def nextIdentifierStart: Char = IDENTIFIER_CHARS(rng.nextInt(ALPHABET_LENGTH * 2 + 1))
  def nextIdentifierChar: Char  = IDENTIFIER_CHARS.pick(using rng)

  @tailrec def nextIdentifier(length: Int): String = {
    val builder = StringBuilder()
    for i <- 0 until length do
      if i == 0 then builder.addOne(rng.nextIdentifierStart)
      else builder.addOne(rng.nextIdentifierChar)
    val res = builder.result
    if res == "self" then nextIdentifier(length) else res
  }
}

case class Range(minInclusive: Int, maxInclusive: Int) {
  def pick(using rng: Random): Int = rng.between(minInclusive, maxInclusive + 1)
}

case class GenPolicy(
    identifierLength: Range,
    structCount: Range,
    functionCount: Range,
    paramsPerFunction: Range,
    fieldsPerStruct: Range,
    methodsPerStruct: Range,
) {
  def randomIdentifier(using rng: Random): String = rng.nextIdentifier(identifierLength.pick)
}

class Generator(policy: GenPolicy)(using rng: Random) {
  private val globalCtx: GlobalCtx = GlobalCtx.empty
  private var curCtx: Ctx          = globalCtx

  @tailrec private def randomUniqueIdentifier(seen: String => Boolean): String =
    val res = policy.randomIdentifier
    if seen(res) then randomUniqueIdentifier(seen) else res

  private def randomType: Type =
    rng.nextInt(globalCtx.structs.keySet.size + 3) match
      case 0 => VoidTy
      case 1 => IntTy
      case 2 => BoolTy
      case n => StructTy(globalCtx.structs.keys.drop(n - 3).head)

  private def randomValueType: Type =
    rng.nextInt(globalCtx.structs.keySet.size + 2) match
      case 0 => IntTy
      case 1 => BoolTy
      case n => StructTy(globalCtx.structs.keys.drop(n - 2).head)

  private def genStructDecl(): Unit = {
    val name: String = randomUniqueIdentifier(globalCtx.isDefinedAt)
    globalCtx.structs.addOne(name -> Struct.empty(name))
  }

  private def genStructFields(struct: Struct): Unit = {
    val count = policy.fieldsPerStruct.pick

    for _ <- 0 until count do
      struct.fields.addOne(
        randomUniqueIdentifier(struct.fields.map { _._1 }.contains)
          -> randomValueType
      )
  }

  private def randomSignature(name: String): FnSignature = {
    val paramCount                   = policy.fieldsPerStruct.pick
    var params: List[(String, Type)] = Nil
    for _ <- 0 until paramCount do {
      params :+= (randomUniqueIdentifier(params.map { _._1 }.contains) -> randomValueType)
    }

    FnSignature(name, params, randomType)
  }

  private def genFunctionDecl(): Unit = {
    val name: String = randomUniqueIdentifier(globalCtx.isDefinedAt)

    val signature = randomSignature(name)
    globalCtx.functionDecls.addOne(name -> signature)
  }

  private def genMethodDecls(struct: Struct): Unit = {
    val count = policy.methodsPerStruct.pick

    for _ <- 0 until count do {
      val name = randomUniqueIdentifier(fn =>
        struct.methodDecls.contains(fn) || struct.fields.exists { _._1 == fn }
      )
      struct.methodDecls.addOne((name, randomSignature(name).toMethod(struct)))
    }
  }

  private def genMethods(struct: Struct): Unit = {
    ???
  }

  private def genFunction(signature: FnSignature): Unit = {
    ???
  }

  private def genStructDecls(): Unit =
    for _ <- 0 until policy.structCount.pick do genStructDecl()
  private def genStructFields(): Unit =
    for struct <- globalCtx.structs.values do genStructFields(struct)
  private def genFunctionDecls(): Unit =
    for _ <- 0 until policy.functionCount.pick do genFunctionDecl()
    for struct <- globalCtx.structs.values do genMethodDecls(struct)

  private def genMethods(): Unit =
    for struct <- globalCtx.structs.values do genMethods(struct)
  private def genFunctionDefs(): Unit =
    for sig <- globalCtx.functionDecls.values do genFunction(sig)

  def genFile(): String = {
    genStructDecls()
    genStructFields()
    genFunctionDecls()

    genMethods()
    genFunctionDefs()

    val builder = StringBuilder()
    for struct <- globalCtx.structs.values do {
      builder addAll s"struct ${struct.name} {{"

      for field <- struct.fields do builder ++= s"var ${field._1} ${field._2};"

      for (name, decl) <- struct.methodDefs do {
        // TODO

        // builder addAll
        //   (s"fun $name") addAll
        //   decl.signature.params.mkString("(", ",", ")") addStmt
        //   decl.body
      }

      ???
    }

    builder.result
  }
}

extension (b: StringBuilder) {
  infix def addStmt(s: Stmt): StringBuilder = {
    s match
      case AssignmentStmt(dest, src) => b addExpr dest addAll " = " addExpr src addAll ";\n"
      case ExprStmt(e)               => b addExpr e addAll ";\n"
      case RetStmt(v)                => b addAll "return " addExpr v addAll ";\n"
      case VoidRetStmt               => b addAll "return;\n"
      case CompoundStmt(stmts) =>
        b addAll "{\n"
        for s <- stmts do b addStmt s
        b addAll "}\n"
      case CondStmt(cond, onTrue, None) =>
        b addAll "if (" addExpr cond addAll ")\n" addStmt onTrue
      case CondStmt(cond, onTrue, Some(onFalse)) =>
        b addAll "if (" addExpr cond addAll ")\n" addStmt onTrue addAll "else " addStmt onFalse
      case WhileStmt(cond, body) =>
        b addAll "while (" addExpr cond addAll ")\n" addStmt body
  }

  infix def addExpr(e: Expr): StringBuilder = {
    e match
      case UnaryMinusExpr(e)   => b addAll "-(" addExpr e addOne ')'
      case NotExpr(e)          => b addAll "!(" addExpr e addOne ')'
      case EqExpr(lhs, rhs)    => b addOne '(' addExpr lhs addAll ") == (" addExpr rhs addOne ')'
      case NeExpr(lhs, rhs)    => b addOne '(' addExpr lhs addAll ") != (" addExpr rhs addOne ')'
      case LtExpr(lhs, rhs)    => b addOne '(' addExpr lhs addAll ") < (" addExpr rhs addOne ')'
      case GtExpr(lhs, rhs)    => b addOne '(' addExpr lhs addAll ") > (" addExpr rhs addOne ')'
      case LeExpr(lhs, rhs)    => b addOne '(' addExpr lhs addAll ") <= (" addExpr rhs addOne ')'
      case GeExpr(lhs, rhs)    => b addOne '(' addExpr lhs addAll ") >= (" addExpr rhs addOne ')'
      case AddExpr(lhs, rhs)   => b addOne '(' addExpr lhs addAll ") + (" addExpr rhs addOne ')'
      case MinusExpr(lhs, rhs) => b addOne '(' addExpr lhs addAll ") - (" addExpr rhs addOne ')'
      case MulExpr(lhs, rhs)   => b addOne '(' addExpr lhs addAll ") * (" addExpr rhs addOne ')'
      case AndExpr(lhs, rhs)   => b addOne '(' addExpr lhs addAll ") && (" addExpr rhs addOne ')'
      case OrExpr(lhs, rhs)    => b addOne '(' addExpr lhs addAll ") || (" addExpr rhs addOne ')'
      case NumLitExpr(n)       => b addAll n.toString
      case BoolLitExpr(v)      => b addAll v.toString
      case NullLitExpr(_)      => b addAll "null"
      case VarRefExpr(_, v)    => b addAll v
      case MemberRefExpr(_, target, member) => b addOne '(' addExpr target addAll ")." addAll member
      case CallExpr(_, callee, args) =>
        b addOne '(' addExpr callee addAll ")("
        for arg <- args do b addExpr arg addAll ", "
        b addOne ')'
      case StructInitExpr(s, fields) =>
        b addAll s.name addOne '{'
        for (name, value) <- fields.init do
          b addOne '.' addAll name addAll " = " addExpr value addAll ", "
        b addOne '}'
  }
}

@main def main: Unit = {}
