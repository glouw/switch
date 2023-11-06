int putint(int x)
{
    if(x < 0)
    {
        x = -x;
        $ '-';
    }
    int temp = x;
    int indices = 0;
    while(temp != 0)
    {
        temp = temp / 10;
        indices = indices + 1;
    }
    int i = indices - 1;
    while(i != -1)
    {
        temp = x;
        int j = 0;
        while(j < i)
        {
            temp = temp / 10;
            j = j + 1;
        }
        int c = '0' + temp % 10;
        $ c;
        i = i - 1;
    }
    $ '\n';
}

int main()
{
    putint(-12345);
}
