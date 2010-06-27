<?php

class SimplePie_First_Item_Content_Test_RSS_090_Atom_03_Summary extends SimplePie_First_Item_Content_Test
{
	function data()
	{
		$this->data = 
'<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns="http://my.netscape.com/rdf/simple/0.9/" xmlns:a="http://purl.org/atom/ns#">
	<item>
		<a:summary>Item Description</a:summary>
	</item>
</rdf:RDF>';
	}
	
	function expected()
	{
		$this->expected = 'Item Description';
	}
}

?>