<?php

class SimplePie_Feed_Category_Label_Test_RSS_090_Atom_10_Category_Label extends SimplePie_Feed_Category_Label_Test
{
	function data()
	{
		$this->data = 
'<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns="http://my.netscape.com/rdf/simple/0.9/" xmlns:a="http://www.w3.org/2005/Atom">
	<channel>
		<a:category label="Feed Category"/>
	</channel>
</rdf:RDF>';
	}
	
	function expected()
	{
		$this->expected = 'Feed Category';
	}
}

?>