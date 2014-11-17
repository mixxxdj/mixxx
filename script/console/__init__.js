__setupPackage__(__extension__);
/**
 * This is a fake Firebug console api. To debug scripts a few more like in
 * javascript.
 * 
 * More info :
 * http://blog.qt.digia.com/blog/2012/03/01/debugging-qt-quick-2-console-api/
 * 
 */


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
