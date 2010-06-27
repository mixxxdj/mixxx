<?php

class SimplePie_Absolutize_Test_RFC3986_Abnormal_18 extends SimplePie_Absolutize_Test_RFC3986
{
	function data()
	{
		$this->data['relative'] = 'g#s/../x';
	}
	
	function expected()
	{
		$this->expected = 'http://a/b/c/g#s/../x';
	}
}

?>