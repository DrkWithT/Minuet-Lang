# README

### Brief
_Minuet_ is a small, JS & Rust inspired language for simple scripting. Unlike shell scripting dialects across systems, this language aims to look and behave nicer for the programmer.

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
   <spaces> = SP | TAB | CR
   <terminator> = LF

   <literal> = <boolean> | <integer> | <double> | <string>
   <primary> = <identifier> | "(" <compare> ")" | <lambda>
   <lambda> = "[" <identifier> ("," <identifier>)* "]" "=>" <block>
   <lhs> = <primary> ("." <primary>)*
   <call> = <lhs> ( "(" <compare> ("," <compare>)* ")" )*
   <unary> = "-"? <call>
   <factor> = <unary> (("*" | "/" | "%") <unary>)*
   <term> = <factor> (("+" | "-") <factor>)*
   <equality> = <term> (("==" | "!=") <term>)*
   <compare> = <equality> (("<" | ">" | ">=" | "<=") <equality>)*
   <assign> = <lhs> (":=" <compare>)?
```

### Grammar: Statements
```
<program> = (<comment> | <top>)* EOF
<top> = <identifier> ":" (<lambda> | (<compare> <terminator>))
<block> = "{" (<definition> | <match> | <expr-stmt>)+ "}"
<definition> = "def" <identifier> ":=" <compare> <terminator>
<expr-stmt> = <expr> <terminator>
<match> = "match" <identifier> "{" <pattern>* <default> "}"
<pattern> = "pat" <literal> "->" <compare> <terminator>
<default> = "_" "->" <compare> <terminator>
```
