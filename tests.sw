let While()
{
    let x = 10;
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

let If()
{
    if(1)
    {
        ret 0;
    }
    ret 1;
}

let Elif()
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

let Else()
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

let OneStarPointer()
{
    let x = 0;
    let *p = &x;
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

let TwoStarPointer()
{
    let x = 0;
    let *p = &x;
    let **pp = &p;
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

let Operators()
{
    let x = 0;
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

let main()
{
    if(While() != 0)
    {
        ret 1;
    }
    if(If() != 0)
    {
        ret 2;
    }
    if(Elif() != 0)
    {
        ret 3;
    }
    if(Else() != 0)
    {
        ret 4;
    }
    if(OneStarPointer() != 0)
    {
        ret 5;
    }
    if(TwoStarPointer() != 0)
    {
        ret 6;
    }
    if(Operators() != 0)
    {
        ret 7;
    }
    ret 0;
}
