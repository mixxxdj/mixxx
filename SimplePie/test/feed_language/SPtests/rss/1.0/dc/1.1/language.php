<?php

class SimplePie_Feed_Language_Test_RSS_10_DC_11_Language extends SimplePie_Feed_Language_Test
{
	function data()
	{
		$this->data = 
'<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns="http://purl.org/rss/1.0/" xmlns:dc="http://purl.org/dc/elements/1.1/">
	<channel>
		<dc:language>en-GB</dc:language>
	</channel>
</rdf:RDF>';
	}
	
	function expected()
	{
		$this->expected = 'en-GB';
	}
}

?>