__setupPackage__(__extension__);

console = {
    log : function(){
        var out = [],
            i   = 0;
        for( ; i<arguments.length; i++ ){
            out.push( JSON.stringify(arguments[i]) );
        }
        print(out.join(' '));
    }
}
