int fibonacci(int x)
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
        ret fibonacci(x - 1) + fibonacci(x - 2);
    }
}

int main()
{
    if(fibonacci(15) == 610)
    {
        ret 0;
    }
    else
    {
        ret 1;
    }
}
