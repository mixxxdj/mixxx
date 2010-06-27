<?php

class SimplePie_First_Item_Permalink_Test_Atom_03_Enclosure extends SimplePie_First_Item_Permalink_Test
{
	function data()
	{
		$this->data = 
'<feed version="0.3" xmlns="http://purl.org/atom/ns#">
	<entry>
		<link href="http://example.com/" rel="enclosure"/>
	</entry>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'http://example.com/';
	}
}

?>