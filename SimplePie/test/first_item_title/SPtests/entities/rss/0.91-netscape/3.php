<?php

class SimplePie_First_Item_Title_Test_RSS_091_Netscape_Title_3 extends SimplePie_First_Item_Title_Test
{
	function data()
	{
		$this->data = 
'<!DOCTYPE rss SYSTEM "http://my.netscape.com/publish/formats/rss-0.91.dtd">
<rss version="0.91">
	<channel>
		<item>
			<title>This <![CDATA[&amp;]]>amp; this</title>
		</item>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'This &amp;amp;amp; this';
	}
}

?>