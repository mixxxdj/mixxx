<?php

class SimplePie_First_Item_Title_Test_RSS_090_DC_10_Title extends SimplePie_First_Item_Title_Test
{
	function data()
	{
		$this->data = 
'<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns="http://my.netscape.com/rdf/simple/0.9/" xmlns:dc="http://purl.org/dc/elements/1.0/">
	<item>
		<dc:title>Item Title</dc:title>
	</item>
</rdf:RDF>';
	}
	
	function expected()
	{
		$this->expected = 'Item Title';
	}
}

?>