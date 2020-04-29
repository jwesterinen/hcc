/*
 * main function for parser
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <getopt.h>
#include "y.tab.h"

extern FILE *yyerfp;

// options
static char *outfileName = 0;
static char *infileName = 0;
int emitVmCode = 0;
int verbose = 0;

static void ParseOptions(int argc, char* argv[])
{
	const char* optStr = "CD:I:PU:o:ivh";
	int opt;

	while ((opt = getopt(argc, argv, optStr)) != -1)
	{
		switch (opt)
		{
		    // cpp options, just pass thru
		    case 'C':
		    case 'D':
		    case 'I':
		    case 'P':
		    case 'U':
		        break;
		    
			case 'o':
				outfileName = optarg;
				break;
			case 'i':
				emitVmCode = 1;
				break;
			case 'v':
				verbose = 1;
				break;
			case 'h':
				printf("usage: hcc [-o <filename>] [-v] [-h]\n");
				printf("\n");
				printf("     options:\n");
				printf("         -o <filename>: set the output file name\n");
				printf("         -v:            set verbose mode\n");
				printf("         -h:            display this help\n");
				exit(0);
			default:
				printf("usage: hcc [CDIPU] [-o <filename>] [-v] [-h]\n");
				exit(-1);
		}
	}
	
	// the input file name is the first non-option cmd line arg
	if (optind < argc)
	{
	    infileName = argv[optind];
	}
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
 *  main() -- run C preprocessor then yyparse()
 */
int main(int argc, char** argv)
{
    // output file
    extern FILE* yyout;
    
    // init the error stream
    //yyerfp = stdout;
    yyerfp = stderr;

#ifdef YYDEBUG
    extern int yydebug;
    //yydebug = 1;
#endif

    // for now cpp will always be called
    int cppflag = 1;
    
	// parse the command line options
	ParseOptions(argc, argv);

    // redirect infileName as stdin      
    if (infileName && !freopen(infileName, "r", stdin))
    {
        perror(infileName);
        exit(1);
    }
    
    // run the C preprocessor
    if (cppflag && cpp(argc, argv))
    {
        perror("C preprocessor");
        exit(1);
    }
        
    // open the output file if specified (otherwise output will be to stdout)
    if (outfileName)
    {
        yyout = fopen(outfileName, "w");
    }
    
    // run the parser
    exit(yyparse());
}

// end of main.c

