var yanone_kaffeesatz = {
	src: './for_the_demo/yanone-kaffeesatz-bold.swf'
};

var lucida_grande = {
	src: './for_the_demo/lucida-grande-bold.swf'
};

sIFR.activate(yanone_kaffeesatz);
//sIFR.activate(lucida_grande);

sIFR.replace(yanone_kaffeesatz, {
//sIFR.replace(lucida_grande, {

	selector: 'h3.header',
	wmode: 'transparent',
	css: {
		'.sIFR-root': {
			'text-align': 'center',
			'color': '#000000',
			'font-weight': 'bold',
			'background-color': '#EEFFEE',

			'font-size': '50px', // For Yanone Kaffeesatz
			//'font-size': '40px', // For Lucida Grande

			'letter-spacing': '0' // For Yanone Kaffeesatz
			//'letter-spacing': '-4' // For Lucida Grande

		},
		'a': {
			'text-decoration': 'none',
			'color': '#000000'
		},
		'a:hover': {
			'text-decoration': 'none',
			'color': '#666666'
		}
	}
});
