/*
 * symtab.h -- definitions for symbol table
 */

struct Symtab
{
    char   *s_name;             // name
    int     s_type;             // symbol type
    int     s_blknum;           // static block depth
    union 
    {
        int s__num;
        struct Symtab *s__link;
    } s__; 
    int     s_offset;           // symbol definition   
    struct Symtab *s_next;      // next entry
};

#define s_pnum  s__.s__num      // count of parameters
#define NOT_SET (-1)            // no count yet set
#define s_plist s__.s__link     // chain of parameters

// s_type values
#define UDEC    0               // not declared
#define FUNC    1               // function
#define UFUNC   2               // undefined function
#define VAR     3               // declared variable
#define PARM    4               // undeclared parameter

// s_type string values
#define SYMmap   "udecl", "fct", "udef fct", "var", "parm"

// offsets
extern int g_offset, l_offset, l_max;

void init();
void blk_push();
void blk_pop();
void all_program();
void all_func(struct Symtab *symbol);
void all_parm(struct Symtab *symbol);
void s_lookup(int yylex);
struct Symtab *s_find(const char *name);
struct Symtab *link_parm(struct Symtab *symbol, struct Symtab *next);
void all_var(struct Symtab *symbol);
struct Symtab *make_func(struct Symtab *symbol);
void chk_parm(struct Symtab *symbol, int count);
int parm_default(struct Symtab *symbol);
void chk_var(struct Symtab *symbol);
void chk_func(struct Symtab *symbol);

