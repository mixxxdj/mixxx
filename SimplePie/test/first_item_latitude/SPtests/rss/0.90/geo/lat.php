<?php

class SimplePie_First_Item_Latitude_Test_RSS_090_Geo_Lat extends SimplePie_First_Item_Latitude_Test
{
	function data()
	{
		$this->data = 
'<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns="http://my.netscape.com/rdf/simple/0.9/" xmlns:geo="http://www.w3.org/2003/01/geo/wgs84_pos#">
	<item>
		<geo:lat>55.701</geo:lat>
		<geo:long>12.552</geo:long>
	</item>
</rdf:RDF>';
	}
	
	function expected()
	{
		$this->expected = 55.701;
	}
}

?>