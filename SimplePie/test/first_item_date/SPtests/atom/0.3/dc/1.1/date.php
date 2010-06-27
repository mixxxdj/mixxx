<?php

class SimplePie_First_Item_Date_Test_Atom_03_DC_11_Date extends SimplePie_First_Item_Date_Test
{
	function data()
	{
		$this->data = 
'<feed version="0.3" xmlns="http://purl.org/atom/ns#" xmlns:dc="http://purl.org/dc/elements/1.1/">
	<entry>
		<dc:date>2007-01-11T16:00:00Z</dc:date>
	</entry>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 1168531200;
	}
}

?>