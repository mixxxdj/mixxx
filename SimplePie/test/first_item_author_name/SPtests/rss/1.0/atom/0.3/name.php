<?php

class SimplePie_First_Item_Author_Name_Test_RSS_10_Atom_03_Name extends SimplePie_First_Item_Author_Name_Test
{
	function data()
	{
		$this->data = 
'<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns="http://purl.org/rss/1.0/" xmlns:a="http://purl.org/atom/ns#">
	<item>
		<a:author>
			<a:name>Item Author</a:name>
		</a:author>
	</item>
</rdf:RDF>';
	}
	
	function expected()
	{
		$this->expected = 'Item Author';
	}
}

?>