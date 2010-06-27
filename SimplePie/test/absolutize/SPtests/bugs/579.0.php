<?php

class SimplePie_Absolutize_Test_Bug_579_Test_0 extends SimplePie_Absolutize_Test
{
	function data()
	{
		$this->data['base'] = 'http://a/b/';
		$this->data['relative'] = "b\x0Ac";
	}
	
	function expected()
	{
		$this->expected = 'http://a/b/b%0Ac';
	}
}

?>