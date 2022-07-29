grammar Remniw;

/*
 * '*' match 0 or more repetitions
 * '+' match 1 or more repetitions
 * '?' match 0 or 1 repetitions
 */

program
   : fun+
   ;

fun
   : 'func' id parameters scalarType '{' (varDeclarations)*  stmt* returnStmt '}'
   ;

parameters
   : '(' ((id paramType',')* id paramType)? ')'
   ;

varDeclarations
   : 'var' (id ',')* id varType ';'
   ;

returnStmt
   : 'return' expr ';'
   ;

arguments
   : '(' ((expr ',')* expr)? ')'
   ;

expr
   : expr arguments # FuncCallExpr
   | '%sizeof' varType # SizeofExpr
   | '&' id # RefExpr
   | '*' expr # DerefExpr
   | expr '[' expr ']' # ArraySubscriptExpr
   | '-' integer # NegIntExpr
   | integer # IntExpr
   | id # IdExpr
   | expr '*' expr # MulExpr
   | expr '/' expr # DivExpr
   | expr '+' expr # AddExpr
   | expr '-' expr # SubExpr
   | expr '>' expr # RelationalExpr
   | expr '==' expr # EqualExpr
   | '(' expr ')' # ParenExpr
   | '%input' # InputExpr
   | '%nil' # NullExpr
   | '{' ((id ':' expr ',')* id ':' expr)? '}' # RecordCreateExpr
   | expr '.' id # RecordAccessExpr
   ;


stmt
   : assignmentStmt
   | outputStmt
   | allocStmt
   | deallocStmt
   | emptyStmt
   | blockStmt
   | ifStmt
   | whileStmt
   ;

assignmentStmt
   : expr '=' expr ';'
   ;

outputStmt
   : '%output' expr ';'
   ;

allocStmt
   : '%alloc' '(' expr ',' expr ')' ';'
   ;

deallocStmt
   : '%dealloc' '(' expr ')' ';'
   ;

emptyStmt
   : ';'
   ;

blockStmt
   : '{' stmt* '}'
   ;

ifStmt
   : 'if' '(' expr ')' stmt ('else' stmt)?
   ;

whileStmt
   : 'while' '(' expr ')' stmt
   ;

varType
   : arrayType
   | scalarType
   ;

paramType
   : scalarType
   ;

scalarType
   : intType
   | pointerType
   | functionType
   ;

arrayType
   : '[' integer ']' varType
   ;

intType
   : 'int'
   ;

pointerType
   : '*' varType
   ;

functionType
   : 'func' parametersType scalarType
   ;

parametersType
   : '(' ((paramType',')* paramType)? ')'
   ;


id
   : IDENTIFIER
   ;

integer
   : NUMBER
   ;

IDENTIFIER
   : [a-zA-Z_][a-zA-Z0-9_]*
   ;

NUMBER
   : [0-9]+
   ;

WS
   : [ \r\n\t] -> skip
   ;

BLOCKCOMMENT
   : '/*' .*? '*/' -> skip
   ;

LINECOMMENT
   : '//' ~[\r\n]* -> skip
   ;
