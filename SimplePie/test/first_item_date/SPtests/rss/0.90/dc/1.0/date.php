<?php

class SimplePie_First_Item_Date_Test_RSS_090_DC_10_Date extends SimplePie_First_Item_Date_Test
{
	function data()
	{
		$this->data = 
'<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns="http://my.netscape.com/rdf/simple/0.9/" xmlns:dc="http://purl.org/dc/elements/1.0/">
	<item>
		<dc:date>2007-01-11T16:00:00Z</dc:date>
	</item>
</rdf:RDF>';
	}
	
	function expected()
	{
		$this->expected = 1168531200;
	}
}

?>