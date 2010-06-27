<?php

class SimplePie_Feed_Title_Test_RSS_10_Atom_03_Title extends SimplePie_Feed_Title_Test
{
	function data()
	{
		$this->data = 
'<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns="http://purl.org/rss/1.0/" xmlns:a="http://purl.org/atom/ns#">
	<channel>
		<a:title>Feed Title</a:title>
	</channel>
</rdf:RDF>';
	}
	
	function expected()
	{
		$this->expected = 'Feed Title';
	}
}

?>