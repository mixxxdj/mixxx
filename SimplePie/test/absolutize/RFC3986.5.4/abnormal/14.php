<?php

class SimplePie_Absolutize_Test_RFC3986_Abnormal_14 extends SimplePie_Absolutize_Test_RFC3986
{
	function data()
	{
		$this->data['relative'] = 'g;x=1/../y';
	}
	
	function expected()
	{
		$this->expected = 'http://a/b/c/y';
	}
}

?>