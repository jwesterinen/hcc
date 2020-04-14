int g;

main(a)
{
    int x,y;
    
    x = g;      // load global, store local
    x = 5;      // load const
    x = a;      // load param
    x = y;      // load local
    
    g = 1;      // store global
    a = 2;      // store param
}

