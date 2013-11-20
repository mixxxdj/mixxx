The skin.xsl file in this directory allows you to do XSL transform which converts a Mixxx skin.xml into HTML, to be viewed in a web browser. In plain english, it lets you preview a skin in your web browser so you don't have to restart Mixxx every time you make a change.

The XSL file can be used by running xsltproc like so:

    xsltproc skin.xsl skin.xml > skin.html

(Contributed by Dave Jarvis, Nov 4, 2007)
