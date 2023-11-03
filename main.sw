let sub(let x, let y)
{
    ret x - y;
}

let add(let x, let y)
{
    ret sub(x, y);
}

let main()
{
    ret add(9, 3);
}
