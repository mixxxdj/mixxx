<?php

class SimplePie_First_Item_Description_Test_RSS_10_Atom_10_Summary extends SimplePie_First_Item_Description_Test
{
	function data()
	{
		$this->data = 
'<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns="http://purl.org/rss/1.0/" xmlns:a="http://www.w3.org/2005/Atom">
	<item>
		<a:summary>Item Description</a:summary>
	</item>
</rdf:RDF>';
	}
	
	function expected()
	{
		$this->expected = 'Item Description';
	}
}

?>