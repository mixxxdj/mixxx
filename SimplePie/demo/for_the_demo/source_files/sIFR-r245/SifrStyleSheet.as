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

import TextField.StyleSheet;

class SifrStyleSheet extends TextField.StyleSheet {
  public var fontSize;
  public var latestLeading = 0;
  
  public function parseCSS(cssText:String) {
    var native = new TextField.StyleSheet();
    var parsed = native.parseCSS(cssText);
    
    if(!parsed) return false;
    
    var selectors = native.getStyleNames();
    for(var i = selectors.length - 1; i >= 0; i--) {
      var selector = selectors[i];
      var nativeStyle = native.getStyle(selector);
      var style = this.getStyle(selector) || nativeStyle;
      if(style != nativeStyle) {
        for(var property in nativeStyle) style[property] = nativeStyle[property];
      }
      this.setStyle(selector, style);
    }
    
    return true;
  }
  
  // Apply leading to the textFormat. Much thanks to <http://www.blog.lessrain.com/?p=98>.
  private function applyLeading(format, leading) {
    this.latestLeading = leading;
    
    if(leading >= 0) {
        format.leading = leading;
        return format;
    }

    // Workaround for negative leading, which is ignored otherwise.
    var newFormat = new TextFormat(null, null, null, null, null, null, null, null, null, null, null, null, leading);
    for(var property in format) if(property != 'leading') newFormat[property] = format[property];

    return newFormat;
  }
  
  public function transform(style) {
    var format = super.transform(style);
    if(style.leading) format = applyLeading(format, style.leading);
    if(style.letterSpacing) format.letterSpacing = style.letterSpacing;
    // Support font sizes relative to the size of .sIFR-root.
    if(this.fontSize && style.fontSize && style.fontSize.indexOf('%')) {
      format.size = this.fontSize * parseInt(style.fontSize) / 100;
    }
    format.kerning = _root.kerning == 'true' || !(_root.kerning == 'false') || sIFR.defaultKerning;
    return format;
  }
}