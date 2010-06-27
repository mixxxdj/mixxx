<?php

class SimplePie_Date_Test_RFC2822_Full_Name_Of_Day extends SimplePie_Date_Test
{
	function data()
	{
		$this->data = 'Friday, 05 Nov 94 13:15:30 GMT';
	}
	
	function expected()
	{
		$this->expected = 784041330;
	}
}

?>