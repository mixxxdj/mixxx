<?php

class SimplePie_Feed_Image_Title_Test_RSS_10_DC_11_Title extends SimplePie_Feed_Image_Title_Test
{
	function data()
	{
		$this->data = 
'<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns="http://purl.org/rss/1.0/" xmlns:dc="http://purl.org/dc/elements/1.1/">
	<image>
		<dc:title>Image Title</dc:title>
	</image>
</rdf:RDF>';
	}
	
	function expected()
	{
		$this->expected = 'Image Title';
	}
}

?>