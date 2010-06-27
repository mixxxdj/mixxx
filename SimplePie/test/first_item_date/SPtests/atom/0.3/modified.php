<?php

class SimplePie_First_Item_Date_Test_Atom_03_Modified extends SimplePie_First_Item_Date_Test
{
	function data()
	{
		$this->data = 
'<feed version="0.3" xmlns="http://purl.org/atom/ns#">
	<entry>
		<modified>2007-01-11T16:00:00Z</modified>
	</entry>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 1168531200;
	}
}

?>