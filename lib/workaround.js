onmessage = function(event) {
    try{
        var block = event.data.replace(/^.*{/m, '{');
        var code  = new Function(block);
        code();
    }catch(e){
        throw e;
    }
};
