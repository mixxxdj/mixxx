<?php

class SimplePie_Feed_Copyright_Test_RSS_090_Atom_10_Rights extends SimplePie_Feed_Copyright_Test
{
	function data()
	{
		$this->data = 
'<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns="http://my.netscape.com/rdf/simple/0.9/" xmlns:a="http://www.w3.org/2005/Atom">
	<channel>
		<a:rights>Example Copyright Information</a:rights>
	</channel>
</rdf:RDF>';
	}
	
	function expected()
	{
		$this->expected = 'Example Copyright Information';
	}
}

?>