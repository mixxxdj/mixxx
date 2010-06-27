<?php

class SimplePie_First_Item_Title_Test_RSS_10_Title_3 extends SimplePie_First_Item_Title_Test
{
	function data()
	{
		$this->data = 
'<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns="http://purl.org/rss/1.0/">
	<item>
		<title>This <![CDATA[&amp;]]>amp; this</title>
	</item>
</rdf:RDF>';
	}
	
	function expected()
	{
		$this->expected = 'This &amp;amp;amp; this';
	}
}

?>