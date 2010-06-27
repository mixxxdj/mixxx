<?php

class SimplePie_First_Item_Description_Test_RSS_090_Atom_10_Content extends SimplePie_First_Item_Description_Test
{
	function data()
	{
		$this->data = 
'<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns="http://my.netscape.com/rdf/simple/0.9/" xmlns:a="http://www.w3.org/2005/Atom">
	<item>
		<a:content>Item Description</a:content>
	</item>
</rdf:RDF>';
	}
	
	function expected()
	{
		$this->expected = 'Item Description';
	}
}

?>