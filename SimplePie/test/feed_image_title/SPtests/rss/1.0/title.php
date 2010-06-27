<?php

class SimplePie_Feed_Image_Title_Test_RSS_10_Title extends SimplePie_Feed_Image_Title_Test
{
	function data()
	{
		$this->data = 
'<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns="http://purl.org/rss/1.0/">
	<image>
		<title>Image Title</title>
	</image>
</rdf:RDF>';
	}
	
	function expected()
	{
		$this->expected = 'Image Title';
	}
}

?>