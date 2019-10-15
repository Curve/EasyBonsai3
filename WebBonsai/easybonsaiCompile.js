    function compile()
    {

        var eCode = /*Code*/;
        var compiled = Module.getCode(eCode);
        var newCode = "";
        for(var i = 0; compiled.size() > i; i++)
        {
            newCode += compiled.get(i) + "\n";
        }
        /*Work with new Code*/
    }