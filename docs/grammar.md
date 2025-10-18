### Minuet Syntax Rules

### Misc. & Expressions
```
<comment> = "#" <NON-HASH-SIGN>* "#"
<spaces> = SP | TAB | CR | LF

<literal> = <boolean> | <integer> | <double> | <string> | <tuple> | <list>
<tuple> = "[" ( <primary> ("," <primary>)* )? "]"
<list> = "{" ( <primary> ("," <primary>)* )? "}"
<primary> = <identifier> | <lambda> | "(" <compare> ")" | <literal>
<lambda> = "fn" "[" <identifier> ("," <identifier>)* "]" "=>" <block>
<lhs> = <primary> ("." <call>)*
<call> = <lhs> ( "(" ( "(" <compare> ("," <compare>)* ")" )? ")" )?
<unary> = "-"? <call>
<factor> = <unary> (("*" | "/" | "%") <unary>)*
<term> = <factor> (("+" | "-") <factor>)*
<equality> = <term> (("==" | "!=") <term>)*
<compare> = <equality> (("<" | ">" | ">=" | "<=") <equality>)*
<assign> = <unary> ("=" <compare>)?
```

### Statements
```
<import> = "import" <string>
<program> = (<import> | <function> | <native>)* EOF
<function> = "fun" <identifier> ":" "[" <identifier> ("," <identifier>)* "]" "=>" <block>
<native> = "native" "fun" <identifier> ":" "[" <identifier> ("," <identifier>)* "]"
<block> = "{" (<definition> | <if> | <return> | <while> | <for-count-loop> | <expr-stmt>)+ "}"
<definition> = "def" <identifier> "=" <compare> <terminator>
<if> = "if" <compare> <block> ("else" <block>)?
<return> = "return" <compare>
<while> = "while" <compare> <block>
<break> = "break"
<expr-stmt> = <expr> <terminator>
```

### Unused
```
; stmts
<match> = "match" <identifier> "{" <pattern>* <default> "}"
<pattern> = "pat" <literal> "->" <compare> <terminator>
<default> = "_" "->" <compare> <terminator>
```
