/*
 *  codegen.c
 */

#include <stdio.h>
#include <stdlib.h>
#include "message.h"
#include "symtab.h"
#include "gen.h"
#include "gen_hack.h"

extern FILE *yyout;
extern int emitVmCode;
extern int verbose;

static struct Bc_stack
{
    int bc_label;               // label from new_label
    struct Bc_stack *bc_next;
} *b_top,                       // head of break stack 
  *c_top;                       // head of continue stack
  
char *gen_mod(struct Symtab *symbol)
{
    switch (symbol->s_blknum)
    {
        case 1:
            return MOD_GLOBAL;
        case 2:
            return MOD_PARAM;
    }    
    return MOD_LOCAL;
}

#define LABEL "_LP%d"
static char *format_label(int label)
{
    static char buffer[sizeof LABEL + 2];
    sprintf(buffer, LABEL, label);
    return buffer;
}

int new_label()
{
    static int next_label = 0;
    return ++next_label;
}

static struct Bc_stack *push(struct Bc_stack *stack, int label)
{
    struct Bc_stack *new_entry = (struct Bc_stack *)calloc(1, sizeof(struct Bc_stack));
    
    if (new_entry)
    {
        new_entry->bc_next = stack;
        new_entry->bc_label = label;
        return new_entry;
    }

    fatal("no more room to compile loops");    
    return (struct Bc_stack *)NULL;
}

static struct Bc_stack *pop(struct Bc_stack *stack)
{
    struct Bc_stack *old_entry;
    
    if (stack)
    {
        old_entry = stack;
        stack = old_entry->bc_next;
        free(old_entry);
        return stack;
    }

    bug("break/continue stack underflow");    
    return (struct Bc_stack *)NULL;
}

static int top(struct Bc_stack *stack)
{
    if (!stack)
    {
        error("no loop open");
        return 0;
    }

    return stack->bc_label;
}

void push_break(int label)
{
    b_top = push(b_top, label);
}

void push_continue(int label)
{
    c_top = push(c_top, label);
}

void pop_break()
{
    b_top = pop(b_top);
}

void pop_continue()
{
    c_top = pop(c_top);
}

void gen_break()
{
    gen_jump(OP_JUMP, top(b_top), "BREAK");
}

void gen_continue()
{
    gen_jump(OP_JUMP, top(c_top), "CONTINUE");
}

void end_program()
{
    // allocate global variables
    all_program();
    gen_pre(OP_END, "end of program");
}


// the following are the actual code generation functions

void gen_begin()
{
    if (emitVmCode)
    {
        fprintf(yyout, "\t%s\n", OP_BEGIN);
    }
    else
    {
        GenBegin();
    }
}

void gen_alu(const char *mod, const char *comment)
{
    if (emitVmCode)
    {
        fprintf(yyout, "\t%s\t%s\t\t;\t%s\n", OP_ALU, mod, comment);
    }
    else
    {
        GenAlu(mod, comment);
    }
}

void gen_load_immed(const char *constant)
{
    if (emitVmCode)
    {
        fprintf(yyout, "\t%s\t%s\t%s\n", OP_LOAD, MOD_IMMED, constant);
    }
    else
    {
        GenLoadImmed(constant);
    }
}

void gen(const char *op, const char *mod, int val, const char *comment)
{
    if (emitVmCode)
    {
        fprintf(yyout, "\t%s\t%s\t%d\t;\t%s\n", op, mod, val, comment);
    }
    else
    {
        Gen(op, mod, val, comment);
    }
}

void gen_pre(const char *op, const char *comment)
{
    if (emitVmCode)
    {
        fprintf(yyout, "\t%s\t\t\t;\t%s\n", op, comment);
    }
    else
    {
        GenPopRetEnd(op, comment);
    }
}

int gen_jump(const char *op, int label, const char *comment)
{
    if (emitVmCode)
    {
        fprintf(yyout, "\t%s\t%s\t\t;\t%s\n", op, format_label(label), comment);
    }
    else
    {
        GenJump(op, format_label(label), comment);
    }
    
    return label;
}

int gen_label(int label)
{
    if (emitVmCode)
    {
        //fprintf(yyout, "%s\tequ\t*\n", format_label(label));
        fprintf(yyout, "label\t%s\n", format_label(label));
    }
    else
    {
        GenLabel(format_label(label));
    }
    
    return label;
}

void gen_call(struct Symtab *symbol, int count)
{
    chk_parm(symbol, count);
    if (emitVmCode)
    {
        //fprintf(yyout, "\t%s\t%d,%s\n", OP_CALL, count, symbol->s_name);
        fprintf(yyout, "\t%s\t%s\n", OP_CALL, symbol->s_name);
    }
    else
    {
        GenCall(symbol->s_name);
    }
    while (count-- > 0)
    {
        gen_pre(OP_POP, "pop argument");
    }
    gen(OP_LOAD, MOD_GLOBAL, 0, "push result");
}

int gen_entry(struct Symtab *symbol)
{
    int label = new_label();
    
    if (emitVmCode)
    {
        fprintf(yyout, "%s\t%s\t%s\n", OP_ENTRY, symbol->s_name, format_label(label));
    }
    else
    {
        GenEntry(symbol->s_name, format_label(label));
    }
    
    return label;
}

void fix_entry(struct Symtab *symbol, int label)
{
    if (emitVmCode)
    {
        fprintf(yyout, "%s\tequ\t%d\t\t;\t%s\n", format_label(label), l_max, symbol->s_name);
    }
    else
    {
        GenEqu(format_label(label), l_max);
    }
}

// end of gen.c

