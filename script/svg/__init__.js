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

svg.hookNames = function(){
    var hookNames = ['variable'],
        that = this;
    for( var i in this.templateHooks )
        hookNames.push(i);
    
    hookNames.toPattern = function(){
        for( var i in this )
            this[i] = that.regexpQuote(this[i]);
        return this.join('|');
    }
    
    return hookNames;
}

global = this;
svg.templateHooks.variable = variable = function( varName ){
	// console.log('!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!');
	// console.log(global[varName]);
    if( varName in global ){
        return global[varName];
    }
    return '';
}

svg.templateHooks.prop = prop = function( propName, varName ){
    var out = '';
    
    if( (varName in global) ){
        var value = global[varName];
        
        if( isNumber(value) ){
            out = propName + ':' + value + ';';
        } else if( value.length ) {
            out = propName + ':' + value + ';';
        }
        
    } else {
        // print( 'Unable to find ' + varName + ' for prop hook.' );
    }
    
    // print( varName + ' => ' out + ' | ' + (varName in global) );
    return out;
}

