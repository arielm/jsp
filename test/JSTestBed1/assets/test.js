var foo =
{
    x: 1.55,
    name: "bar",
    ages: [5, 6, 7],
    children:
    [
        {bar: "baz", y: 233},
        {bar: "BAZ", size: new Number(1.5)}
    ],
    time: new Date("2014-09-27T21:14:32.695Z")
};

function start()
{
    try
    {
        console.log(getDump(foo));
    }
    catch(e)
    {
        console.error(e);
    }
}
