<?php

class SimplePie_Absolutize_Test_Bug_691_Test_4 extends SimplePie_Absolutize_Test
{
	function data()
	{
		$this->data['base'] = 'http://a/b/c';
		$this->data['relative'] = '#0';
	}
	
	function expected()
	{
		$this->expected = 'http://a/b/c#0';
	}
}

?>