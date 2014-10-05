__setupPackage__(__extension__);

svg.templateHooks = {};

svg.regexpQuote = function (str, delimiter) {
    return String(str).replace(
        new RegExp(
            '[.\\\\+*?\\[\\^\\]$(){}=!<>|:\\' + (delimiter || '') + '-]',
            'g'
        ),
        '\\$&'
    );
}

svg.getHooksPattern = function(){
    var hookNames = [],
        that = this;
    for( var i in this.templateHooks )
        hookNames.push(i);
    
    // hook_name( arg1 [, arg2]... )
    if( hookNames.length ){
        var pattern = "("+hookNames.join('|')+")\\(([^\\(\\)]+)\\)\\s*;?";
        
        // console.log('!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!');
        // console.log(pattern);
        return pattern;
    }
}

global = this;
svg.templateHooks.variable = function( varName ){
    // console.log('!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!');
    // console.log(global[varName]);
    if( varName in global ){
        return global[varName];
    }
    return '';
}

svg.templateHooks.prop = function( propName, varName ){
    var out = '';
    
    if( (varName in global) ){
        var value = global[varName];
        
        if( isNumber(value) ){
            out = propName + ':' + value + ';';
        } else if( value.length ) {
            out = propName + ':' + value + ';';
        }
        
    } else {
        print( 'Unable to find ' + varName + ' for prop hook.' );
    }
    
    // print( varName + ' => ' out + ' | ' + (varName in global) );
    return out;
}

