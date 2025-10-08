# README

### Brief
_Minuet_ is a small, Python & C inspired language for simple scripting. Unlike shell scripting dialects across systems, this language aims to look and behave nicer for the programmer.

### Design Principles
 - Concise and semi-familiar syntax
 - Dynamic, _strong_, null-safe typing
 - Imperative & functional
    - First class functions
    - Higher order functions & some closures?
 - Built-in helper functions

### Grammar: Misc. & Expressions
```
<comment> = "#" <NON-HASH-SIGN>* "#"
<spaces> = SP | TAB | CR | LF

<literal> = <boolean> | <integer> | <double> | <string>
<primary> = <identifier> | <lambda> | "(" <compare> ")" | <literal>
<lambda> = "[" <identifier> ("," <identifier>)* "]" "=>" <block>
<lhs> = <primary> ("." <call>)*
<call> = <lhs> ( "(" <compare> ("," <compare>)* ")" )?
<unary> = "-"? <call>
<factor> = <unary> (("*" | "/" | "%") <unary>)*
<term> = <factor> (("+" | "-") <factor>)*
<equality> = <term> (("==" | "!=") <term>)*
<compare> = <equality> (("<" | ">" | ">=" | "<=") <equality>)*
<assign> = <lhs> (":=" <compare>)?
```

### Grammar: Statements
```
<import> = "import" <string>
<program> = (<import> | <function>)* EOF
<function> = "fun" <identifier> ":" "[" <identifier> ("," <identifier>)* "]" "=>" <block>
<block> = "{" (<definition> | <if> | <expr-stmt>)+ "}"
<definition> = "def" <identifier> ":=" <compare> <terminator>
<if> = "if" <compare> <block> ("else" <block>)?
<return> = "return" <compare>
<expr-stmt> = <expr> <terminator>

; todo
; <match> = "match" <identifier> "{" <pattern>* <default> "}"
; <pattern> = "pat" <literal> "->" <compare> <terminator>
; <default> = "_" "->" <compare> <terminator>
```

### Roadmap
 - Optimize VM to go under 2s for a naively recursive fibonacci 35.
