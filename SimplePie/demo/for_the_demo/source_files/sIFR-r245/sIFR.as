/*=:project
    scalable Inman Flash Replacement (sIFR) version 3.

  =:file
    Copyright: 2006 Mark Wubben.
    Author: Mark Wubben, <http://novemberborn.net/>

  =:history
    * IFR: Shaun Inman
    * sIFR 1: Mike Davidson, Shaun Inman and Tomas Jogin
    * sIFR 2: Mike Davidson, Shaun Inman, Tomas Jogin and Mark Wubben

  =:license
    This software is licensed and provided under the CC-GNU LGPL.
    See <http://creativecommons.org/licenses/LGPL/2.1/>    
*/

import SifrStyleSheet;

class sIFR {
  public static var DEFAULT_TEXT                 = 'Rendered with sIFR 3, revision 245';
  public static var CSS_ROOT_CLASS               = 'sIFR-root';
  public static var DEFAULT_WIDTH                = 300;
  public static var DEFAULT_HEIGHT               = 100;
  public static var DEFAULT_ANTI_ALIAS_TYPE      = 'advanced';
  public static var MARGIN_LEFT                  = -3;
  public static var PADDING_BOTTOM               = 5; // Extra padding to make sure the movie is high enough in most cases.
  public static var LEADING_REMAINDER            = 2; // Flash uses the specified leading minus 2 as the applied leading.

  public static var MAX_FONT_SIZE                = 126;
  public static var ALIASING_MAX_FONT_SIZE       = 48;
  
  //= Holds CSS properties and other rendering properties for the Flash movie.
  //  *Don't overwrite!*
  public static var styles:SifrStyleSheet        = new SifrStyleSheet();
  //= Allow sIFR to be run from localhost
  public static var fromLocal:Boolean            = true;
  //= Array containing domains for which sIFR may render text. Used to prevent
  //  hotlinking. Use `*` to allow all domains.
  public static var domains:Array                = [];
  //= Whether kerning is enabled by default. This can be overriden from the client side.
  //  See also <http://livedocs.macromedia.com/flash/8/main/wwhelp/wwhimpl/common/html/wwhelp.htm?context=LiveDocs_Parts&file=00002811.html>.
  public static var defaultKerning:Boolean       = true;
  //= Default value which can be overriden from the client side.
  //  See also <http://livedocs.macromedia.com/flash/8/main/wwhelp/wwhimpl/common/html/wwhelp.htm?context=LiveDocs_Parts&file=00002788.html>.
  public static var defaultSharpness:Number      = 0;
  //= Default value which can be overriden from the client side.
  //  See also <http://livedocs.macromedia.com/flash/8/main/wwhelp/wwhimpl/common/html/wwhelp.htm?context=LiveDocs_Parts&file=00002787.html>.
  public static var defaultThickness:Number      = 0;
  //= Default value which can be overriden from the client side.
  //  See also <http://livedocs.macromedia.com/flash/8/main/wwhelp/wwhimpl/common/html/wwhelp.htm?context=LiveDocs_Parts&file=00002732.html>.
  public static var defaultOpacity:Number        = -1; // Use client settings
  //= Default value which can be overriden from the client side.
  //  See also <http://livedocs.macromedia.com/flash/8/main/wwhelp/wwhimpl/common/html/wwhelp.htm?context=LiveDocs_Parts&file=00002788.html>.
  public static var defaultBlendMode:Number      = -1; // Use cliest settings
  //= Overrides the grid fit type as defined on the client side.
  //  See also <http://livedocs.macromedia.com/flash/8/main/wwhelp/wwhimpl/common/html/wwhelp.htm?context=LiveDocs_Parts&file=00002444.html>.
  public static var enforcedGridFitType:String   = null;
  //= If `true` sIFR won't override the anti aliasing set in the Flash IDE when exporting.
  //  Thickness and sharpness won't be affected either.
  public static var preserveAntiAlias:Boolean    = false;
  //= If `true` sIFR will disable anti-aliasing if the font size is larger than `ALIASING_MAX_FONT_SIZE`.
  //  This setting is *independent* from `preserveAntiAlias`.
  public static var conditionalAntiAlias:Boolean = true;
  //= Sets the anti alias type. By default it's `DEFAULT_ANTI_ALIAS_TYPE`.
  //  See also <http://livedocs.macromedia.com/flash/8/main/wwhelp/wwhimpl/common/html/wwhelp.htm?context=LiveDocs_Parts&file=00002733.html>.
  public static var antiAliasType:String         = null;
  //= Flash filters can be added to this array and will be applied to the text field.
  public static var filters:Array                = [];
  //= A mapping from the names of the filters to their actual objecs, used when transforming
  //  filters defined on the client. You can add additional filters here so they'll be supported
  //  when defined on the client.
  public static var filterMap:Object             = {
    DisplacementMapFilter : flash.filters.DisplacementMapFilter,
    ColorMatrixFilter     : flash.filters.ColorMatrixFilter,
    ConvolutionFilter     : flash.filters.ConvolutionFilter,
    GradientBevelFilter   : flash.filters.GradientBevelFilter,
    GradientGlowFilter    : flash.filters.GradientGlowFilter,
    BevelFilter           : flash.filters.BevelFilter,
    GlowFilter            : flash.filters.GlowFilter,
    BlurFilter            : flash.filters.BlurFilter,
    DropShadowFilter      : flash.filters.DropShadowFilter
  };

  private static var instance;
  
  private var textField;
  private var content;
  private var realHeight;
  private var originalHeight;
  private var currentHeight;
  private var fontSize;
  private var tuneWidth;
  private var tuneHeight;
  

  
  //= Sets the default styles for `sIFR.styles`. This method is called
  //  directly in `sifr.fla`, before options are applied.
  public static function setDefaultStyles() {
    sIFR.styles.parseCSS([
      '.', CSS_ROOT_CLASS, ' { color: #000000; }',
      'strong { display: inline; font-weight: bold; } ',
      'em { display: inline; font-style: italic; }',
      'a { color: #0000FF; text-decoration: underline; }',
      'a:hover { color: #0000FF; text-decoration: none; }'
    ].join(''));
  }
  
  //= Validates the domain sIFR is being used on.
  //  Returns `true` if the domain is valid, `false` otherwise.  
  public static function checkDomain():Boolean {
    if(sIFR.domains.length == 0) return true;

    var domain = (new LocalConnection()).domain();
    if(sIFR.fromLocal) sIFR.domains.push('localhost');
    
    for(var i = 0; i < sIFR.domains.length; i++) {
      var match = sIFR.domains[i];
      if(match == '*' || match == domain) return true;

      var wildcard = match.lastIndexOf('*');
      if(wildcard > -1) {
        match = match.substr(wildcard + 1);
        var matchPosition = domain.lastIndexOf(match);
        if(matchPosition > -1 && (matchPosition + match.length) == domain.length) return true;
      }
    }
    
    return false;
  }
  
  //= Runs sIFR. Called automatically.
  public static function run() {
    var holder  = _root.holder;
    var content = checkDomain() ? unescape(_root.content) : DEFAULT_TEXT
    if(content == 'undefined' || content == '') {
      content = DEFAULT_TEXT;
      fscommand('resetmovie', '');
    } else fscommand('ping', '');

    // Sets stage parameters
    Stage.scaleMode = 'noscale';
    Stage.align = 'TL';
    Stage.showMenu = false;
    
    // Other parameters
    var opacity = parseInt(_root.opacity);
    if(!isNaN(opacity)) holder._alpha = sIFR.defaultOpacity == -1 ? opacity : sIFR.defaultOpacity;
    else holder._alpha = 100;
    _root.blendMode = sIFR.defaultBlendMode == -1 ? _root.blendmode : sIFR.defaultBlendMode;

    sIFR.instance = new sIFR(holder.txtF, content);
    // This should ignore resizes from the callback. Disabled for now.
/*    if(_root.zoomsupport == 'true') Stage.addListener({onResize: function() { sIFR.instance.scale() }});*/

    // Setup callbacks
    _root.watch('callbackTrigger', function() { 
      sIFR.callback();
      return false;
    });
  }
  
  private static function eval(str) {
    var as;
    
    if(str.charAt(0) == '{') { // Ah, we need to create an object
      as = {};
      str = str.substring(1, str.length - 1);
      var $ = str.split(',');
      for(var i = 0; i < $.length; i++) {
        var $1 = $[i].split(':');
        as[$1[0]] = sIFR.eval($1[1]);
      }
    } else if(str.charAt(0) == '"') { // String
      as = str.substring(1, str.length - 1);
    } else if(str == 'true' || str == 'false') { // Boolean
      as = str == 'true';
    } else { // Float
      as = parseFloat(str);
    }
    
    return as;
  }
  
  private function applyFilters() {
    var $filters = this.textField.filters;
    $filters = $filters.concat(sIFR.filters);
    
    var $ = _root.flashfilters.split(';'); // name,prop:value,...;
    for(var i = 0; i < $.length; i++) {
      var $1 = $[i].split(',');
      
      var newFilter = new sIFR.filterMap[$1[0]]();
      for(var j = 1; j < $1.length; j++) {
        var $2 = $1[j].split(':');
        newFilter[$2[0]] = sIFR.eval(unescape($2[1]));
      }
      
      $filters.push(newFilter);
    }

    this.textField.filters = $filters;
  }
  
  private function sIFR(textField, content) {
    this.textField = textField;
    this.content   = content;

    var offsetLeft = parseInt(_root.offsetleft);
    textField._x = MARGIN_LEFT + (isNaN(offsetLeft) ? 0 : offsetLeft);
    var offsetTop = parseInt(_root.offsettop);
    if(!isNaN(offsetTop)) textField._y += offsetTop;
    
    tuneWidth = parseInt(_root.tunewidth);
    if(isNaN(tuneWidth)) tuneWidth = 0;
    tuneHeight = parseInt(_root.tuneheight);
    if(isNaN(tuneHeight)) tuneHeight = 0;

    textField._width = tuneWidth + (isNaN(parseInt(_root.width)) ? DEFAULT_WIDTH : parseInt(_root.width));
    textField._height = tuneHeight + (isNaN(parseInt(_root.height)) ? DEFAULT_HEIGHT : parseInt(_root.height));
    textField.wordWrap = true;
    textField.selectable = _root.selectable == 'true';
    textField.gridFitType = sIFR.enforcedGridFitType || _root.gridfittype;
    this.applyFilters();

    // Determine font-size and the number of lines
    this.fontSize = parseInt(_root.size);
    if(isNaN(this.fontSize)) this.fontSize = 26;
    styles.fontSize = this.fontSize;
    
    if(!sIFR.preserveAntiAlias && (sIFR.conditionalAntiAlias && this.fontSize < ALIASING_MAX_FONT_SIZE
    || !sIFR.conditionalAntiAlias)) {
      textField.antiAliasType = sIFR.antiAliasType || DEFAULT_ANTI_ALIAS_TYPE;      
    }

    if(!sIFR.preserveAntiAlias || !isNaN(parseInt(_root.sharpness))) {
      textField.sharpness = parseInt(_root.sharpness);
    }
    if(isNaN(textField.sharpness)) textField.sharpness = sIFR.defaultSharpness;

    if(!sIFR.preserveAntiAlias || !isNaN(parseInt(_root.thickness))) {
      textField.thickness = parseInt(_root.thickness);
    }
    if(isNaN(textField.thickness)) textField.thickness = sIFR.defaultThickness;
    
    // Set font-size and other styles
    sIFR.styles.parseCSS(unescape(_root.css));
    
    var rootStyle = styles.getStyle('.sIFR-root') || {};
    rootStyle.fontSize = this.fontSize; // won't go higher than 126!
    styles.setStyle('.sIFR-root', rootStyle);
    textField.styleSheet = styles;
    
    this.write(content);
    this.repaint();
  }
  
  private function repaint() {
    var leadingFix = this.isSingleLine() ? sIFR.styles.latestLeading : 0;
    if(leadingFix > 0) leadingFix -= LEADING_REMAINDER;

    // Flash wants to scroll the movie by one line, by adding the fontSize to the
    // textField height this is no longer happens. We also add the absolute tuneHeight,
    // to prevent a negative value from triggering the bug. We won't send the fake
    // value to the JavaScript side, though.
    textField._height = textField.textHeight + PADDING_BOTTOM + this.fontSize + Math.abs(tuneHeight) + tuneHeight - leadingFix;
    this.realHeight = textField._height - this.fontSize - Math.abs(tuneHeight);
    var arg = 'height:' + this.realHeight;
    if(_root.fitexactly == 'true') arg += ',width:' + (textField.textWidth + tuneWidth);
    fscommand('resize', arg);
        
    this.originalHeight = textField._height;
    this.currentHeight = Stage.height;

    textField._xscale = textField._yscale = parseInt(_root.zoom);
  }
  
  private function write(content) {
    this.textField.htmlText = ['<p class="', CSS_ROOT_CLASS, '">', 
                                content, '</p>'
                              ].join('');
  }
  
  private function isSingleLine() {
    return Math.round((this.textField.textHeight - sIFR.styles.latestLeading) / this.fontSize) == 1;
  }

  //= Scales the text field to the new scale of the Flash movie itself.
  public function scale() {
    this.currentHeight = Stage.height;
    var scale = 100 * Math.round(this.currentHeight / this.originalHeight);
    textField._xscale = textField._yscale = scale;
  }
  
  private function calculateRatios() {
    var strings = ['X', 'X<br>X', 'X<br>X<br>X', 'X<br>X<br>X<br>X'];
    var results = {};

    for(var i = 1; i <= strings.length; i++) {
      var size = 6;
    
      this.write(strings[i - 1]);
      while(size < MAX_FONT_SIZE) {
        var rootStyle = sIFR.styles.getStyle('.sIFR-root') || {};
        rootStyle.fontSize = size;
        sIFR.styles.setStyle('.sIFR-root', rootStyle);
        this.textField.styleSheet = sIFR.styles;
        this.repaint();
        var ratio = (this.realHeight - PADDING_BOTTOM) / i / size;
        if(!results[size]) results[size] = ratio;
        else results[size] = ((i - 1) * results[size] + ratio) / i;
        size++;
      }
    }

    var sizes = [], ratios = [];
    var ratiosToSizes = {}, sizesToRatios = {};

    for(var size in results) {
      if(results[size] == Object.prototype[size]) continue;
      var ratio = results[size];
      ratiosToSizes[ratio] = Math.max(ratio, parseInt(size));
    }

    for(var ratio in ratiosToSizes) {
      if(ratiosToSizes[ratio] == Object.prototype[ratio]) continue;
      sizesToRatios[ratiosToSizes[ratio]] = roundDecimals(ratio, 2);
      sizes.push(ratiosToSizes[ratio]);
    }

    sizes.sort(function(a, b) { return a - b; });
    for(var j = 0; j < sizes.length - 1; j++) ratios.push(sizes[j], sizesToRatios[sizes[j]]);
    ratios.push(sizesToRatios[sizes[sizes.length - 1]]);

    fscommand('debug:ratios', '[' + ratios.join(',') + ']');
  }
  
  private function roundDecimals(value, decimals) {
    return Math.round(value * Math.pow(10, decimals)) / Math.pow(10, decimals);
  }
  
  public static function callback() {
    switch(_root.callbackType) {
      case 'replacetext':
        sIFR.instance.content = _root.callbackValue;
        sIFR.instance.write(_root.callbackValue);
        sIFR.instance.repaint();
        break;
      case 'resettext':
        sIFR.instance.write('');
        sIFR.instance.write(sIFR.instance.content);
        break;
      case 'ratios':
        sIFR.instance.calculateRatios();
        break;
    }
  }
}
