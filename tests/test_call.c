int g;

void foo(a)
{
    g = a;
}

int bar(a)
{
    return a+1;
}

void main()
{
    int x, y;
    
    x = 1;
    foo(x);
    x = x + 1;
    foo(x);
    y = bar(x);
}

