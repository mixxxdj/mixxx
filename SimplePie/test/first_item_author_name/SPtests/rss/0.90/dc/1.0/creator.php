<?php

class SimplePie_First_Item_Author_Name_Test_RSS_090_DC_10_Creator extends SimplePie_First_Item_Author_Name_Test
{
	function data()
	{
		$this->data = 
'<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns="http://my.netscape.com/rdf/simple/0.9/" xmlns:dc="http://purl.org/dc/elements/1.0/">
	<item>
		<dc:creator>Item Author</dc:creator>
	</item>
</rdf:RDF>';
	}
	
	function expected()
	{
		$this->expected = 'Item Author';
	}
}

?>