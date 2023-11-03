let _fun()
{
    ret -1;
}

let puts(int *s)
{
    while(*s)
    {
        $*s;
        s = s + 1;
    }
}

let func()
{
    let string[] = {'t','e','s','t','\0'}; # strings are defined like this
    let x = _fun();
    let i = 0;
    while(i < 10)
    {
        let y = *(string + i);
        i = i + 1;
    }


    let *array[16] = { &x };
    let *p = &x + (1 - 2);
    let *z = *&p;
    let test = x;
    while(1)
    {
    }
    if(1)
    {
    }
    elif(2)
    {
    }
    elif(3)
    {
    }
    else
    {
    }
    while(1)
    {
    }
    ret 0;
}
