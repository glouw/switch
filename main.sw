let _fun()
{
    ret -1;
}

let func()
{
    let x = _fun();
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
