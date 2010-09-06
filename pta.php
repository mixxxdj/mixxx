<!doctype html public 
  "-//W3C//DTD HTML 4.0//EN" 
  "http://www.w3.org/TR/REC-html40/strict.dtd"> 
 
<html> 
 
  <head> 
 
    <title></title> 
    <script type="text/javascript">
        /** Google Analytics. Must be above where _trackPageview is used.*/
        var _gaq = _gaq || [];
        _gaq.push(['_setAccount', 'UA-12721335-1']);
        //_gaq.push(['_setDomainName', 'none']);
        //_gaq.push(['_setAllowHash', 'false']);
        _gaq.push(['_trackPageview']);

        (function() {
            var ga = document.createElement('script'); ga.type = 'text/javascript'; ga.async = true;
            ga.src = ('https:' == document.location.protocol ? 'https://ssl' : 'http://www') + '.google-analytics.com/ga.js';
            var s = document.getElementsByTagName('script')[0]; s.parentNode.insertBefore(ga, s);
        })();
        
        var category = "<?php echo $_GET["c"]; ?>";
        var action = "<?php echo $_GET["a"]; ?>";
        var label = "<?php echo $_GET["l"]; ?>";
        
        //Useful for debugging to make sure the iframe is being loaded.
        //alert(action);
                        
        window.onload = function() {
            _gaq.push(['_trackEvent', 
                       category, 
                       action, 
                       label]);
                   
        }
    
    </script>
  </head> 
 
  <body> 
 
  </body> 
 
</html>
