<?php

class SimplePie_First_Item_Latitude_Test_Atom_10_Georss_Point extends SimplePie_First_Item_Latitude_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom" xmlns:georss="http://www.georss.org/georss">
	<entry>
		<georss:point>55.701 12.552</georss:point>
	</entry>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 55.701;
	}
}

?>