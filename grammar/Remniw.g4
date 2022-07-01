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
   : 'func' id parameters type '{' (varDeclarations)*  stmt* returnStmt '}'
   ;

parameters
   : '(' ((id type',')* id type)? ')'
   ;

varDeclarations
   : 'var' (id ',')* id type ';'
   ;

returnStmt
   : 'return' expr ';'
   ;

arguments
   : '(' ((expr ',')* expr)? ')'
   ;

expr
   : expr arguments # FuncCallExpr
   | 'alloc' expr # AllocExpr
   | '&' id # RefExpr
   | '*' expr # DerefExpr
   | '-' integer # NegIntExpr
   | 'nil' # NullExpr
   | integer # IntExpr
   | id # IdExpr
   | expr '*' expr # MulExpr
   | expr '/' expr # DivExpr
   | expr '+' expr # AddExpr
   | expr '-' expr # SubExpr
   | expr '>' expr # RelationalExpr
   | expr '==' expr # EqualExpr
   | '(' expr ')' # ParenExpr
   | 'input' # InputExpr
   | '{' ((id ':' expr ',')* id ':' expr)? '}' # RecordCreateExpr
   | expr '.' id # RecordAccessExpr
   ;

stmt
   : assignmentStmt
   | outputStmt
   | emptyStmt
   | blockStmt
   | ifStmt
   | whileStmt
   ;

assignmentStmt
   : expr '=' expr ';'
   ;

basicAssignmentStmt
   : id '=' expr ';'
   ;

derefAssignmentStmt
   : '*' expr '=' expr ';'
   ;

outputStmt
   : 'output' expr ';'
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

recordFieldBasicAssignmentStmt
   : id '.' id '=' expr ';'
   ;

recordFieldDerefAssignmentStmt
   : '(' '*' expr ')' '.' id '=' expr ';'
   ;

type
   : intType
   | pointerType
   | functionType
   ;

intType
   : 'int'
   ;

pointerType
   : '*' type
   ;

functionType
   : 'func' parametersType type
   ;

parametersType
   : '(' ((type',')* type)? ')'
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
