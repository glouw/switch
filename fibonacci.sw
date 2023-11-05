let fibonacci(let x)
{
    if(x == 0)
    {
        ret 0;
    }
    elif(x == 1)
    {
        ret 1;
    }
    else
    {
        ret fibonacci(x - 1)
          + fibonacci(x - 2);
    }
}

let main()
{
    ret fibonacci(13);
}
