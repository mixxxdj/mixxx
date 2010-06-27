<?php

class SimplePie_Date_Test_RFC2822_44 extends SimplePie_Date_Test
{
	function data()
	{
		$this->data = 'Fri(day), 05 Nov(ember) 94 13:15:30 A';
	}
	
	function expected()
	{
		$this->expected = 784041330;
	}
}

?>