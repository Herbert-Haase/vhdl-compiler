grammar VHDL;

design_file
    : entity_declaration* EOF
    ;

entity_declaration
    : ENTITY IDENTIFIER IS END ENTITY? IDENTIFIER? SEMI
    ;

ENTITY: 'entity';
IS: 'is';
END: 'end';
SEMI: ';';
IDENTIFIER: [a-zA-Z_] [a-zA-Z_0-9]*;
WS: [ \t\r\n]+ -> skip;
