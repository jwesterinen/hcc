# 1 "test_assign.c"
# 1 "<built-in>"
# 1 "<command-line>"
# 31 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 32 "<command-line>" 2
# 1 "test_assign.c"
int g;

main(a)
{
    int x,y;

    x = g;
    x = 5;
    x = a;
    x = y;

    g = 1;
    a = 2;
}
