let add(let x, let y)
{
    let a = 1;
    let b = 2;
    let c = 3;
    ret x + y;
}

let temp(let x, let y)
{
    let a = 1;
    let b = 2;
    let c = 3;
    ret add(x, y);
}

let main()
{
    let a = 1;
    let b = 2;
    let c = 3;
    ret temp(9, 3);
}
