<?php

class SimplePie_Absolutize_Test_RFC3986 extends SimplePie_Absolutize_Test
{
	function SimplePie_Absolutize_Test_RFC3986()
	{
		// Ugly hack so it only applies to this and none of its children
		if (!is_subclass_of($this, 'SimplePie_Absolutize_Test_RFC3986'))
		{
			$this->test = false;
		}
		// Only call the parent constructor if it exists
		if (is_callable(array('parent', 'SimplePie_Absolutize_Test')))
		{
			parent::SimplePie_Absolutize_Test();
		}
	}
	
	function init()
	{
		$this->data['base'] = 'http://a/b/c/d;p?q';
	}
}

?>