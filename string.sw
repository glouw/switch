let puts(let *str)
{
    let size = 0;
    while(*str != 0)
    {
        let c = *str;
        $c;
        str = str + 1;
        size = size + 1;
    }
    ret size;
}

let main()
{
    let A = 99; # padding check
    let x[] = { 's','w','i','t','c','h',0 };
    let B = 99; # padding check
    if(*(x + 0) != 's') { ret 2; }
    if(*(x + 1) != 'w') { ret 3; }
    if(*(x + 2) != 'i') { ret 4; }
    if(*(x + 3) != 't') { ret 5; }
    if(*(x + 4) != 'c') { ret 6; }
    if(*(x + 5) != 'h') { ret 7; }
    if(*(x + 6) !=  0 ) { ret 8; }
    if(puts(x) == 6)
    {
        ret 0;
    }
    else
    {
        ret 1;
    }
}
