int strlen(int* s)
{
    int len = 0;
    while(*s)
    {
        len = len + 1;
        s = s + 1;
    }
    ret len;
}

int puts(int* s)
{
    while(*s)
    {
        int c = *s;
        $ c;
        s = s + 1;
    }
    $ '\n';
}

int streq(int* a, int* b)
{
    int x = strlen(a);
    int y = strlen(b);
    if(x == y)
    {
        int i = 0;
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

int main()
{
    int s[] = { 's','w','i','t','c','h','\0' };
    int t[] = { 's','t','r','i','n','g','\0' };
    if(strlen(s) != 6)
    {
        ret 1;
    }
    if(!streq(s, s))
    {
        ret 2;
    }
    if(streq(s, t))
    {
        ret 3;
    }
    puts(s);
    ret 0;
}
