<?php

class SimplePie_First_Item_Date_Test_Atom_10_Updated extends SimplePie_First_Item_Date_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom">
	<entry>
		<updated>2007-01-11T16:00:00Z</updated>
	</entry>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 1168531200;
	}
}

?>