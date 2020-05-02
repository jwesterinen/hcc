/*
 *  gen.c - hack code generator
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "message.h"
#include "symtab.h"
#include "gen.h"

// output file as defined in the parser
extern FILE *yyout;

char *curFileName;
char *curFctName;
int labelId = 0;
int returnId = 0;
unsigned curLocalVarQty;

#define STACK_BASE 256
#define FRAME_BASE 512
#define EXPR_BASE 768

void GenBeginProg()
{
    fprintf(yyout, "\n// begin program\n");
    
	// set the initial stack, frame, and expression base pointers
	fprintf(yyout, "    @%d\n", STACK_BASE);
	fprintf(yyout, "    D=A\n");
	fprintf(yyout, "    @SP\n");
	fprintf(yyout, "    M=D\n");
	fprintf(yyout, "    @%d\n", FRAME_BASE);
	fprintf(yyout, "    D=A\n");
	fprintf(yyout, "    @BP\n");
	fprintf(yyout, "    M=D\n");
	fprintf(yyout, "    @%d\n", EXPR_BASE);
	fprintf(yyout, "    D=A\n");
	fprintf(yyout, "    @EP\n");
	fprintf(yyout, "    M=D\n");
	
	// jump to main()
	fprintf(yyout, "    @main\n");
	fprintf(yyout, "    0;JMP\n");
}

void GenEndProg()
{
    fprintf(yyout, "\n// end program\n");
    
    fprintf(yyout, "(__Exit)\n");
    fprintf(yyout, "    @__Exit\n");
    fprintf(yyout, "    0;JMP\n\n");
}

void GenCall(const char *fctname)
{
    fprintf(yyout, "\n// call\n");
    
    fprintf(yyout, "    @L%d\n", ++labelId);     // push return address
    fprintf(yyout, "    D=A\n");
    fprintf(yyout, "    @SP\n");
    fprintf(yyout, "    M=M+1\n");
    fprintf(yyout, "    A=M\n");
    fprintf(yyout, "    M=D\n");
    fprintf(yyout, "    @%s\n", fctname);        // jump to function
    fprintf(yyout, "    0;JMP\n");
    fprintf(yyout, "(L%d)\n", labelId);          // return address
}

void GenEntry(const char *fctname, const char *symbol)
{    
    fprintf(yyout, "\n// entry\n");

    curFctName = (char *)fctname;    
    
    fprintf(yyout, "(%s)\n", fctname);
    fprintf(yyout, "    @BP\n");                // push current BP
    fprintf(yyout, "    D=M\n");
    fprintf(yyout, "    @SP\n");
    fprintf(yyout, "    M=M+1\n");
    fprintf(yyout, "    A=M\n");
    fprintf(yyout, "    M=D\n");    
    fprintf(yyout, "    @SP\n");                // move SP to BP
    fprintf(yyout, "    D=M\n");
    fprintf(yyout, "    @BP\n");
    fprintf(yyout, "    M=D\n");
	fprintf(yyout, "    @%s\n", symbol);        // adjust SP past local variables
	fprintf(yyout, "    D=A\n");
    fprintf(yyout, "    @SP\n");
    fprintf(yyout, "    M=D+M\n");
}

void GenAlu(const char *mod, const char *comment)
{
    fprintf(yyout, "\n// alu, %s, %s\n", mod, comment);
    
	//if (mod == "+" || mod == "-" || mod == "&" || mod == "|")
	if (!strcmp(mod, "+") || !strcmp(mod, "-") || !strcmp(mod, "&") || !strcmp(mod, "|"))
	{
		fprintf(yyout, "    @SP\n");             // pop y
		fprintf(yyout, "    M=M-1\n");
		fprintf(yyout, "    A=M+1\n");
		fprintf(yyout, "    D=M\n");
		fprintf(yyout, "    A=A-1\n");
		if (!strcmp(mod, "+"))
		{
			fprintf(yyout, "    M=D+M\n");       // TOS = x + y
		}
		else if (!strcmp(mod, "-"))
		{
			fprintf(yyout, "    M=M-D\n");	    // TOS = x - y
		}
		else if (!strcmp(mod, "&"))
		{
			fprintf(yyout, "    M=D&M\n");	    // TOS = x & y
		}
		else if (!strcmp(mod, "|"))
		{
			fprintf(yyout, "    M=D|M\n");	    // TOS = x | y
		}
	}
	else if (!strcmp(mod, "*") || !strcmp(mod, "/"))
	{
		if (!strcmp(mod, "*"))
		{
		    // create a dummy symbol table entry for Multiply() fct
			GenCall("Multiply");       // TOS = x * y
		}
		else if (!strcmp(mod, "/"))
		{
		    // create a dummy symbol table entry for Divide() fct
			GenCall("Divide");	    // TOS = x / y
		}
	    fprintf(yyout, "    @RETVAL\n");
	    fprintf(yyout, "    D=M\n");
	    fprintf(yyout, "    @SP\n");
	    fprintf(yyout, "    M=M-1\n");
	    fprintf(yyout, "    A=M\n");
	    fprintf(yyout, "    M=D\n");
    }
#ifdef NEG_AND_NOT_IMPLMENTED_IN_THE_PARSER	
	else if (!strcmp(mod, "neg") || !strcmp(mod, "not\n"));
	{
		fprintf(yyout, "    @SP\n");		    // pop y
		fprintf(yyout, "    A=M\n");
		if (!strcmp(mod, "neg"))
		{
			fprintf(yyout, "    M=-M\n");	    // [sp-1] = -y
		}
		else if (!strcmp(mod, "not\n"))
		{
			fprintf(yyout, "    M=!M\n");	    // [sp-1] = !y
		}
	}
#endif
	else if (!strcmp(mod, "==") || !strcmp(mod, "!=") || !strcmp(mod, ">") || !strcmp(mod, ">=") || 
	         !strcmp(mod, "<") || !strcmp(mod, "<="))
	{
	    
		fprintf(yyout, "    @SP\n");		    // pop y
		fprintf(yyout, "    M=M-1\n");
		fprintf(yyout, "    A=M+1\n");
		fprintf(yyout, "    D=M\n");
		fprintf(yyout, "    A=A-1\n");
		fprintf(yyout, "    D=M-D\n");          // make the logical comparison (x-y)
	    fprintf(yyout, "    @SP\n");            // replace TOS with True (assume True)
	    fprintf(yyout, "    A=M\n");
	    fprintf(yyout, "    M=-1\n");
		fprintf(yyout, "    @LT%d\n", ++labelId); // jump if the following logical expressions are True
		if (!strcmp(mod, "=="))
		{
			fprintf(yyout, "    D;JEQ\n");      // x == y
		}
		else if (!strcmp(mod, "!="))
		{
			fprintf(yyout, "    D;JNE\n");      // x != y           
		}
		else if (!strcmp(mod, ">"))
		{
			fprintf(yyout, "    D;JGT\n");      // x > y
		}
		else if (!strcmp(mod, ">="))
		{
			fprintf(yyout, "    D;JGE\n");      // x >= y
		}
		else if (!strcmp(mod, "<"))
		{
			fprintf(yyout, "    D;JLT\n");      // x < y
		}
		else if (!strcmp(mod, "<="))
		{
			fprintf(yyout, "    D;JLE\n");      // x <= y
		}
		fprintf(yyout, "    @SP\n");			// otherwise replace TOS with False
		fprintf(yyout, "    A=M\n");
		fprintf(yyout, "    M=0\n");
		fprintf(yyout, "(LT%d)\n", labelId);
	}
	else
	{
		fprintf(yyout, "illegal mod\n");
	}
}

void GenLoadImmed(const char *constant)
{
    fprintf(yyout, "\n// load immed\n");
        
    fprintf(yyout, "    @%s\t// constant\n", constant);
    fprintf(yyout, "    D=A\n");
    fprintf(yyout, "    @SP\n");
    fprintf(yyout, "    M=M+1\n");
    fprintf(yyout, "    A=M\n");
    fprintf(yyout, "    M=D\n");
}

// set the effective address (M) based on the variable type
static void GenAccessVar(const char *vartype, int offset, const char *globalName)
{
    if (!strcmp(vartype, "gbl"))
    {
        if (offset == 0)
        {
            // global 0 signifies the global address of the return value of the function, "RETVAL"
	        fprintf(yyout, "    @RETVAL\t// fct return value\n");
	    }
	    else
	    {
            // use the global name directly
            fprintf(yyout, "    @%s\t// global\n", globalName);
        }
    }
    else if (!strcmp(vartype, "par"))
    {
        // parameter n is at BP-(n+2)
        offset += 2;
        fprintf(yyout, "    @%d\t// %s\n", offset, globalName); // adjust the var offset from the BP
        fprintf(yyout, "    D=A\n");
        fprintf(yyout, "    @BP\n");                    // point to the var
        fprintf(yyout, "    A=M-D\n");
    }
    else if (!strcmp(vartype, "lcl"))
    {
        // local variable n is at BP+n+1
        offset += 1;
        fprintf(yyout, "    @%d\t// %s\n", offset, globalName); // adjust the var offset from the BP
        fprintf(yyout, "    D=A\n");
        fprintf(yyout, "    @BP\n");                    // point to the var
        fprintf(yyout, "    A=D+M\n");
    }
    else
    {
        fprintf(stderr, "Error: illegal variable type - exiting...\n");
        exit(1);
    }
}

void GenDirect(const char *op, const char *vartype, int offset, const char *globalName)
{
    if (!strcmp(op, OP_LOAD))
    {
        fprintf(yyout, "\n// load\n");
        
        GenAccessVar(vartype, offset, globalName);      // point to the effective address of the var
        fprintf(yyout, "    D=M\n");
        fprintf(yyout, "    @SP\n");                    // push the var
        fprintf(yyout, "    M=M+1\n");               
        fprintf(yyout, "    A=M\n");
        fprintf(yyout, "    M=D\n");
    }
    else if (!strcmp(op, OP_STORE))
    {
        fprintf(yyout, "\n// store\n");
        
        if (!strcmp(vartype, "gbl"))
        {
            // use the global name directly
            fprintf(yyout, "    @SP\n");                // copy TOS to D
            fprintf(yyout, "    A=M\n");
            fprintf(yyout, "    D=M\n");
            GenAccessVar(vartype, offset, globalName);  // write D to global address
	        fprintf(yyout, "    M=D\n");	    
        }
        else
        {
            GenAccessVar(vartype, offset, globalName);  // get the var _address_
            fprintf(yyout, "    D=A\n");        
            fprintf(yyout, "    @SP\n");                // push the var address (<segment addr>+<index>)
            fprintf(yyout, "    M=M+1\n");
            fprintf(yyout, "    A=M\n");
            fprintf(yyout, "    M=D\n");
            
            fprintf(yyout, "    @SP\n");                // load D with the TOS-1 value
            fprintf(yyout, "    A=M\n");
            fprintf(yyout, "    A=A-1\n");
            fprintf(yyout, "    D=M\n");
            fprintf(yyout, "    @SP\n");                // pop the ea
            fprintf(yyout, "    M=M-1\n");              //   [SP] = <segment addr>+<index>
            fprintf(yyout, "    A=M+1\n");              //   ea = [<segment addr>+<index>]
            fprintf(yyout, "    A=M\n");                // write the value to the ea
            fprintf(yyout, "    M=D\n");
        }
    }
    else if (!strcmp(op, OP_INC))
    {
        fprintf(yyout, "\n// inc\n");
        
        GenAccessVar(vartype, offset, globalName);
        fprintf(yyout, "    M=M+1\n");                  // increment the var
        fprintf(yyout, "    @SP\n");                    // dummy push to accomodate the parser's insisting on a pop after inc
        fprintf(yyout, "    M=M+1\n");
    }
    else if (!strcmp(op, OP_DEC))
    {
        fprintf(yyout, "\n// dec\n");
       
        GenAccessVar(vartype, offset, globalName);
        fprintf(yyout, "    M=M-1\n");                  // decrement the var
        fprintf(yyout, "    @SP\n");                    // dummy push to accomodate the parser's insisting on a pop after inc
        fprintf(yyout, "    M=M+1\n");
    }
}

// FIXME: break these up into their own functions
void GenPopRet(const char *op, const char *comment)
{
    if (!strcmp(op, OP_POP))
    {
        fprintf(yyout, "\n// pop\n");
        
	    fprintf(yyout, "    @SP\n");
	    fprintf(yyout, "    M=M-1\n");
    }
    else if (!strcmp(op, OP_RETURN))
    {
        // optimization: no need to return from main - WriteEnd() takes care of that
        
        fprintf(yyout, "\n// return\n");
        
        fprintf(yyout, "    @BP\n");                 // move BP to SP to remove local vars
        fprintf(yyout, "    D=M-1\n");
        fprintf(yyout, "    @SP\n");
        fprintf(yyout, "    M=D\n");    
        fprintf(yyout, "    @SP\n");                 // restore old BP
        fprintf(yyout, "    A=M+1\n");
        fprintf(yyout, "    D=M\n");
        fprintf(yyout, "    @BP\n");
        fprintf(yyout, "    M=D\n");            
        if (!strcmp(curFctName, "main"))
        {
            fprintf(yyout, "    @__Exit\n");         // for main() jump to the end of the program -- essentially "exit"
            fprintf(yyout, "    0;JMP\n");
        }
        else
        {
            fprintf(yyout, "    @SP\n");             // pop old BP, now return is TOS
            fprintf(yyout, "    M=M-1\n");
            fprintf(yyout, "    A=M+1\n");           // restore the return address and return
            fprintf(yyout, "    A=M\n");
            fprintf(yyout, "    0;JMP\n");
        }
    }
}

void GenJump(const char *op, const char *label, const char *comment)
{
    if (!strcmp(op, OP_JUMPZ))
    {
        fprintf(yyout, "\n// jumpz\t%s\n", comment);
        
        fprintf(yyout, "    @SP\n");
        fprintf(yyout, "    A=M\n");
        fprintf(yyout, "    D=M\n");
        fprintf(yyout, "    @%s\n", label);
        fprintf(yyout, "    D;JEQ\n");
    }
    else
    {
        fprintf(yyout, "\n// jump\t%s\n", comment);
        
        fprintf(yyout, "    @%s\n", label);
        fprintf(yyout, "    0;JMP\n");
    }
}

void GenLabel(const char *label)
{
    fprintf(yyout, "(%s)\n", label);
}

void GenEqu(const char *symbol, int value)
{
    fprintf(yyout, "%s\tequ\t%d\n", symbol, value);
}

// end of gen.c

