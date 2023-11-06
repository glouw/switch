let strlen(let* s)
{
    let len = 0;
    while(*s)
    {
        len = len + 1;
        s = s + 1;
    }
    ret len;
}

let puts(let* s)
{
    while(*s)
    {
        let c = *s;
        $ c;
        s = s + 1;
    }
    $ '\n';
}

let streq(let* a, let* b)
{
    let x = strlen(a);
    let y = strlen(b);
    if(x == y)
    {
        let i = 0;
        while(i < x)
        {
            if(*(a + i) != *(b + i))
            {
                ret 0;
            }
            i = i + 1;
        }
        ret 1;
    }
    ret 0;
}

let main()
{
    let s[] = { 's','w','i','t','c','h','\0' };
    let t[] = { 's','t','r','i','n','g','\0' };
    if(strlen(s) != 6)
    {
        ret 1;
    }
    if(*(s + 0) !=  's') { ret 2; }
    if(*(s + 1) !=  'w') { ret 3; }
    if(*(s + 2) !=  'i') { ret 4; }
    if(*(s + 3) !=  't') { ret 5; }
    if(*(s + 4) !=  'c') { ret 6; }
    if(*(s + 5) !=  'h') { ret 7; }
    if(*(s + 6) != '\0') { ret 8; }

    # Just for show.
    puts(s);

    if(!streq(s, s))
    {
        ret 9;
    }

    if(streq(s, t))
    {
        ret 10;
    }

    ret 0;
}
