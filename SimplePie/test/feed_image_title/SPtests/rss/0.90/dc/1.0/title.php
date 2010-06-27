<?php

class SimplePie_Feed_Image_Title_Test_RSS_090_DC_10_Title extends SimplePie_Feed_Image_Title_Test
{
	function data()
	{
		$this->data = 
'<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns="http://my.netscape.com/rdf/simple/0.9/" xmlns:dc="http://purl.org/dc/elements/1.0/">
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