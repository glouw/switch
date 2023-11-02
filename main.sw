let test(let x)
{
    ret x;
}

let func()
{
    let x = 1;
    let *p = &x + (1 - 2);
    let *z = *&p;
    let *X = *&p + ~z;
    ~test() + 1;

    if (1)
    {
    }
    elif (2)
    {
    }
    elif (3)
    {
    }
    else
    {
    }
    while (1)
    {
    }

    ret 0;
}
