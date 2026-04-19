parser grammar VHDLParser;

options { tokenVocab=VHDLLexer; }

start : startRule EOF;

startRule: libs* units+;

libs : lib_declaration | use_declaration;

lib_declaration : LIBRARY ID SEMI;
use_declaration : USE (ID DOT)+(ID | ALL) SEMI;

units : entity_unit | arch_unit;

entity_unit
  : ENTITY ID IS port_obj? END (ID)? SEMI;

port_obj
  : PORT LPAREN port_list RPAREN SEMI;

port_list
  : in_out_signal+
  ;

in_out_signal
  : ID (COMMA ID)* COLON (IN | OUT) STD_LOGIC SEMI?
  ;

arch_unit
  : ARCHITECTURE ID OF ID IS signal_obj* BEGIN statement* END (ID)? SEMI;

signal_obj
  : SIGNAL ID (COMMA ID)* COLON STD_LOGIC SEMI;

statement
  : ID ASSIGN expression SEMI;

expression : orExpr;

orExpr  : andExpr (OR andExpr)*;
andExpr : notExpr (AND notExpr)*;
notExpr : NOT notExpr | primary;
primary : ID | LPAREN expression RPAREN;

