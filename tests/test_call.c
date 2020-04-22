int g;

main()
{
    int x;
    
    x = 1;
    foo(x);
    x = x + 1;
    foo(x);
}

foo(a)
{
    g = a;
}

