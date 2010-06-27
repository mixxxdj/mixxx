<?php

class SimplePie_Absolutize_Test_RFC3986_Normal_12 extends SimplePie_Absolutize_Test_RFC3986
{
	function data()
	{
		$this->data['relative'] = ';x';
	}
	
	function expected()
	{
		$this->expected = 'http://a/b/c/;x';
	}
}

?>