<?php

class SimplePie_First_Item_Latitude_Test_RSS_092_Georss_Point extends SimplePie_First_Item_Latitude_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.92" xmlns:georss="http://www.georss.org/georss">
	<channel>
		<item>
			<georss:point>55.701 12.552</georss:point>
		</item>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 55.701;
	}
}

?>