let test()
{
    let a = 1;
    let b = 2;
    let* c = &a + 0;
    ret *c;
}

let fun(let a)
{
    let b = 4;
    ret a + b + test();
}

let main()
{
    ret fun(9);
}
