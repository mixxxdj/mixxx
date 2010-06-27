<?php

class SimplePie_iTunesRSS_Channel_Block_Reverse_Test_RSS_20 extends SimplePie_iTunesRSS_Channel_Block_Test
{
	function data()
	{
		$this->data = 
'<rss xmlns:itunes="http://www.itunes.com/DTDs/Podcast-1.0.dtd">
	<channel>
		<itunes:block>no</itunes:block>
		<item>
			<enclosure url="http://test.com/test.mp3" />
		</item>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'allow';
	}
}

?>