
#############################################
#                                           #
#  All the SVG files are in this directory  #
#                                           #
#############################################

1) The included SVG files in this skin, not only are the screen displayed "images", but they are the EDITABLE/WORKING files too, so they can be opened and customized by anyone else too. You just need to have INKSCAPE or any other similar software.

2) Avoid Corel Draw - Very powerfull! But it adds extra proprietary metadata in the SVG files, what results in huge file sizes and it converts shadows into bitmaps. Besides all that, most of those files just can´t be parsed with Mixxx, or any other "standard" opensource SVG parser.

3) Always export SVG files (from INKSCAPE) with all texts converted to paths!

4) Preparing graphics to handle all the possible color <Schemes> correctly (When system is processing colors, to generate a new color scheme, colors may blur borders (eg: Very visible at the buttons), this happens because that colors are overriding theirs (normally defined by 1px strokes) own limits, what makes the visuals very ugly and incorrect). To avoid this: A) Degradee the colors to 100% black (or white) just before it reaches your graphic maximum width or height, just 1px before, will be enought. B) Void set <VConst> values. 0 (zero) should always be "perfect".

SOFTWARE TOOLS - URLs:
1) INKSCAPE webpage: https://inkscape.org
2) About SVG at wikipedia: https://en.wikipedia.org/wiki/Scalable_Vector_Graphics