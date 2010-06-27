<?php

class SimplePie_Feed_Image_URL_Test_RSS_10_URL extends SimplePie_Feed_Image_URL_Test
{
	function data()
	{
		$this->data = 
'<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns="http://purl.org/rss/1.0/">
	<image>
		<url>http://example.com/</url>
	</image>
</rdf:RDF>';
	}
	
	function expected()
	{
		$this->expected = 'http://example.com/';
	}
}

?>