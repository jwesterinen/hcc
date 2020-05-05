int g;

void foo(int *a)
{
    g = a;
}

int bar(int a)
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

