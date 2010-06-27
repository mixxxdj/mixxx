<?php

class SimplePie_First_Item_Permalink_Test_Atom_10_xmlbase_3 extends SimplePie_First_Item_Permalink_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom" xml:base="http://example.org/">
	<entry>
		<link rel="alternate" href="//example.com/alternate"/>
	</entry>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'http://example.com/alternate';
	}
}

?>