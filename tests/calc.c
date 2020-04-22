/* calc -- A simple, 4-function RPN calculator
 *
 *  Test case 1 -- test all digits and functions including negative mult and div
 *   Enter      Display
 *   123         123
 *   <enter>     123
 *   456         456
 *   +           579
 *   789         789
 *   -          -210
 *   2           002
 *   *          -420
 *   10          010
 *   /          -042
 *   100         100
 *   +           058
 *   4           004
 *   *           232
 *   2           002
 *   /           116
 *
 *  Test case 2 -- non-trivial expression, (11+5)/(6+2) = 2
 *   <clr> 11 <enter> 5 + 6 <enter> 2 + /  should display 2
 *
 *  Test case 3 -- RPN city
 *   Enter      Display
 *   2           002
 *   <enter>     002
 *   3           003
 *   <enter>     003
 *   4           004
 *   <enter>     004
 *   5           005
 *   <enter>     005
 *   6           006
 *   <enter>     006
 *   +           011
 *   +           015
 *   +           018
 *   +           020
 * 
 */

// CLR, +, -, *, /
#define KEYCODE_CLR         30
#define KEYCODE_PLUS        27
#define KEYCODE_MINUS       28
#define KEYCODE_MULTIPLY    29
#define KEYCODE_DIVIDE      31

// converts a digit keycode to an integer
#define KEYCODE_MASK        15

int isNewEntry;
int keycode;
int a, b;

main()
{
    isNewEntry = 1;
    
    ClearDisplay();
    
    while (1)
    {
        // get the next key pressed
        keycode = ReadKey();
        
        if (keycode == KEYCODE_CLR)
        {
            // reset the system
            ResetSys();
            continue;
        }
        
        // accept and process entries
        if (keycode < 26)
        {
            // rotate a new digit into the display
            if (isNewEntry)
                ClearDisplay();
            isNewEntry = 0;
            AppendNum3(keycode & KEYCODE_MASK);
        }
        else 
        {
            if (isNewEntry == 0)
            {
                // push displayed value onto the expr stack for any non-digit key
                ExprPush(Getsd());
            }
            if (keycode == KEYCODE_PLUS)
            {
                // add: TOS = a + b
                ExprPut(ExprPop() + ExprTop());
            }
            else if (keycode == KEYCODE_MINUS)
            {
                // sub: TOS = b - a
                a = ExprPop();
                b = ExprTop();
                ExprPut(b - a);
            }
            else if (keycode == KEYCODE_MULTIPLY)
            {
                // mul: TOS = a * b
                ExprPut(ExprPop() * ExprTop());
            }
            else if (keycode == KEYCODE_DIVIDE)
            {
                // div: TOS = b / a
                a = ExprPop();
                b = ExprTop();
                ExprPut(b / a);
            }
            
            // display the expression result
            Printsd(ExprTop());
            
            // flag a new entry so that the display will clear when a key is pressed
            isNewEntry = 1;
        }
    }
}

// end of calc.c
