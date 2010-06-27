<?php

class SimplePie_Feed_Category_Label_Test_RSS_092_Atom_10_Category_Term extends SimplePie_Feed_Category_Label_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.92" xmlns:a="http://www.w3.org/2005/Atom">
	<channel>
		<a:category term="Feed Category"/>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'Feed Category';
	}
}

?>