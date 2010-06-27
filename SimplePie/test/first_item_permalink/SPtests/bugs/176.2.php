<?php

class SimplePie_First_Item_Permalink_Test_Bug_176_Test_2 extends SimplePie_First_Item_Permalink_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom">
	<entry>
		<link rel="related" href="http://example.com/related"/>
		<link rel="alternate" href="http://example.com/alternate"/>
		<link rel="via" href="http://example.com/via"/>
	</entry>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'http://example.com/alternate';
	}
}

?>