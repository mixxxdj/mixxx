<?php

class SimplePie_First_Item_ID_Test_Atom_03_ID extends SimplePie_First_Item_ID_Test
{
	function data()
	{
		$this->data = 
'<feed version="0.3" xmlns="http://purl.org/atom/ns#">
	<entry>
		<id>http://example.com/</id>
	</entry>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'http://example.com/';
	}
}

?>