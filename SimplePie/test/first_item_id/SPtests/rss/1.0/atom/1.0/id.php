<?php

class SimplePie_First_Item_ID_Test_RSS_10_Atom_10_ID extends SimplePie_First_Item_ID_Test
{
	function data()
	{
		$this->data = 
'<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns="http://purl.org/rss/1.0/" xmlns:a="http://www.w3.org/2005/Atom">
	<item>
		<a:id>http://example.com/</a:id>
	</item>
</rdf:RDF>';
	}
	
	function expected()
	{
		$this->expected = 'http://example.com/';
	}
}

?>