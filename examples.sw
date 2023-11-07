# Prints an integer <x> to stdout
int putint(int x)
{
    if(x < 0)
    {
        x = -x;
        $ '-'; # The dollar sign calls putchar() in C's stdlib
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
    ret indices;
}

# Returns fibonacci of <x>
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

# Returns factorial of <x>
int factorial(int x)
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

# Swaps two values in array <a> at position <x> and <y>
int swap(int* a, int x, int y)
{
    int temp = *(a + x);
    *(a + x) = *(a + y);
    *(a + y) = temp;
}

# Quick sorts array <a> using starting position <l> at 0
# and ending position <r> at size - 1
int qsort(int* a, int l, int r)
{
    int i = 0;
    int last = 0;
    if(l >= r)
    {
        ret 0;
    }
    swap(a, l, (l + r) / 2);
    last = l;
    i = l + 1;
    while(i <= r)
    {
        if(*(a + i) < *(a + l))
        {
            last = last + 1;
            swap(a, last, i);
        }
        i = i + 1;
    }
    swap(a, l, last);
    qsort(a, l, last - 1);
    qsort(a, last + 1, r);
}

# Returns length of null terminated string <s>
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

# Prints string <s>
int putstr(int* s)
{
    while(*s)
    {
        int c = *s;
        $ c;
        s = s + 1;
    }
    $ '\n';
}

# Returns 1 if strings <a> and <b> are equal
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

# Returns 1 if arrays <a> and <b> are equal, based on their
# respective sizes <size_a> and <size_b>
int arreq(int* a, int* b, int size_a, int size_b)
{
    if(size_a != size_b)
    {
        ret 0;
    }
    int i = 0;
    while(i < size_a)
    {
        if(*(a + i) != *(b + i))
        {
            ret 0;
        }
        i = i + 1;
    }
    ret 1;
}

# Print array <a> of <size>
int putarr(int* a, int size)
{
    int i = 0;
    while(i < size)
    {
        putint(*(a + i));
        i = i + 1;
    }
}

# Showcase how Switch can be used
int main()
{
    if(factorial(8) != 40320)
    {
        ret 1;
    }
    if(fibonacci(15) != 610)
    {
        ret 2;
    }
    int a[] = { 9, 43, 1, 5, 32 };
    int b[] = { 1, 5, 9, 32, 43 };
    qsort(a, 0, @a - 1);
    putarr(a, @a);
    putarr(b, @b);
    if(!arreq(a, b, @a, @b))
    {
        ret 3;
    }
    int s[] = { 's','w','i','t','c','h','\0' };
    int t[] = { 's','t','r','i','n','g','\0' };
    putstr(s);
    putstr(t);
    if(strlen(s) != 6)
    {
        ret 4;
    }
    if(!streq(s, s))
    {
        ret 5;
    }
    if(streq(s, t))
    {
        ret 6;
    }
}
