<?php

class SimplePie_First_Item_Latitude_Test_RSS_090_Georss_Point extends SimplePie_First_Item_Latitude_Test
{
	function data()
	{
		$this->data = 
'<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns="http://my.netscape.com/rdf/simple/0.9/" xmlns:georss="http://www.georss.org/georss">
	<item>
		<georss:point>55.701 12.552</georss:point>
	</item>
</rdf:RDF>';
	}
	
	function expected()
	{
		$this->expected = 55.701;
	}
}

?>