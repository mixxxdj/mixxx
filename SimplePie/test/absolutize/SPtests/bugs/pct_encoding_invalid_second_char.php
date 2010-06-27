<?php

class SimplePie_Absolutize_Test_Bug_Pct_Encoding_Invalid_Second_Char extends SimplePie_Absolutize_Test
{
	function data()
	{
		$this->data['base'] = 'http://a/b/c/d';
		$this->data['relative'] = 'f%0o';
	}
	
	function expected()
	{
		$this->expected = 'http://a/b/c/f%250o';
	}
}

?>