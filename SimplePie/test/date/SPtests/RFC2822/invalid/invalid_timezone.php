<?php

class SimplePie_Date_Test_RFC2822_Invalid_Timezone extends SimplePie_Date_Test
{
	function data()
	{
		$this->data = 'Fri, 05 Nov 94 13:15:30 UTC';
	}
	
	function expected()
	{
		$this->expected = 784041330;
	}
}

?>