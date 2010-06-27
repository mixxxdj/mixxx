// MTASC only parses as-files with class definitions, so here goes...
class Options {
	public static function apply() {
		sIFR.fromLocal = true;
		sIFR.domains   = ['*'];
		
		// Parsing `p.foo` might not work, see: <http://livedocs.macromedia.com/flash/mx2004/main_7_2/wwhelp/wwhimpl/common/html/wwhelp.htm?context=Flash_MX_2004&file=00001766.html>
		// Appearantly you have to use hex color codes as well, names are not supported!

		sIFR.styles.parseCSS('.foo { text-decoration: underline; }'); 
	}
}
