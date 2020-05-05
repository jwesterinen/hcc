/*
 *	hcc - a hack C compiler grammar
 *	syntax analysis with error recovery
 * 	(s/r conflict: one on ELSE, one on error)
 */

%{
#include "error.h"
#include "symtab.h"
#include "gen.h"
int yylex(void);

#define OFFSET(x)   (((struct Symtab *)x)->s_offset)
#define NAME(x)     (((struct Symtab *)x)->s_name)
%}

/*
 * parser stack type union
 */
 
%union  {
            struct Symtab *y_sym;   // Identifier
            char *y_str;            // Constant, type  
            int y_num;              // count
            int y_lab;              // label
        }

/*
 *	terminal symbols
 */

%token  <y_sym> Identifier
%token  <y_str> Constant
%token  INT
%token  VOID
%token  IF
%token  ELSE
%token  WHILE
%token  BREAK
%token  CONTINUE
%token  RETURN
%token  ';'
%token  '('
%token  ')'
%token  '{'
%token  '}'
%token  '+'
%token  '-'
%token  '*'
%token  '/'
%token  '%'
%token  '>'
%token  '<'
%token  GE
%token  LE
%token  EQ
%token  NE
%token  '&'
%token  '^'
%token  '|'
%token  '='
%token  PE
%token  ME
%token  TE
%token  DE
%token  RE
%token  PP
%token  MM
%token  ','

/*
 *	typed non-terminal symbols
 */

%type   <y_sym> function_definition function_declaration optional_parameter_list parameter_list
%type   <y_num> parameter_type optional_argument_list argument_list
%type   <y_lab> if_prefix loop_prefix

/*
 *	precedence table
 */

%right	'=' PE ME TE DE RE
%left	'|'
%left	'^'
%left	'&'
%left	EQ NE
%left	'<' '>' GE LE
%left	'+' '-'
%left	'*' '/' '%'
%right	PP MM

%%

program
	:   
	    {
	        init();
	        gen_begin_prog();
	    }
	  definitions
	    {
	        end_program();
	    }

definitions
	: definition
	| definitions definition
	    {
	        yyerrok;
	    }
	| error
	| definitions error

definition
	: function_definition
	| declaration

function_definition
	: function_declaration
	    {
	        l_max = 0;
	        $<y_lab>$ = gen_entry($1);
	    }
	  compound_statement
	    {
	        all_func($1);
	        gen_pr(OP_RETURN, "end of function");
	        fix_entry($1, $<y_lab>2);
	    }

function_declaration
	: INT Identifier '(' 
	    {
	        make_func(1, $2);
	        blk_push();
	    }
	  optional_parameter_list rp/*)*/
	    {
	        chk_parm($2, parm_default($5));
	        all_parm($5);
	        $$ = $2;
	    } 
	| VOID Identifier '(' 
	    {
	        make_func(0, $2);
	        blk_push();
	    }
	  optional_parameter_list rp/*)*/
	    {
	        chk_parm($2, parm_default($5));
	        all_parm($5);
	        $$ = $2;
	    } 

optional_parameter_list
	: /* no formal parameters */
	    {
	        $$ = 0;
	    }
	| parameter_list
	    /* $$ = $1 = chain of formal parameters */

parameter_list
    : parameter_type Identifier
        {
            $$ = link_parm($1, $2, (struct Symtab *)0);
        }
    | parameter_type Identifier ',' parameter_list
        {
            $$ = link_parm($1, $2, $4);
            yyerrok;
        }
    | error
        {
            $$ = 0;
        }
    | error parameter_list
        {
            $$ = $2;
        }
    | parameter_type Identifier error parameter_list
        {
            $$ = link_parm($1, $2, $4);
            yyerrok;
        }
    | error ',' parameter_list
        {
            $$ = $3;
            yyerrok;
        }
        
parameter_type
    : /* none */
        {$$ = 0;}
    | INT  
        {$$ = 0;}
    | INT '*'    
        {$$ = 1;}  
    
compound_statement
	: '{' 
	    {
	        $<y_lab>$ = l_offset;
	        blk_push();
	    }
	  declarations statements rr/*'}'*/
	    {
	        if (l_offset > l_max)
	            l_max = l_offset;
	        l_offset = $<y_lab>2;
	        blk_pop();
	    }

declarations
	: /* null */
	| declarations declaration
	    {
	        yyerrok;
	    }
	| declarations error

declaration
	: INT declarator_list sc/*';'*/
	| function_declaration sc/*';'*/
	    {
	        blk_pop();
	    }
	
declarator_list
	: declarator
	| declarator_list ',' declarator
	    {
	        yyerrok;
	    }
	| error
	| declarator_list error
	| declarator_list error declarator
	    {
	        yyerrok;
	    }
	| declarator_list ',' error

declarator
    : Identifier
        {
            all_var($1);
        }
    | Identifier '=' initializer
        {
            all_var($1);
	        gen_direct(OP_STORE, gen_mod($1), OFFSET($1), NAME($1));
	        gen_pr(OP_POP, "clear stack");
        }

initializer
    : Constant
	    {
	        gen_load_immed($1);
	    }
    
statements
	: /* null */
	| statements statement
	    {
	        yyerrok;
	    }
	| statements error

statement
	: expression sc/*';'*/
	    {
	        if (!is_void)
	            gen_pr(OP_POP, "clear stack");
	        else
	            is_void = 0;
	    }
	| sc/*';'*/  /* null statement */
	| BREAK sc/*';'*/
	    {gen_break();}
	| CONTINUE sc/*';'*/
	    {gen_continue();}
	| RETURN sc/*';'*/
	    {gen_pr(OP_RETURN, "RETURN");}
	| RETURN expression sc/*';'*/
	    {
	        gen_direct(OP_STORE, MOD_GLOBAL, 0, "save result");
            gen_pr(OP_RETURN, "RETURN");	        
	    }
	| compound_statement
	| if_prefix statement
	    {gen_label($1);}
	| if_prefix statement ELSE 
	    {
	        $<y_lab>$ = gen_jump(OP_JUMP, new_label(), "past ELSE");
	        gen_label($1);
	    }
	  statement
	    {gen_label($<y_lab>4);}
	| loop_prefix 
	    {
	        $<y_lab>$ = gen_jump(OP_JUMPZ, new_label(), "WHILE");
	        push_break($<y_lab>$);
	    }
	  statement
	    {
	        gen_jump(OP_JUMP, $1, "repeat WHILE");
	        gen_label($<y_lab>2);
	        pop_break();
	        pop_continue();
	    }

if_prefix
	: IF '(' expression rp/*')'*/
	    {$$ = gen_jump(OP_JUMPZ, new_label(), "IF");}
	| IF error
	    {$$ = gen_jump(OP_JUMPZ, new_label(), "IF");}

loop_prefix
	: WHILE '(' 
	    {
	        $<y_lab>$ = gen_label(new_label());
	        push_continue($<y_lab>$);
	    }
	  expression rp/*')'*/
	    {$$ = $<y_lab>3;}
	| WHILE error
	    {
	        $$ = gen_label(new_label());
	        push_continue($$);
	    }

expression
	: binary
	| expression ','
	    {
	        yyerrok;
	        gen_pr(OP_POP, "discard");
	    }
	  binary
	| error ',' binary
	    {
	        yyerrok;
	    }
	| expression error
	| expression ',' error

binary
	: Identifier
	    { 
	        chk_var($1);
	        gen_direct(OP_LOAD, gen_mod($1), OFFSET($1), NAME($1));
	    }
	| Constant
	    {gen_load_immed($1);}
	| '(' expression rp/*')'*/
	| '(' error rp/*')'*/
	| Identifier '(' 
	    {chk_func($1);}
	  optional_argument_list rp/*')'*/
	    {gen_call($1, $4);}
	| PP Identifier
	    {
	        chk_var($2);
	        gen_direct(OP_INC, gen_mod($2), OFFSET($2), NAME($2));
	    }
	| MM Identifier
	    {
	        chk_var($2);
	        gen_direct(OP_DEC, gen_mod($2), OFFSET($2), NAME($2));
	    }
	| binary '+' binary
	    {gen_alu(ALU_ADD, "+");}
	| binary '-' binary
	    {gen_alu(ALU_SUB, "-");}
	| binary '*' binary
	    {gen_alu(ALU_MUL, "*");}
	| binary '/' binary
	    {gen_alu(ALU_DIV, "/");}
	| binary '%' binary
	    {gen_alu(ALU_MOD, "%");}
	| binary '>' binary
	    {gen_alu(ALU_GT, ">");}
	| binary '<' binary
	    {gen_alu(ALU_LT, "<");}
	| binary GE binary
	    {gen_alu(ALU_GE, ">=");}
	| binary LE binary
	    {gen_alu(ALU_LE, "<=");}
	| binary EQ binary
	    {gen_alu(ALU_EQ, "==");}
	| binary NE binary
	    {gen_alu(ALU_NE, "!=");}
	| binary '&' binary
	    {gen_alu(ALU_AND, "&");}
	| binary '|' binary
	    {gen_alu(ALU_OR, "|");}
	| binary '^' binary
	    {gen_alu(ALU_XOR, "^");}
	| Identifier '=' binary
	    { 
	        chk_var($1); 
	        gen_direct(OP_STORE, gen_mod($1), OFFSET($1), NAME($1));
	    }
	| Identifier PE
	    { 
	        chk_var($1); 
	        gen_direct(OP_LOAD, gen_mod($1), OFFSET($1), NAME($1));
	    }
	  binary
	    { 
	        gen_alu(ALU_ADD, "+");
	        gen_direct(OP_STORE, gen_mod($1), OFFSET($1), NAME($1));
	    }
	| Identifier ME
	    { 
	        chk_var($1); 
	        gen_direct(OP_LOAD, gen_mod($1), OFFSET($1), NAME($1));
	    }
	  binary
	    { 
	        gen_alu(ALU_SUB, "-");
	        gen_direct(OP_STORE, gen_mod($1), OFFSET($1), NAME($1));
	    }
	| Identifier TE binary
	    { 
	        chk_var($1); 
	        gen_direct(OP_LOAD, gen_mod($1), OFFSET($1), NAME($1));
	    }
	  binary
	    { 
	        gen_alu(ALU_MUL, "*");
	        gen_direct(OP_STORE, gen_mod($1), OFFSET($1), NAME($1));
	    }
	| Identifier DE binary
	    { 
	        chk_var($1); 
	        gen_direct(OP_LOAD, gen_mod($1), OFFSET($1), NAME($1));
	    }
	  binary
	    { 
	        gen_alu(ALU_DIV, "/");
	        gen_direct(OP_STORE, gen_mod($1), OFFSET($1), NAME($1));
	    }
	| Identifier RE binary
	    { 
	        chk_var($1); 
	        gen_direct(OP_LOAD, gen_mod($1), OFFSET($1), NAME($1));
	    }
	  binary
	    { 
	        gen_alu(ALU_MOD, "%");
	        gen_direct(OP_STORE, gen_mod($1), OFFSET($1), NAME($1));
	    }

optional_argument_list
	: /* no actual arguments */
	    {$$ = 0;}
	| argument_list
	    /* default action, $$ = $1, = # of actual arguments */

argument_list
	: binary
	    {$$ = 1;}
	| argument_list ',' binary
	    {
	        ++$$;
	        yyerrok;
	    }
	| error
	    {$$ = 0;}
	| argument_list error
	| argument_list ',' error
	    
/*
 *  make certain terminal symbols very important
 */

rp  : ')'   {yyerrok;}	    
sc  : ';'   {yyerrok;}	    
rr  : '}'   {yyerrok;}	    

