<?php

class SimplePie_First_Item_Title_Test_RSS_10_Atom_03_Title extends SimplePie_First_Item_Title_Test
{
	function data()
	{
		$this->data = 
'<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns="http://purl.org/rss/1.0/" xmlns:a="http://purl.org/atom/ns#">
	<item>
		<a:title>Item Title</a:title>
	</item>
</rdf:RDF>';
	}
	
	function expected()
	{
		$this->expected = 'Item Title';
	}
}

?>