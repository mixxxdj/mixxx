<?php

class SimplePie_First_Item_Date_Test_RSS_10_Atom_10_Updated extends SimplePie_First_Item_Date_Test
{
	function data()
	{
		$this->data = 
'<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns="http://purl.org/rss/1.0/" xmlns:a="http://www.w3.org/2005/Atom">
	<item>
		<a:updated>2007-01-11T16:00:00Z</a:updated>
	</item>
</rdf:RDF>';
	}
	
	function expected()
	{
		$this->expected = 1168531200;
	}
}

?>