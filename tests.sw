int While()
{
    int x = 10;
    while(x > 0)
    {
        x = x - 1;
    }
    if(x == 0)
    {
        ret 0;
    }
    else
    {
        ret 1;
    }
}

int If()
{
    if(1)
    {
        ret 0;
    }
    ret 1;
}

int Elif()
{
    if(0)
    {
        ret 1;
    }
    elif(1)
    {
        ret 0;
    }
    ret 1;
}

int Else()
{
    if(0)
    {
        ret 1;
    }
    elif(0)
    {
        ret 1;
    }
    else
    {
        ret 0;
    }
    ret 1;
}

int OneStarPointer()
{
    int x = 0;
    int *p = &x;
    *p = 42;
    if(x == 42)
    {
        ret 0;
    }
    else
    {
        ret 1;
    }
}

int TwoStarPointer()
{
    int x = 0;
    int *p = &x;
    int **pp = &p;
    **pp = 42;
    if(x == 42)
    {
        ret 0;
    }
    else
    {
        ret 1;
    }
}

int NStarPointer()
{
    int a = 99;
    int* b = &a;
    int** c = &b;
    int*** d = &c;
    int**** e = &d;
    int***** f = &e;
    int****** g = &f;
    int******* h = &g;
    int******** i = &h;
    int********* j = &i;
    int********** k = &j;

    if(**********k == 99)
    {
        ret 0;
    }
    else
    {
        ret 1;
    }
}

int Operators()
{
    int x = 0;
    x = 9;
    if(x != 9)
    {
        ret 1;
    }
    if(-5 != 0 - 5)
    {
        ret 1;
    }
    if(+5 != 5)
    {
        ret 1;
    }
    if(!1 != 0)
    {
        ret 1;
    }
    if(~0 != -1)
    {
        ret 1;
    }
    if(1 + 1 != 2)
    {
        ret 1;
    }
    if(2 - 1 != 1)
    {
        ret 1;
    }
    if(1 & 3 != 1)
    {
        ret 1;
    }
    if(1 ^ 1 != 0)
    {
        ret 1;
    }
    if(1 | 2 != 3)
    {
        ret 1;
    }
    if(5 * 5 != 25)
    {
        ret 1;
    }
    if(10 / 2 != 5)
    {
        ret 1;
    }
    if(10 % 6 != 4)
    {
        ret 1;
    }
    if((5 == 5) != 1)
    {
        ret 1;
    }
    if((1 < 2) != 1)
    {
        ret 1;
    }
    if((2 > 1) != 1)
    {
        ret 1;
    }
    if((1 <= 1) != 1)
    {
        ret 1;
    }
    if((1 >= 1) != 1)
    {
        ret 1;
    }
    ret 0;
}

int Array()
{
    int a[] = { 0, 1, 2, 3, 4, 5 };

    # Read.
    if(*(a + 0) != 0) { ret 1; }
    if(*(a + 1) != 1) { ret 1; }
    if(*(a + 2) != 2) { ret 1; }
    if(*(a + 3) != 3) { ret 1; }
    if(*(a + 4) != 4) { ret 1; }
    if(*(a + 5) != 5) { ret 1; }

    # Write
    *(a + 0) = 5;
    *(a + 1) = 4;
    *(a + 2) = 3;
    *(a + 3) = 2;
    *(a + 4) = 1;
    *(a + 5) = 0;
    if(*(a + 0) != 5) { ret 1; }
    if(*(a + 1) != 4) { ret 1; }
    if(*(a + 2) != 3) { ret 1; }
    if(*(a + 3) != 2) { ret 1; }
    if(*(a + 4) != 1) { ret 1; }
    if(*(a + 5) != 0) { ret 1; }

    # Brace match size
    int b[3] = { 0, 1, 2 };
    if(*(b + 0) != 0) { ret 1; }
    if(*(b + 1) != 1) { ret 1; }
    if(*(b + 2) != 2) { ret 1; }

    # All same with one element
    int c[3] = { 9 };
    if(*(c + 0) != 9) { ret 1; }
    if(*(c + 1) != 9) { ret 1; }
    if(*(c + 2) != 9) { ret 1; }

    ret 0;
}

int Assign()
{
    int x = 0;
    x = 99;
    if(x != 99)
    {
        ret 1;
    }
    else
    {
        ret 0;
    }
}

int LinkedAssign()
{
    int x = 0;
    int y = 0;
    int z = 0;
    x = y = z = 99;
    if(x != 99) { ret 1; }
    if(y != 99) { ret 1; }
    if(z != 99) { ret 1; }
    ret 0;
}

int main()
{
    if(While() != 0)          { ret  1; }
    if(If() != 0)             { ret  2; }
    if(Elif() != 0)           { ret  3; }
    if(Else() != 0)           { ret  4; }
    if(OneStarPointer() != 0) { ret  5; }
    if(TwoStarPointer() != 0) { ret  6; }
    if(NStarPointer() != 0)   { ret  7; }
    if(Operators() != 0)      { ret  8; }
    if(Array() != 0)          { ret  9; }
    if(Assign() != 0)         { ret 10; }
    if(LinkedAssign() != 0)   { ret 11; }
    ret 0;
}
