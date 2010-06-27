/**********************************************************
Sleight
(c) 2001, Aaron Boodman
http://www.youngpup.net
**********************************************************/

if (navigator.platform == "Win32" && navigator.appName == "Microsoft Internet Explorer" && window.attachEvent)
{
	document.writeln('<style type="text/css">img { visibility:hidden; } </style>');
	window.attachEvent("onload", fnLoadPngs);
}

function fnLoadPngs()
{
	var rslt = navigator.appVersion.match(/MSIE (\d+\.\d+)/, '');
	var itsAllGood = (rslt != null && Number(rslt[1]) >= 5.5);

	for (var i = document.images.length - 1, img = null; (img = document.images[i]); i--)
	{
		if (itsAllGood && img.src.match(/\.png$/i) != null)
		{
			var src = img.src;
			var div = document.createElement("DIV");
			div.style.filter = "progid:DXImageTransform.Microsoft.AlphaImageLoader(src='" + src + "', sizing='scale')"
			div.style.width = img.width + "px";
			div.style.height = img.height + "px";
			img.replaceNode(div);
		}
		img.style.visibility = "visible";
	}
}
