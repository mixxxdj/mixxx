<?php

class SimplePie_First_Item_Date_Test_RSS_090_Atom_03_Created extends SimplePie_First_Item_Date_Test
{
	function data()
	{
		$this->data = 
'<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns="http://my.netscape.com/rdf/simple/0.9/" xmlns:a="http://purl.org/atom/ns#">
	<item>
		<a:created>2007-01-11T16:00:00Z</a:created>
	</item>
</rdf:RDF>';
	}
	
	function expected()
	{
		$this->expected = 1168531200;
	}
}

?>