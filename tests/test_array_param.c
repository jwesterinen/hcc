// test arrays

int b[5];
int n;

void foo(int n[])
{
    n[3] = 8;
}

void main()
{
    int a[5];
    int x;

    foo(a);   // local array parameter
    foo(b);   // global array parameter    
}
