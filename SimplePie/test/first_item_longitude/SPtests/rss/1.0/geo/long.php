<?php

class SimplePie_First_Item_Longitude_Test_RSS_10_Geo_Long extends SimplePie_First_Item_Longitude_Test
{
	function data()
	{
		$this->data = 
'<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns="http://purl.org/rss/1.0/" xmlns:geo="http://www.w3.org/2003/01/geo/wgs84_pos#">
	<item>
		<geo:lat>55.701</geo:lat>
		<geo:long>12.552</geo:long>
	</item>
</rdf:RDF>';
	}
	
	function expected()
	{
		$this->expected = 12.552;
	}
}

?>