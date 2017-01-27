
Transifex doesn't handle wxl files.
So we convert the english one to a pot file and convert back the translated po files to wxl using
https://github.com/sblaisot/wxl-po-tools

Updating source pot
===================

`build/wix/Localization$ /path/to/wxl2pot.py -l Language mixxx_en-us.wxl po/mixxx.pot`

Updating translations from Transifex
======================================

First, pull new translations from Transifex

`$ tx pull -r mixxxdj-windows-installer.mixxxpot -a --minimum-perc=60`

Then, rebuild wxl files from po files

```
$ cd build/wix/Localization
build/wix/Localization$ for i in po/*.po; do destfile=${i#po/}; /path/to/po2wxl.py -l Language -f -p 60 $i mixxx_${destfile%.po}.wxl; done
```

Explanations for the above line :
For each po file in the po subdirectory, call po2wxl to transform it to wxl file with the following options :
* `-l Language` add a string with Id Language containing the auto-determined LCID
* `-f` Overwrite the wxl file if it already exists
* `-p 60` don't transform po files if less than 60% of strings are translated

NOTES
=====

Adding new languages
--------------------

You can't add new languages from Transifex that are not in wix toolset translation, or package generation will not work.
List of available (base) translations of wixUI is here: http://wixtoolset.org/documentation/manual/v3/wixui/wixui_localization.html

Bad characters
--------------

Sometimes, you can face the following error message after pulling translations from Transifex:

```path\to\mixxx\build\wix\mixxx.wxs(67) : error LGHT0311 : A string was provided with characters that are not available in the specified database code page '1250'. Either change these characters to ones that exist in the database's code page, or update the database's code page by modifying one of the following attributes: Product/@Codepage, Module/@Codepage, Patch/@Codepage, PatchCreation/@Codepage, or WixLocalization/@Codepage.```

This is because a translation you pulled contain a character that is not part of the default codepage used by wix for that language. This happens sometimes with UTF-8 encoded translations from Transifex.
In this case, you need to replace the UTF-8 or UTF-16 char in the translation with the equivalent char in the codepage.

This has happened in the past for the ro-RO language and the following characters:

| faulty char (error) | replacement (OK) |
|:---:|:--:|
| ț   | ţ  |
| ș   | ş  |
