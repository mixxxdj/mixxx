<?php

class SimplePie_Absolutize_Test_Bug_691_Test_0 extends SimplePie_Absolutize_Test
{
	function data()
	{
		$this->data['base'] = 'http://a/b/c';
		$this->data['relative'] = 'zero://a/b/c';
	}
	
	function expected()
	{
		$this->expected = 'zero://a/b/c';
	}
}

?>