<?php

class SimplePie_First_Item_Longitude_Test_Atom_03_Georss_Point extends SimplePie_First_Item_Longitude_Test
{
	function data()
	{
		$this->data = 
'<feed version="0.3" xmlns="http://purl.org/atom/ns#" xmlns:georss="http://www.georss.org/georss">
	<entry>
		<georss:point>55.701 12.552</georss:point>
	</entry>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 12.552;
	}
}

?>