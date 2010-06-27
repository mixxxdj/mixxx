<?php

class SimplePie_First_Item_Permalink_Test_RSS_10_Link extends SimplePie_First_Item_Permalink_Test
{
	function data()
	{
		$this->data = 
'<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns="http://purl.org/rss/1.0/">
	<item>
		<link>http://example.com/</link>
	</item>
</rdf:RDF>';
	}
	
	function expected()
	{
		$this->expected = 'http://example.com/';
	}
}

?>