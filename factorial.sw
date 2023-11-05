let factorial(let x)
{
    if(x == 0)
    {
        ret 1;
    }
    else
    {
        ret x * factorial(x - 1);
    }
}

let main()
{
    if(factorial(8) == 40320)
    {
        ret 0;
    }
    else
    {
        ret 1;
    }
}
