let fibbonacci(let x)
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
        ret fibbonacci(x - 1)
          + fibbonacci(x - 2);
    }
}

let main()
{
    ret fibbonacci(13);
}
