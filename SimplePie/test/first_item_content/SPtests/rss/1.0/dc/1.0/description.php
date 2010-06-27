<?php

class SimplePie_First_Item_Content_Test_RSS_10_DC_10_Description extends SimplePie_First_Item_Content_Test
{
	function data()
	{
		$this->data = 
'<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns="http://purl.org/rss/1.0/" xmlns:dc="http://purl.org/dc/elements/1.0/">
	<item>
		<dc:description>Item Description</dc:description>
	</item>
</rdf:RDF>';
	}
	
	function expected()
	{
		$this->expected = 'Item Description';
	}
}

?>