int swap(int* a, int x, int y)
{
    int temp = *(a + x);
    *(a + x) = *(a + y);
    *(a + y) = temp;
}

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

int main()
{
    int a[] = { 9, 43, 1, 5, 32 };
    int b[] = { 1, 5, 9, 32, 43 };
    qsort(a, 0, 4);
    if(*(a + 0) != *(b + 0)) { ret 1; }
    if(*(a + 1) != *(b + 1)) { ret 1; }
    if(*(a + 2) != *(b + 2)) { ret 1; }
    if(*(a + 3) != *(b + 3)) { ret 1; }
    if(*(a + 4) != *(b + 4)) { ret 1; }
    ret 0;
}
