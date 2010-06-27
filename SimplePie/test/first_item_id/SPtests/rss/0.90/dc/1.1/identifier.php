<?php

class SimplePie_First_Item_ID_Test_RSS_090_DC_11_Identifier extends SimplePie_First_Item_ID_Test
{
	function data()
	{
		$this->data = 
'<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns="http://my.netscape.com/rdf/simple/0.9/" xmlns:dc="http://purl.org/dc/elements/1.1/">
	<item>
		<dc:identifier>http://example.com/</dc:identifier>
	</item>
</rdf:RDF>';
	}
	
	function expected()
	{
		$this->expected = 'http://example.com/';
	}
}

?>