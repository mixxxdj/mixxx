<?php

class SimplePie_Absolutize_Test_Bug_691_Test_7 extends SimplePie_Absolutize_Test
{
	function data()
	{
		$this->data['base'] = 'http://a/b/c?0';
		$this->data['relative'] = 'd';
	}
	
	function expected()
	{
		$this->expected = 'http://a/b/d';
	}
}

?>