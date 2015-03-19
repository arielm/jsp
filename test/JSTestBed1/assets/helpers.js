var console =
{
    log: function(message)
    {
        print(message);
    },

    error: function(exception)
    {
        print("ERROR: " + getErrorReport(exception));
    }
};

function getErrorReport(exception)
{
    if (exception)
    {
        return "[" + exception.fileName + " | LINE " + exception.lineNumber + "] " + exception.message;
    }
    else
    {
        return "";
    }
}

/*
 * USING A CUSTOM "REPLACER FUNCTION", IN ORDER TO AVOID "TypeError: cyclic object value"
 * SEE: http://stackoverflow.com/questions/11616630/json-stringify-avoid-typeerror-converting-circular-structure-to-json#11616993
 */
function getDump(object)
{
    if (object)
    {
        var cache = [];

        return JSON.stringify(
            object,
            function (key, value) {
                if (typeof value === 'object' && value !== null) {
                    if (cache.indexOf(value) !== -1) {
                        return null; // Circular reference found, discard key
                    }

                    cache.push(value); // Store value in our collection
                }

                return value;
            },
            4
        );
    }
    else {
        return "";
    }
}

function dump(object)
{
    print(getDump(object));
}
