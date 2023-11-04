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
    ret factorial(5);
}
