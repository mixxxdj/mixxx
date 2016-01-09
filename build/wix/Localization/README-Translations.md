
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
build/wix/Localization$ for i in po/*.po; do destfile=${i#po/}; /path/to/po2wxl.py -l Language $i mixxx_${destfile%.po}.wxl; done
```
