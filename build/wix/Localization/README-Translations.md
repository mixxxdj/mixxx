
Transifex doesn't handle wxl files.
So we convert the english one to a pot file and convert back the translated po files to wxl using
https://github.com/sblaisot/wxl-po-tools

Updating source pot
-------------------

`build/wix/Localization$ /path/to/wxl2pot.py -l Language mixxx_en-us.wxl po/mixxx.pot`

Updating translations from transifex
------------------------------------

First, pull new translations from transifex
`$ tx pull -r mixxxdj-windows-installer.mixxxpot -a`

Then, rebuils wxl files from po files
```
$ cd build/wix/Localization
build/wix/Localization$ for i in po/*.po; do destfile=${i#po/}; /path/to/po2wxl.py -l Language -f -p 60 $i mixxx_${destfile%.po}.wxl; done
```

explainations for the above line :
For earch po file in the po subdirectory, call po2wxl to transform it to wxl file with the following options :  
`-l Language` add a string with Id Language containing the auto-determined LCID  
`-f` Overwrite the wxl file if it already exists
`-p 60` don't transform po files if less than 60% of strings are translated