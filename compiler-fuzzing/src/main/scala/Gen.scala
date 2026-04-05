import scala.annotation.tailrec

import scala.collection.immutable
import scala.collection.mutable.HashMap
import scala.collection.IndexedSeq
import scala.collection.Seq

import scala.util.Random
import scala.collection.mutable.Stack
import scala.collection.mutable.ArrayBuffer

case class Range(minInclusive: Int, maxInclusive: Int) {
  def pick()(using rng: Random): Int = rng.between(minInclusive, maxInclusive + 1)
}

case class GenPolicy(
    identifierLength: Range,
    globalsCount: Range,
    structCount: Range,
    functionCount: Range,
    paramsPerFunction: Range,
    fieldsPerStruct: Range,
    methodsPerStruct: Range,
    statementBlockSize: Range,
    statementDepth: Range,
    expressionDepth: Range,
) {}

class Generator(policy: GenPolicy)(using rng: Random) {
  def genFile(): String = {
    genStructDecls()
    genStructFields()
    genFunctionDecls()
    genMethodDecls()

    genGlobals()

    genMethods()
    genFunctionDefs()

    var res = ""

    def fmtFn(decl: FnDecl): String =
      s"fun ${decl.signature.name}(${decl.signature.params
          .map { (name, ty) => if name == "self" then "self" else s"$name $ty" }
          .mkString(",")}) ${decl.signature.rettype} ${decl.body}\n"

    for struct <- structs.values do {
      res ++= s"struct ${struct.name} {\n"
      for field <- struct.fields do res ++= s"var ${field._1} ${field._2};\n"
      res += '\n'
      for decl <- struct.methodDefs.values do res ++= fmtFn(decl)
      res ++= "}\n"
    }

    for global <- globals do res ++= global._2.toString + '\n'

    functionDefs.values.foreach { res ++= fmtFn(_) }

    res
  }

  private val structs: HashMap[String, Struct]            = HashMap.empty
  private val functionDecls: HashMap[String, FnSignature] = HashMap.empty
  private val functionDefs: HashMap[String, FnDecl]       = HashMap.empty
  private val globals: HashMap[String, VarDecl]           = HashMap.empty

  private val varPool = VariablePool()

  private def isGlobalName(name: String): Boolean =
    structs.contains(name) || functionDecls.contains(name) || globals.contains(name)

  @tailrec private def randomUniqueIdentifier(seen: String => Boolean): String =
    val res = rng.randomIdentifier(policy)
    if seen(res) then randomUniqueIdentifier(seen) else res

  private def randomType: Type =
    rng.nextInt(structs.keySet.size + 3) match
      case 0 => VoidTy
      case 1 => IntTy
      case 2 => BoolTy
      case n => StructTy(structs.keys.drop(n - 3).head)

  private def randomValueType: Type =
    rng.nextInt(structs.keySet.size + 2) match
      case 0 => IntTy
      case 1 => BoolTy
      case n => StructTy(structs.keys.drop(n - 2).head)

  private def genStructDecl(): Unit = {
    val name: String = randomUniqueIdentifier(isGlobalName)
    structs.addOne(name -> Struct.empty(name))
  }

  private def genStructFields(struct: Struct): Unit = {
    val count = policy.fieldsPerStruct.pick()

    for _ <- 0 until count do
      struct.fields.addOne(
        randomUniqueIdentifier(struct.fields.map { _._1 }.contains)
          -> randomValueType
      )
  }

  private def randomSignature(name: String): FnSignature = {
    val paramCount                   = policy.fieldsPerStruct.pick()
    var params: List[(String, Type)] = Nil
    for _ <- 0 until paramCount do {
      params :+= (randomUniqueIdentifier(params.map { _._1 }.contains) -> randomValueType)
    }

    FnSignature(name, params, randomType)
  }

  private def randomTerminalExpr(ty: Type): Expr = {
    val ref = if rng.nextBoolean() then varPool.randomVarWithType(ty) else None

    ref getOrElse {
      ty match
        case BoolTy         => BoolLitExpr(rng.nextBoolean())
        case IntTy          => NumLitExpr(rng.nextInt().abs)
        case StructTy(name) => NullLitExpr
        case t =>
          throw IllegalArgumentException(s"`$t` is not a valid type for a terminal expression")
    }
  }

  private def randomRefExpr(ty: Type, depth: Int): Expr = {
    if rng.nextBoolean() then {
      val suitableStructs = structs.values.filter { _.fields.exists { _._2 == ty } }.toArray
      if suitableStructs.isEmpty then throw RuntimeException("couldn't find a suitable field")

      val struct = suitableStructs(rng.nextInt(suitableStructs.size))
      val fields = struct.fields.filter { _._2 == ty }.toArray
      val field  = fields(rng.nextInt(fields.size))._1

      val target = randomRefExpr(struct.ty, (depth - 1) min 1)
      MemberRefExpr(target, field)
    } else {
      varPool
        .randomVarWithType(ty)
        .getOrElse(
          throw RuntimeException("couldn't find a suitable variable")
        )
    }
  }

  private def randomCallableExpr(
      depth: Int,
      rettype: Type,
      params: Option[Seq[Type]] = None,
  ): (Expr, FnSignature) = {
    def signatureMatches(sig: FnSignature): Boolean =
      sig.rettype == rettype && (params match
        case None => true
        case Some(params) =>
          if sig.params.headOption.filter { _._1 == "self" }.isDefined then
            sig.params.tail == params
          else sig.params == params)

    def fail = throw RuntimeException("couldn't find a suitable callee")

    if rng.nextBoolean() then {
      val suitableStructs = structs.values.filter {
        _.methodDecls.values.exists(signatureMatches)
      }.toArray
      if suitableStructs.isEmpty then fail

      val struct  = suitableStructs(rng.nextInt(suitableStructs.size))
      val methods = struct.methodDecls.map { _._2 }.filter { _.rettype == rettype }.toArray
      val method  = methods(rng.nextInt(methods.size))
      MemberRefExpr(randomRefExpr(struct.ty, depth - 1), method.name) -> method
    } else {
      val suitableFunctions = functionDecls.values.filter(signatureMatches).toArray
      if suitableFunctions.isEmpty then fail

      val fn = suitableFunctions(rng.nextInt(suitableFunctions.size))
      VarRefExpr(fn.name) -> fn
    }
  }

  private def randomExpr(ty: Type, depth: Int): Expr = {
    var res: Option[Expr] = None

    if rng.nextBoolean() || depth <= 0 then res = Some(randomTerminalExpr(ty))

    if rng.nextBoolean() then {
      res = res orElse {
        try {
          val (callee, sig) = randomCallableExpr(depth - 1, ty)
          val args          = sig.params.map { _._2 }.map { ty => randomExpr(ty, depth - 1) }
          Some(CallExpr(callee, args))
        } catch case _ => None
      }
    }

    res getOrElse {
      ty match
        case BoolTy =>
          if rng.nextBoolean() then NotExpr(randomExpr(BoolTy, depth - 1))
          else BoolLitExpr(false) // TODO: logic, cmp, eq
        case FnTy(params, rettype) => randomCallableExpr(depth, rettype, Some(params))._1
        case IntTy                 => NumLitExpr(42) // TODO: arithmetic
        case ty: StructTy =>
          StructInitExpr(
            ty,
            structs(ty.name).fields.map { field => field._1 -> randomExpr(field._2, depth - 1) },
          )
        case VoidTy => throw IllegalArgumentException("`void` is not a valid type for expression")
    }
  }

  private def randomTerminalStatement(using rettype: Type): Stmt = {
    rng.nextInt(4) match
      case 0 =>
        val ty   = randomValueType
        val name = randomUniqueIdentifier(varPool.containsName)
        VarDeclStmt(VarDecl(name, ty, randomExpr(ty, policy.expressionDepth.pick())))
      case 1 =>
        val ty = randomValueType
        AssignmentStmt(
          randomRefExpr(ty, policy.expressionDepth.pick()),
          randomExpr(ty, policy.expressionDepth.pick()),
        )
      case 2 => ExprStmt(randomExpr(randomValueType, policy.expressionDepth.pick()))
      case 3 =>
        if rettype == VoidTy then VoidRetStmt
        else RetStmt(randomExpr(rettype, policy.expressionDepth.pick()))
  }

  private def randomCondStmt(depth: Int)(using rettype: Type): CondStmt = {
    val onFalse: Option[CondStmt | CompoundStmt] = Option.when(rng.nextBoolean) {
      if rng.nextBoolean then randomCompoundStmt(depth - 1)
      else randomCondStmt(depth - 1)
    }
    CondStmt(
      randomExpr(BoolTy, policy.expressionDepth.pick()),
      randomCompoundStmt(depth - 1),
      onFalse,
    )
  }

  private def randomStmt(depth: Int)(using rettype: Type): Stmt = {
    if depth <= 0 then randomTerminalStatement
    else {
      rng.nextInt(3) match
        case 0 => randomCompoundStmt(depth)
        case 1 =>
          WhileStmt(
            randomExpr(BoolTy, policy.expressionDepth.pick()),
            randomCompoundStmt(depth - 1),
          )
        case 2 => randomCondStmt(depth)
    }
  }

  private def randomCompoundStmt(depth: Int)(using rettype: Type): CompoundStmt = {
    varPool.enterScope()

    val count = policy.statementBlockSize.pick()
    val stmts = Array.fill(count) { randomStmt((depth - 1) max 0) }

    varPool.leaveScope()
    CompoundStmt(stmts)
  }

  private def randomFunction(signature: FnSignature): FnDecl = {
    varPool.enterScope()

    signature.params.foreach(varPool.addVar)

    val res = FnDecl(
      signature,
      randomCompoundStmt(policy.statementDepth.pick())(using signature.rettype),
    )
    varPool.leaveScope()
    res
  }

  private def genFunctionDecl(): Unit = {
    val name: String = randomUniqueIdentifier(isGlobalName)

    val signature = randomSignature(name)
    functionDecls.addOne(name -> signature)
  }

  private def genMethodDecls(struct: Struct): Unit = {
    val count = policy.methodsPerStruct.pick()

    for _ <- 0 until count do {
      val name = randomUniqueIdentifier(fn =>
        struct.methodDecls.contains(fn) || struct.fields.exists { _._1 == fn }
      )
      struct.methodDecls.addOne((name, randomSignature(name).toMethod(struct)))
    }
  }

  private def genStructDecls(): Unit =
    for _ <- 0 until policy.structCount.pick() do genStructDecl()
  private def genStructFields(): Unit =
    for struct <- structs.values do genStructFields(struct)
  private def genFunctionDecls(): Unit =
    for _ <- 0 until policy.functionCount.pick() do genFunctionDecl()
  private def genMethodDecls(): Unit =
    for struct <- structs.values do genMethodDecls(struct)

  private def genGlobals(): Unit = {
    val count = policy.globalsCount.pick()

    for _ <- 0 until count do {
      val name = randomUniqueIdentifier(isGlobalName)
      val ty   = randomValueType
      varPool.addVar(name, ty)

      val decl = VarDecl(name, ty, randomExpr(ty, policy.expressionDepth.pick()))
      globals.addOne(name, decl)
    }
  }

  private def genMethods(): Unit =
    for struct <- structs.values do genMethods(struct)
  private def genMethods(struct: Struct): Unit =
    for (name, sig) <- struct.methodDecls do struct.methodDefs += (name -> randomFunction(sig))

  private def genFunctionDefs(): Unit =
    for sig <- functionDecls.values do genFunctionDef(sig)
  private def genFunctionDef(signature: FnSignature): Unit =
    functionDefs += (signature.name -> randomFunction(signature))
}

private val ALPHABET_LENGTH: Int = 'z' - 'a' + 1
private val IDENTIFIER_CHARS     = '_' +: (('a' to 'z') ++ ('A' to 'Z') ++ ('0' to '9'))

extension [A](seq: IndexedSeq[A]) def pick()(using rng: Random) = seq(rng.nextInt(seq.length))

extension (rng: Random) {
  def nextIdentifierStart: Char = IDENTIFIER_CHARS(rng.nextInt(ALPHABET_LENGTH * 2 + 1))
  def nextIdentifierChar: Char  = IDENTIFIER_CHARS.pick()(using rng)

  def randomIdentifier(policy: GenPolicy): String =
    rng.nextIdentifier(policy.identifierLength.pick()(using rng))

  @tailrec def nextIdentifier(length: Int): String = {
    val builder = StringBuilder()
    for i <- 0 until length do
      if i == 0 then builder.addOne(rng.nextIdentifierStart)
      else builder.addOne(rng.nextIdentifierChar)
    val res = builder.result
    if res == "self" then nextIdentifier(length) else res
  }
}
