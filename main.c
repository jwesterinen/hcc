/*
 * main function for parser
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "y.tab.h"

extern FILE *yyerfp;

static int usage(const char* name)
{
    fputs("usage: ", stderr);
    fputs(name, stderr);
    fputs(" [C preprocessor options] [source]\n", stderr);
    exit(1);
}

/*
 * cpp() -- preprocess lex input() through C preprocessor
 */

#ifndef CPP
#   define CPP "/usr/bin/cpp"
#endif

int cpp(int argc, char** argv)
{
    char **argp, *cmd;
    extern FILE* yyin;      // for lex input
    int i;
    
    for (i = 0, argp = argv; *++argp; )
    {
        if (**argp == '-' && strchr("CDEIUP", (*argp)[i]))
        {
            i += strlen(*argp) + 1;
        }
    }
    
    if (!(cmd = (char*)calloc(i + sizeof CPP, sizeof(char))))
    {
        return -1;
    }
    
    strcpy(cmd, CPP);
    for (argp = argv; *++argp; )
    {
        if (**argp == '-' && strchr("CDEIUP", (*argp)[i]))
        {
            strcat(cmd, " "), strcat(cmd, *argp);
        }
    }
    
    if ((yyin = popen(cmd, "r")))
        i = 0;      // all's well
    else
        i = -1;     // no preprocessor
    free(cmd);
    
    return i;
}

/*
 *  main() -- possibly run C preprocessor before yyparse()
 */
int main(int argc, char** argv)
{
    // init the error stream
    //yyerfp = stdout;
    yyerfp = stderr;

#ifdef YYDEBUG
    extern int yydebug;
    //yydebug = 1;
#endif

    char** argp;
    int cppflag = 1;
    for (argp = argv; *++argp && **argp == '-'; )
    {
        switch((*argp)[1])
        {
            case 'C':
            case 'D':
            case 'E':
            case 'I':
            case 'P':
            case 'U':
                cppflag = 1;
                break;
            default:
                usage(argv[0]);
                break;
        }
    }
    if (argp[0] && argp[1])
        usage(argv[0]);
    if (*argp && !freopen(*argp, "r", stdin))
        perror(*argp), exit(1);
    if (cppflag && cpp(argc, argv))
        perror("C preprocessor"), exit(1);
    exit(yyparse());
}

// end of main.c

