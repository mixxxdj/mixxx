<?php

class SimplePie_First_Item_Title_Test_Atom_10_Title_xmlbase_1 extends SimplePie_First_Item_Title_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom">
	<entry>
		<title type="xhtml" xml:base="http://example.com/"><div xmlns="http://www.w3.org/1999/xhtml"><p xml:base="/test/"><a href="bleh">Link</a></p></div></title>
	</entry>
</feed>';
	}
	
	function expected()
	{
		$this->expected = '<p><a href="http://example.com/bleh">Link</a></p>';
	}
}

?>