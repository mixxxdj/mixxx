#!/bin/bash
git filter-branch -f --env-filter '
if [ "$GIT_AUTHOR_NAME" = "" ] ; then
  if [ "$GIT_AUTHOR_EMAIL" = "" ]; then
    GIT_AUTHOR_NAME="unknown" ;
    GIT_AUTHOR_EMAIL="unknown@unknown.com" ;
  else
    GIT_AUTHOR_NAME=$GIT_AUTHOR_EMAIL ;
  fi ;
  export GIT_AUTHOR_NAME ;
fi ;
if [ "$GIT_COMMITTER_NAME" = "" ] ; then
  if [ "$GIT_COMMITTER_EMAIL" = "" ]; then
    GIT_COMMITTER_NAME=$GIT_AUTHOR_NAME;
    GIT_COMMITTER_EMAIL=$GIT_AUTHOR_EMAIL;
  else
    GIT_COMMITTER_NAME=$GIT_COMMITTER_EMAIL;
  fi;
  export GIT_COMMITTER_NAME;
  export GIT_COMMITTER_EMAIL;
fi;
if [ "$GIT_AUTHOR_EMAIL" = "mavrinac@gmail.com" ]; then
  GIT_AUTHOR_EMAIL="mavrinac@gmail.com";
  GIT_AUTHOR_NAME="Aaron Mavrinac";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "mavrinac@gmail.com" ]; then
  GIT_COMMITTER_EMAIL="mavrinac@gmail.com";
  GIT_COMMITTER_NAME="Aaron Mavrinac";
fi;
if [ "$GIT_AUTHOR_NAME" = "adam_d" ]; then
  GIT_AUTHOR_EMAIL="adamd@mixxx.org";
  GIT_AUTHOR_NAME="Adam Davison";
fi;
if [ "$GIT_COMMITTER_NAME" = "adam_d" ]; then
  GIT_COMMITTER_EMAIL="adamd@mixxx.org";
  GIT_COMMITTER_NAME="Adam Davison";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "adamdavison@gmail.com" ]; then
  GIT_AUTHOR_EMAIL="adamd@mixxx.org";
  GIT_AUTHOR_NAME="Adam Davison";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "adamdavison@gmail.com" ]; then
  GIT_COMMITTER_EMAIL="adamd@mixxx.org";
  GIT_COMMITTER_NAME="Adam Davison";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "shetyeakash@gmail.com" ]; then
  GIT_AUTHOR_EMAIL="shetyeakash@gmail.com";
  GIT_AUTHOR_NAME="Akash Shetye";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "shetyeakash@gmail.com" ]; then
  GIT_COMMITTER_EMAIL="shetyeakash@gmail.com";
  GIT_COMMITTER_NAME="Akash Shetye";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "alb@mixxx-builder" ]; then
  GIT_AUTHOR_EMAIL="alberts@mixxx.org";
  GIT_AUTHOR_NAME="Albert Santoni";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "alb@mixxx-builder" ]; then
  GIT_COMMITTER_EMAIL="alberts@mixxx.org";
  GIT_COMMITTER_NAME="Albert Santoni";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "alb@mixxx-ubuntu810-32" ]; then
  GIT_AUTHOR_EMAIL="alberts@mixxx.org";
  GIT_AUTHOR_NAME="Albert Santoni";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "alb@mixxx-ubuntu810-32" ]; then
  GIT_COMMITTER_EMAIL="alberts@mixxx.org";
  GIT_COMMITTER_NAME="Albert Santoni";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "alberts@mixxx.org" ]; then
  GIT_AUTHOR_EMAIL="alberts@mixxx.org";
  GIT_AUTHOR_NAME="Albert Santoni";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "alberts@mixxx.org" ]; then
  GIT_COMMITTER_EMAIL="alberts@mixxx.org";
  GIT_COMMITTER_NAME="Albert Santoni";
fi;
if [ "$GIT_AUTHOR_NAME" = "Albert Santoni alberts@mixxx.org" ]; then
  GIT_AUTHOR_EMAIL="alberts@mixxx.org";
  GIT_AUTHOR_NAME="Albert Santoni";
fi;
if [ "$GIT_COMMITTER_NAME" = "Albert Santoni alberts@mixxx.org" ]; then
  GIT_COMMITTER_EMAIL="alberts@mixxx.org";
  GIT_COMMITTER_NAME="Albert Santoni";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "alb@jupiter.home" ]; then
  GIT_AUTHOR_EMAIL="alberts@mixxx.org";
  GIT_AUTHOR_NAME="Albert Santoni";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "alb@jupiter.home" ]; then
  GIT_COMMITTER_EMAIL="alberts@mixxx.org";
  GIT_COMMITTER_NAME="Albert Santoni";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "alb@w052-215.wireless.uvic.ca" ]; then
  GIT_AUTHOR_EMAIL="alberts@mixxx.org";
  GIT_AUTHOR_NAME="Albert Santoni";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "alb@w052-215.wireless.uvic.ca" ]; then
  GIT_COMMITTER_EMAIL="alberts@mixxx.org";
  GIT_COMMITTER_NAME="Albert Santoni";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "alb@w143-126.wireless.uvic.ca" ]; then
  GIT_AUTHOR_EMAIL="alberts@mixxx.org";
  GIT_AUTHOR_NAME="Albert Santoni";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "alb@w143-126.wireless.uvic.ca" ]; then
  GIT_COMMITTER_EMAIL="alberts@mixxx.org";
  GIT_COMMITTER_NAME="Albert Santoni";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "asantoni@atrius" ]; then
  GIT_AUTHOR_EMAIL="alberts@mixxx.org";
  GIT_AUTHOR_NAME="Albert Santoni";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "asantoni@atrius" ]; then
  GIT_COMMITTER_EMAIL="alberts@mixxx.org";
  GIT_COMMITTER_NAME="Albert Santoni";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "alex@1stleg.com" ]; then
  GIT_AUTHOR_EMAIL="alex@1stleg.com";
  GIT_AUTHOR_NAME="Alex Barker";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "alex@1stleg.com" ]; then
  GIT_COMMITTER_EMAIL="alex@1stleg.com";
  GIT_COMMITTER_NAME="Alex Barker";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "d00guan@gmail.com" ]; then
  GIT_AUTHOR_EMAIL="d00guan@gmail.com";
  GIT_AUTHOR_NAME="Anders Gunnarsson";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "d00guan@gmail.com" ]; then
  GIT_COMMITTER_EMAIL="d00guan@gmail.com";
  GIT_COMMITTER_NAME="Anders Gunnarsson";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "bkgood@gmail.com" ]; then
  GIT_AUTHOR_EMAIL="bkgood@mixxx.org";
  GIT_AUTHOR_NAME="Bill Good";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "bkgood@gmail.com" ]; then
  GIT_COMMITTER_EMAIL="bkgood@mixxx.org";
  GIT_COMMITTER_NAME="Bill Good";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "Box@Box-PC" ]; then
  GIT_AUTHOR_EMAIL="alberts@mixxx.org";
  GIT_AUTHOR_NAME="Albert Santoni";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "Box@Box-PC" ]; then
  GIT_COMMITTER_EMAIL="alberts@mixxx.org";
  GIT_COMMITTER_NAME="Albert Santoni";
fi;
if [ "$GIT_AUTHOR_NAME" = "ctaf" ]; then
  GIT_AUTHOR_EMAIL="ctaf42@gmail.com";
  GIT_AUTHOR_NAME="Cedric GESTES";
fi;
if [ "$GIT_COMMITTER_NAME" = "ctaf" ]; then
  GIT_COMMITTER_EMAIL="ctaf42@gmail.com";
  GIT_COMMITTER_NAME="Cedric GESTES";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "daniel@lindenfelser.de" ]; then
  GIT_AUTHOR_EMAIL="daniel@lindenfelser.de";
  GIT_AUTHOR_NAME="Daniel Lindenfelser";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "daniel@lindenfelser.de" ]; then
  GIT_COMMITTER_EMAIL="daniel@lindenfelser.de";
  GIT_COMMITTER_NAME="Daniel Lindenfelser";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "daschuer@gmx.de" ]; then
  GIT_AUTHOR_EMAIL="daschuer@mixxx.org";
  GIT_AUTHOR_NAME="Daniel Schürmann";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "daschuer@gmx.de" ]; then
  GIT_COMMITTER_EMAIL="daschuer@mixxx.org";
  GIT_COMMITTER_NAME="Daniel Schürmann";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "daschuer@gmx.de" ]; then
  GIT_AUTHOR_EMAIL="daschuer@mixxx.org";
  GIT_AUTHOR_NAME="Daniel Schürmann";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "daschuer@gmx.de" ]; then
  GIT_COMMITTER_EMAIL="daschuer@mixxx.org";
  GIT_COMMITTER_NAME="Daniel Schürmann";
fi;
if [ "$GIT_AUTHOR_NAME" = "emaix" ]; then
  GIT_AUTHOR_EMAIL="tuuresairon@gmail.com";
  GIT_AUTHOR_NAME="Pawel Bartkiewicz";
fi;
if [ "$GIT_COMMITTER_NAME" = "emaix" ]; then
  GIT_COMMITTER_EMAIL="tuuresairon@gmail.com";
  GIT_COMMITTER_NAME="Pawel Bartkiewicz";
fi;
if [ "$GIT_AUTHOR_NAME" = "eythian" ]; then
  GIT_AUTHOR_EMAIL="robin@kallisti.net.nz";
  GIT_AUTHOR_NAME="Robin Sheat";
fi;
if [ "$GIT_COMMITTER_NAME" = "eythian" ]; then
  GIT_COMMITTER_EMAIL="robin@kallisti.net.nz";
  GIT_COMMITTER_NAME="Robin Sheat";
fi;
if [ "$GIT_AUTHOR_NAME" = "gamegod" ]; then
  GIT_AUTHOR_EMAIL="alberts@mixxx.org";
  GIT_AUTHOR_NAME="Albert Santoni";
fi;
if [ "$GIT_COMMITTER_NAME" = "gamegod" ]; then
  GIT_COMMITTER_EMAIL="alberts@mixxx.org";
  GIT_COMMITTER_NAME="Albert Santoni";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "ged@eeepc901" ]; then
  GIT_AUTHOR_EMAIL="garth@mixxx.org";
  GIT_AUTHOR_NAME="Garth Dahlstrom";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "ged@eeepc901" ]; then
  GIT_COMMITTER_EMAIL="garth@mixxx.org";
  GIT_COMMITTER_NAME="Garth Dahlstrom";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "ged@garth-dahlstroms-macbook.local" ]; then
  GIT_AUTHOR_EMAIL="garth@mixxx.org";
  GIT_AUTHOR_NAME="Garth Dahlstrom";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "ged@garth-dahlstroms-macbook.local" ]; then
  GIT_COMMITTER_EMAIL="garth@mixxx.org";
  GIT_COMMITTER_NAME="Garth Dahlstrom";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "ged@gmacbook" ]; then
  GIT_AUTHOR_EMAIL="garth@mixxx.org";
  GIT_AUTHOR_NAME="Garth Dahlstrom";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "ged@gmacbook" ]; then
  GIT_COMMITTER_EMAIL="garth@mixxx.org";
  GIT_COMMITTER_NAME="Garth Dahlstrom";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "ged@gMacBook" ]; then
  GIT_AUTHOR_EMAIL="garth@mixxx.org";
  GIT_AUTHOR_NAME="Garth Dahlstrom";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "ged@gMacBook" ]; then
  GIT_COMMITTER_EMAIL="garth@mixxx.org";
  GIT_COMMITTER_NAME="Garth Dahlstrom";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "ged@gMacBook.local" ]; then
  GIT_AUTHOR_EMAIL="garth@mixxx.org";
  GIT_AUTHOR_NAME="Garth Dahlstrom";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "ged@gMacBook.local" ]; then
  GIT_COMMITTER_EMAIL="garth@mixxx.org";
  GIT_COMMITTER_NAME="Garth Dahlstrom";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "ged@quadcore" ]; then
  GIT_AUTHOR_EMAIL="garth@mixxx.org";
  GIT_AUTHOR_NAME="Garth Dahlstrom";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "ged@quadcore" ]; then
  GIT_COMMITTER_EMAIL="garth@mixxx.org";
  GIT_COMMITTER_NAME="Garth Dahlstrom";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "ged@eeepc-901" ]; then
  GIT_AUTHOR_EMAIL="garth@mixxx.org";
  GIT_AUTHOR_NAME="Garth Dahlstrom";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "ged@eeepc-901" ]; then
  GIT_COMMITTER_EMAIL="garth@mixxx.org";
  GIT_COMMITTER_NAME="Garth Dahlstrom";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "mathieg2@mathieg2-sony32" ]; then
  GIT_AUTHOR_EMAIL="graeme@salsaholics.org";
  GIT_AUTHOR_NAME="Graeme Mathieson";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "mathieg2@mathieg2-sony32" ]; then
  GIT_COMMITTER_EMAIL="graeme@salsaholics.org";
  GIT_COMMITTER_NAME="Graeme Mathieson";
fi;
if [ "$GIT_AUTHOR_NAME" = "ikdev" ]; then
  GIT_AUTHOR_EMAIL="kossyki@cs.tu-berlin.de";
  GIT_AUTHOR_NAME="Ingo Kossyk";
fi;
if [ "$GIT_COMMITTER_NAME" = "ikdev" ]; then
  GIT_COMMITTER_EMAIL="kossyki@cs.tu-berlin.de";
  GIT_COMMITTER_NAME="Ingo Kossyk";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "hile@iki.fi" ]; then
  GIT_AUTHOR_EMAIL="hile@iki.fi";
  GIT_AUTHOR_NAME="Ilkka Tuohela";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "hile@iki.fi" ]; then
  GIT_COMMITTER_EMAIL="hile@iki.fi";
  GIT_COMMITTER_NAME="Ilkka Tuohela";
fi;
if [ "$GIT_AUTHOR_NAME" = "ironstorm" ]; then
  GIT_AUTHOR_EMAIL="garth@mixxx.org";
  GIT_AUTHOR_NAME="Garth Dahlstrom";
fi;
if [ "$GIT_COMMITTER_NAME" = "ironstorm" ]; then
  GIT_COMMITTER_EMAIL="garth@mixxx.org";
  GIT_COMMITTER_NAME="Garth Dahlstrom";
fi;
if [ "$GIT_AUTHOR_NAME" = "jammin23" ]; then
  GIT_AUTHOR_EMAIL="ben@qolc.net";
  GIT_AUTHOR_NAME="Ben Wheeler";
fi;
if [ "$GIT_COMMITTER_NAME" = "jammin23" ]; then
  GIT_COMMITTER_EMAIL="ben@qolc.net";
  GIT_COMMITTER_NAME="Ben Wheeler";
fi;
if [ "$GIT_AUTHOR_NAME" = "jayorama" ]; then
  GIT_AUTHOR_EMAIL="romaninz@aruba.it";
  GIT_AUTHOR_NAME="Gianluca Romanin";
fi;
if [ "$GIT_COMMITTER_NAME" = "jayorama" ]; then
  GIT_COMMITTER_EMAIL="romaninz@aruba.it";
  GIT_COMMITTER_NAME="Gianluca Romanin";
fi;
if [ "$GIT_AUTHOR_NAME" = "jonathan" ]; then
  GIT_AUTHOR_EMAIL="jonathan.costers@googlemail.com";
  GIT_AUTHOR_NAME="Jonathan Costers";
fi;
if [ "$GIT_COMMITTER_NAME" = "jonathan" ]; then
  GIT_COMMITTER_EMAIL="jonathan.costers@googlemail.com";
  GIT_COMMITTER_NAME="Jonathan Costers";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "raskolnikov@es.gnu.org" ]; then
  GIT_AUTHOR_EMAIL="raskolnikov@es.gnu.org";
  GIT_AUTHOR_NAME="Juan Pedro Bolívar Puente";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "raskolnikov@es.gnu.org" ]; then
  GIT_COMMITTER_EMAIL="raskolnikov@es.gnu.org";
  GIT_COMMITTER_NAME="Juan Pedro Bolívar Puente";
fi;
if [ "$GIT_AUTHOR_NAME" = "jus" ]; then
  GIT_AUTHOR_EMAIL="s.brandt@mixxx.org";
  GIT_AUTHOR_NAME="jus";
fi;
if [ "$GIT_COMMITTER_NAME" = "jus" ]; then
  GIT_COMMITTER_EMAIL="s.brandt@mixxx.org";
  GIT_COMMITTER_NAME="jus";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "https" ]; then
  GIT_AUTHOR_EMAIL="s.brandt@mixxx.org";
  GIT_AUTHOR_NAME="jus";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "https" ]; then
  GIT_COMMITTER_EMAIL="s.brandt@mixxx.org";
  GIT_COMMITTER_NAME="jus";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "https://launchpad.net/~jus" ]; then
  GIT_AUTHOR_EMAIL="s.brandt@mixxx.org";
  GIT_AUTHOR_NAME="jus";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "https://launchpad.net/~jus" ]; then
  GIT_COMMITTER_EMAIL="s.brandt@mixxx.org";
  GIT_COMMITTER_NAME="jus";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "jus@justmail.de" ]; then
  GIT_AUTHOR_EMAIL="s.brandt@mixxx.org";
  GIT_AUTHOR_NAME="jus";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "jus@justmail.de" ]; then
  GIT_COMMITTER_EMAIL="s.brandt@mixxx.org";
  GIT_COMMITTER_NAME="jus";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "jus@local" ]; then
  GIT_AUTHOR_EMAIL="s.brandt@mixxx.org";
  GIT_AUTHOR_NAME="jus";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "jus@local" ]; then
  GIT_COMMITTER_EMAIL="s.brandt@mixxx.org";
  GIT_COMMITTER_NAME="jus";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "jus@mbp" ]; then
  GIT_AUTHOR_EMAIL="s.brandt@mixxx.org";
  GIT_AUTHOR_NAME="jus";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "jus@mbp" ]; then
  GIT_COMMITTER_EMAIL="s.brandt@mixxx.org";
  GIT_COMMITTER_NAME="jus";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "s.brandt@mixxx.org" ]; then
  GIT_AUTHOR_EMAIL="s.brandt@mixxx.org";
  GIT_AUTHOR_NAME="jus";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "s.brandt@mixxx.org" ]; then
  GIT_COMMITTER_EMAIL="s.brandt@mixxx.org";
  GIT_COMMITTER_NAME="jus";
fi;
if [ "$GIT_AUTHOR_NAME" = "jus@ubuntu" ]; then
  GIT_AUTHOR_EMAIL="s.brandt@mixxx.org";
  GIT_AUTHOR_NAME="jus";
fi;
if [ "$GIT_COMMITTER_NAME" = "jus@ubuntu" ]; then
  GIT_COMMITTER_EMAIL="s.brandt@mixxx.org";
  GIT_COMMITTER_NAME="jus";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "jwc@jwcxz.com" ]; then
  GIT_AUTHOR_EMAIL="jwc@jwcxz.com";
  GIT_AUTHOR_NAME="JWC";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "jwc@jwcxz.com" ]; then
  GIT_COMMITTER_EMAIL="jwc@jwcxz.com";
  GIT_COMMITTER_NAME="JWC";
fi;
if [ "$GIT_AUTHOR_NAME" = "kenand" ]; then
  GIT_AUTHOR_EMAIL="ken@haste.dk";
  GIT_AUTHOR_NAME="Ken Haste Andersen";
fi;
if [ "$GIT_COMMITTER_NAME" = "kenand" ]; then
  GIT_COMMITTER_EMAIL="ken@haste.dk";
  GIT_COMMITTER_NAME="Ken Haste Andersen";
fi;
if [ "$GIT_AUTHOR_NAME" = "kousu" ]; then
  GIT_AUTHOR_EMAIL="nick@kousu.ca";
  GIT_AUTHOR_NAME="Nick Guenther";
fi;
if [ "$GIT_COMMITTER_NAME" = "kousu" ]; then
  GIT_COMMITTER_EMAIL="nick@kousu.ca";
  GIT_COMMITTER_NAME="Nick Guenther";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "kousu@splat" ]; then
  GIT_AUTHOR_EMAIL="nick@kousu.ca";
  GIT_AUTHOR_NAME="Nick Guenther";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "kousu@splat" ]; then
  GIT_COMMITTER_EMAIL="nick@kousu.ca";
  GIT_COMMITTER_NAME="Nick Guenther";
fi;
if [ "$GIT_AUTHOR_NAME" = "Launchpad Translations on behalf of mixxxdevelopers" ]; then
  GIT_AUTHOR_EMAIL="rosetta@launchpad.net";
  GIT_AUTHOR_NAME="Launchpad Translations";
fi;
if [ "$GIT_COMMITTER_NAME" = "Launchpad Translations on behalf of mixxxdevelopers" ]; then
  GIT_COMMITTER_EMAIL="rosetta@launchpad.net";
  GIT_COMMITTER_NAME="Launchpad Translations";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "madjester@voidwalker" ]; then
  GIT_AUTHOR_EMAIL="pwhelan@mixxx.org";
  GIT_AUTHOR_NAME="Phillip Whelan";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "madjester@voidwalker" ]; then
  GIT_COMMITTER_EMAIL="pwhelan@mixxx.org";
  GIT_COMMITTER_NAME="Phillip Whelan";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "maxime.bochon@gmail.com" ]; then
  GIT_AUTHOR_EMAIL="maxime.bochon@gmail.com";
  GIT_AUTHOR_NAME="Maxime Bochon";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "maxime.bochon@gmail.com" ]; then
  GIT_COMMITTER_EMAIL="maxime.bochon@gmail.com";
  GIT_COMMITTER_NAME="Maxime Bochon";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "kain88@640X4" ]; then
  GIT_AUTHOR_EMAIL="kain88@mixxx.org";
  GIT_AUTHOR_NAME="Max Linke";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "kain88@640X4" ]; then
  GIT_COMMITTER_EMAIL="kain88@mixxx.org";
  GIT_COMMITTER_NAME="Max Linke";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "kain88@640X4.kel.wh.lokal" ]; then
  GIT_AUTHOR_EMAIL="kain88@mixxx.org";
  GIT_AUTHOR_NAME="Max Linke";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "kain88@640X4.kel.wh.lokal" ]; then
  GIT_COMMITTER_EMAIL="kain88@mixxx.org";
  GIT_COMMITTER_NAME="Max Linke";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "kain88@mixxx.org" ]; then
  GIT_AUTHOR_EMAIL="kain88@mixxx.org";
  GIT_AUTHOR_NAME="Max Linke";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "kain88@mixxx.org" ]; then
  GIT_COMMITTER_EMAIL="kain88@mixxx.org";
  GIT_COMMITTER_NAME="Max Linke";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "kain88@Thinki" ]; then
  GIT_AUTHOR_EMAIL="kain88@mixxx.org";
  GIT_AUTHOR_NAME="Max Linke";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "kain88@Thinki" ]; then
  GIT_COMMITTER_EMAIL="kain88@mixxx.org";
  GIT_COMMITTER_NAME="Max Linke";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "mika@haulo.fi" ]; then
  GIT_AUTHOR_EMAIL="mika@haulo.fi";
  GIT_AUTHOR_NAME="Mika Haulo";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "mika@haulo.fi" ]; then
  GIT_COMMITTER_EMAIL="mika@haulo.fi";
  GIT_COMMITTER_NAME="Mika Haulo";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "mikoki@mikoki-laptop" ]; then
  GIT_AUTHOR_EMAIL="miko.kiiski@gmail.com";
  GIT_AUTHOR_NAME="Miko Kiiski";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "mikoki@mikoki-laptop" ]; then
  GIT_COMMITTER_EMAIL="miko.kiiski@gmail.com";
  GIT_COMMITTER_NAME="Miko Kiiski";
fi;
if [ "$GIT_AUTHOR_NAME" = "nategoofs" ]; then
  GIT_AUTHOR_EMAIL="prado.nathan@gmail.com";
  GIT_AUTHOR_NAME="Nathan Prado";
fi;
if [ "$GIT_COMMITTER_NAME" = "nategoofs" ]; then
  GIT_COMMITTER_EMAIL="prado.nathan@gmail.com";
  GIT_COMMITTER_NAME="Nathan Prado";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "neale@woozle.org" ]; then
  GIT_AUTHOR_EMAIL="neale@woozle.org";
  GIT_AUTHOR_NAME="Neale Pickett";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "neale@woozle.org" ]; then
  GIT_COMMITTER_EMAIL="neale@woozle.org";
  GIT_COMMITTER_NAME="Neale Pickett";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "kousu@guarana.csclub.uwaterloo.ca" ]; then
  GIT_AUTHOR_EMAIL="nick@kousu.ca";
  GIT_AUTHOR_NAME="Nick Guenther";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "kousu@guarana.csclub.uwaterloo.ca" ]; then
  GIT_COMMITTER_EMAIL="nick@kousu.ca";
  GIT_COMMITTER_NAME="Nick Guenther";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "nick _a_t_ k o u s u dot CA" ]; then
  GIT_AUTHOR_EMAIL="nick@kousu.ca";
  GIT_AUTHOR_NAME="Nick Guenther";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "nick _a_t_ k o u s u dot CA" ]; then
  GIT_COMMITTER_EMAIL="nick@kousu.ca";
  GIT_COMMITTER_NAME="Nick Guenther";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "nick_a_t_kousudotCA" ]; then
  GIT_AUTHOR_EMAIL="nick@kousu.ca";
  GIT_AUTHOR_NAME="Nick Guenther";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "nick_a_t_kousudotCA" ]; then
  GIT_COMMITTER_EMAIL="nick@kousu.ca";
  GIT_COMMITTER_NAME="Nick Guenther";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "nick@kousu.ca" ]; then
  GIT_AUTHOR_EMAIL="nick@kousu.ca";
  GIT_AUTHOR_NAME="Nick Guenther";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "nick@kousu.ca" ]; then
  GIT_COMMITTER_EMAIL="nick@kousu.ca";
  GIT_COMMITTER_NAME="Nick Guenther";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "owen@ywwg" ]; then
  GIT_AUTHOR_EMAIL="owilliams@mixxx.org";
  GIT_AUTHOR_NAME="Owen Williams";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "owen@ywwg" ]; then
  GIT_COMMITTER_EMAIL="owilliams@mixxx.org";
  GIT_COMMITTER_NAME="Owen Williams";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "owen@ywwg.com" ]; then
  GIT_AUTHOR_EMAIL="owilliams@mixxx.org";
  GIT_AUTHOR_NAME="Owen Williams";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "owen@ywwg.com" ]; then
  GIT_COMMITTER_EMAIL="owilliams@mixxx.org";
  GIT_COMMITTER_NAME="Owen Williams";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "owilliams@localhost" ]; then
  GIT_AUTHOR_EMAIL="owilliams@mixxx.org";
  GIT_AUTHOR_NAME="Owen Williams";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "owilliams@localhost" ]; then
  GIT_COMMITTER_EMAIL="owilliams@mixxx.org";
  GIT_COMMITTER_NAME="Owen Williams";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "owilliams@mixxx.org" ]; then
  GIT_AUTHOR_EMAIL="owilliams@mixxx.org";
  GIT_AUTHOR_NAME="Owen Williams";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "owilliams@mixxx.org" ]; then
  GIT_COMMITTER_EMAIL="owilliams@mixxx.org";
  GIT_COMMITTER_NAME="Owen Williams";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "owen-github@ywwg.com" ]; then
  GIT_AUTHOR_EMAIL="owilliams@mixxx.org";
  GIT_AUTHOR_NAME="Owen Williams";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "owen-github@ywwg.com" ]; then
  GIT_COMMITTER_EMAIL="owilliams@mixxx.org";
  GIT_COMMITTER_NAME="Owen Williams";
fi;
if [ "$GIT_AUTHOR_NAME" = "pegasusrpg" ]; then
  GIT_AUTHOR_EMAIL="spappalardo@mixxx.org";
  GIT_AUTHOR_NAME="Sean M. Pappalardo";
fi;
if [ "$GIT_COMMITTER_NAME" = "pegasusrpg" ]; then
  GIT_COMMITTER_EMAIL="spappalardo@mixxx.org";
  GIT_COMMITTER_NAME="Sean M. Pappalardo";
fi;
if [ "$GIT_AUTHOR_NAME" = "petemix" ]; then
  GIT_AUTHOR_EMAIL="Peter.Chang@nottingham.ac.uk";
  GIT_AUTHOR_NAME="Peter Chang";
fi;
if [ "$GIT_COMMITTER_NAME" = "petemix" ]; then
  GIT_COMMITTER_EMAIL="Peter.Chang@nottingham.ac.uk";
  GIT_COMMITTER_NAME="Peter Chang";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "madjester@voidwalker" ]; then
  GIT_AUTHOR_EMAIL="pwhelan@mixxx.org";
  GIT_AUTHOR_NAME="Phillip Whelan";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "madjester@voidwalker" ]; then
  GIT_COMMITTER_EMAIL="pwhelan@mixxx.org";
  GIT_COMMITTER_NAME="Phillip Whelan";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "pwhelan@mixxx.org" ]; then
  GIT_AUTHOR_EMAIL="pwhelan@mixxx.org";
  GIT_AUTHOR_NAME="Phillip Whelan";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "pwhelan@mixxx.org" ]; then
  GIT_COMMITTER_EMAIL="pwhelan@mixxx.org";
  GIT_COMMITTER_NAME="Phillip Whelan";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "madjester@madjester-netbook" ]; then
  GIT_AUTHOR_EMAIL="pwhelan@mixxx.org";
  GIT_AUTHOR_NAME="Phillip Whelan";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "madjester@madjester-netbook" ]; then
  GIT_COMMITTER_EMAIL="pwhelan@mixxx.org";
  GIT_COMMITTER_NAME="Phillip Whelan";
fi;
if [ "$GIT_AUTHOR_NAME" = "psyc0de" ]; then
  GIT_AUTHOR_EMAIL="psyc0de@corrodedreality.org";
  GIT_AUTHOR_NAME="Tom Care";
fi;
if [ "$GIT_COMMITTER_NAME" = "psyc0de" ]; then
  GIT_COMMITTER_EMAIL="psyc0de@corrodedreality.org";
  GIT_COMMITTER_NAME="Tom Care";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "khyew@khyew.com" ]; then
  GIT_AUTHOR_EMAIL="khyew@khyew.com";
  GIT_AUTHOR_NAME="Qifan Xi";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "khyew@khyew.com" ]; then
  GIT_COMMITTER_EMAIL="khyew@khyew.com";
  GIT_COMMITTER_NAME="Qifan Xi";
fi;
if [ "$GIT_AUTHOR_NAME" = "raffitea" ]; then
  GIT_AUTHOR_EMAIL="trafreider@mixxx.org";
  GIT_AUTHOR_NAME="Tobias Rafreider";
fi;
if [ "$GIT_COMMITTER_NAME" = "raffitea" ]; then
  GIT_COMMITTER_EMAIL="trafreider@mixxx.org";
  GIT_COMMITTER_NAME="Tobias Rafreider";
fi;
if [ "$GIT_AUTHOR_NAME" = "Raffitea" ]; then
  GIT_AUTHOR_EMAIL="trafreider@mixxx.org";
  GIT_AUTHOR_NAME="Tobias Rafreider";
fi;
if [ "$GIT_COMMITTER_NAME" = "Raffitea" ]; then
  GIT_COMMITTER_EMAIL="trafreider@mixxx.org";
  GIT_COMMITTER_NAME="Tobias Rafreider";
fi;
if [ "$GIT_AUTHOR_NAME" = "RaffiTea" ]; then
  GIT_AUTHOR_EMAIL="trafreider@mixxx.org";
  GIT_AUTHOR_NAME="Tobias Rafreider";
fi;
if [ "$GIT_COMMITTER_NAME" = "RaffiTea" ]; then
  GIT_COMMITTER_EMAIL="trafreider@mixxx.org";
  GIT_COMMITTER_NAME="Tobias Rafreider";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "rryan@mit.edu" ]; then
  GIT_AUTHOR_EMAIL="rryan@mixxx.org";
  GIT_AUTHOR_NAME="RJ Ryan";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "rryan@mit.edu" ]; then
  GIT_COMMITTER_EMAIL="rryan@mixxx.org";
  GIT_COMMITTER_NAME="RJ Ryan";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "rryan@mixxx.org" ]; then
  GIT_AUTHOR_EMAIL="rryan@mixxx.org";
  GIT_AUTHOR_NAME="RJ Ryan";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "rryan@mixxx.org" ]; then
  GIT_COMMITTER_EMAIL="rryan@mixxx.org";
  GIT_COMMITTER_NAME="RJ Ryan";
fi;
if [ "$GIT_AUTHOR_NAME" = "rryan" ]; then
  GIT_AUTHOR_EMAIL="rryan@mixxx.org";
  GIT_AUTHOR_NAME="RJ Ryan";
fi;
if [ "$GIT_COMMITTER_NAME" = "rryan" ]; then
  GIT_COMMITTER_EMAIL="rryan@mixxx.org";
  GIT_COMMITTER_NAME="RJ Ryan";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "rryan@atrius" ]; then
  GIT_AUTHOR_EMAIL="rryan@mixxx.org";
  GIT_AUTHOR_NAME="RJ Ryan";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "rryan@atrius" ]; then
  GIT_COMMITTER_EMAIL="rryan@mixxx.org";
  GIT_COMMITTER_NAME="RJ Ryan";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "rryan@free-radical" ]; then
  GIT_AUTHOR_EMAIL="rryan@mixxx.org";
  GIT_AUTHOR_NAME="RJ Ryan";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "rryan@free-radical" ]; then
  GIT_COMMITTER_EMAIL="rryan@mixxx.org";
  GIT_COMMITTER_NAME="RJ Ryan";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "ryanbaker@MacBookPro.local" ]; then
  GIT_AUTHOR_EMAIL="bakerry@onid.orst.edu";
  GIT_AUTHOR_NAME="Ryan Baker";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "ryanbaker@MacBookPro.local" ]; then
  GIT_COMMITTER_EMAIL="bakerry@onid.orst.edu";
  GIT_COMMITTER_NAME="Ryan Baker";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "pegasus@8440p" ]; then
  GIT_AUTHOR_EMAIL="spappalardo@mixxx.org";
  GIT_AUTHOR_NAME="Sean M. Pappalardo";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "pegasus@8440p" ]; then
  GIT_COMMITTER_EMAIL="spappalardo@mixxx.org";
  GIT_COMMITTER_NAME="Sean M. Pappalardo";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "pegasus@NikkiNetbook" ]; then
  GIT_AUTHOR_EMAIL="spappalardo@mixxx.org";
  GIT_AUTHOR_NAME="Sean M. Pappalardo";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "pegasus@NikkiNetbook" ]; then
  GIT_COMMITTER_EMAIL="spappalardo@mixxx.org";
  GIT_COMMITTER_NAME="Sean M. Pappalardo";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "pegasus@presariolaptop" ]; then
  GIT_AUTHOR_EMAIL="spappalardo@mixxx.org";
  GIT_AUTHOR_NAME="Sean M. Pappalardo";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "pegasus@presariolaptop" ]; then
  GIT_COMMITTER_EMAIL="spappalardo@mixxx.org";
  GIT_COMMITTER_NAME="Sean M. Pappalardo";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "pegasus@renegadetech.com" ]; then
  GIT_AUTHOR_EMAIL="spappalardo@mixxx.org";
  GIT_AUTHOR_NAME="Sean M. Pappalardo";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "pegasus@renegadetech.com" ]; then
  GIT_COMMITTER_EMAIL="spappalardo@mixxx.org";
  GIT_COMMITTER_NAME="Sean M. Pappalardo";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "spappalardo@mixxx.org" ]; then
  GIT_AUTHOR_EMAIL="spappalardo@mixxx.org";
  GIT_AUTHOR_NAME="Sean M. Pappalardo";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "spappalardo@mixxx.org" ]; then
  GIT_COMMITTER_EMAIL="spappalardo@mixxx.org";
  GIT_COMMITTER_NAME="Sean M. Pappalardo";
fi;
if [ "$GIT_AUTHOR_NAME" = "snipexv" ]; then
  GIT_AUTHOR_EMAIL="snipexv@gmail.com";
  GIT_AUTHOR_NAME="Micah Lee";
fi;
if [ "$GIT_COMMITTER_NAME" = "snipexv" ]; then
  GIT_COMMITTER_EMAIL="snipexv@gmail.com";
  GIT_COMMITTER_NAME="Micah Lee";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "kabelfrickler@googlemail.com" ]; then
  GIT_AUTHOR_EMAIL="kabelfrickler@googlemail.com";
  GIT_AUTHOR_NAME="Stefan Nuernberger";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "kabelfrickler@googlemail.com" ]; then
  GIT_COMMITTER_EMAIL="kabelfrickler@googlemail.com";
  GIT_COMMITTER_NAME="Stefan Nuernberger";
fi;
if [ "$GIT_AUTHOR_NAME" = "sveinmb" ]; then
  GIT_AUTHOR_EMAIL="sveinmb@stud.ntnu.no";
  GIT_AUTHOR_NAME="Svein Magne Bang";
fi;
if [ "$GIT_COMMITTER_NAME" = "sveinmb" ]; then
  GIT_COMMITTER_EMAIL="sveinmb@stud.ntnu.no";
  GIT_COMMITTER_NAME="Svein Magne Bang";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "root@MacBookPro.local" ]; then
  GIT_AUTHOR_EMAIL="bakerry@onid.orst.edu";
  GIT_AUTHOR_NAME="Ryan Baker";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "root@MacBookPro.local" ]; then
  GIT_COMMITTER_EMAIL="bakerry@onid.orst.edu";
  GIT_COMMITTER_NAME="Ryan Baker";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "tobias@tobias-laptop" ]; then
  GIT_AUTHOR_EMAIL="trafreider@mixxx.org";
  GIT_AUTHOR_NAME="Tobias Rafreider";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "tobias@tobias-laptop" ]; then
  GIT_COMMITTER_EMAIL="trafreider@mixxx.org";
  GIT_COMMITTER_NAME="Tobias Rafreider";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "Tobias@Mixxx" ]; then
  GIT_AUTHOR_EMAIL="trafreider@mixxx.org";
  GIT_AUTHOR_NAME="Tobias Rafreider";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "Tobias@Mixxx" ]; then
  GIT_COMMITTER_EMAIL="trafreider@mixxx.org";
  GIT_COMMITTER_NAME="Tobias Rafreider";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "Tobias@Silverstar" ]; then
  GIT_AUTHOR_EMAIL="trafreider@mixxx.org";
  GIT_AUTHOR_NAME="Tobias Rafreider";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "Tobias@Silverstar" ]; then
  GIT_COMMITTER_EMAIL="trafreider@mixxx.org";
  GIT_COMMITTER_NAME="Tobias Rafreider";
fi;
if [ "$GIT_AUTHOR_NAME" = "Tom Mast thomasomast@gmail.com" ]; then
  GIT_AUTHOR_EMAIL="thomasomast@gmail.com";
  GIT_AUTHOR_NAME="Tom Mast";
fi;
if [ "$GIT_COMMITTER_NAME" = "Tom Mast thomasomast@gmail.com" ]; then
  GIT_COMMITTER_EMAIL="thomasomast@gmail.com";
  GIT_COMMITTER_NAME="Tom Mast";
fi;
if [ "$GIT_AUTHOR_NAME" = "tuehaste" ]; then
  GIT_AUTHOR_EMAIL="tue@haste.dk";
  GIT_AUTHOR_NAME="Tue Haste Andersen";
fi;
if [ "$GIT_COMMITTER_NAME" = "tuehaste" ]; then
  GIT_COMMITTER_EMAIL="tue@haste.dk";
  GIT_COMMITTER_NAME="Tue Haste Andersen";
fi;
if [ "$GIT_AUTHOR_NAME" = "uid77225" ]; then
  GIT_AUTHOR_EMAIL="tue@haste.dk";
  GIT_AUTHOR_NAME="Tue Haste Andersen";
fi;
if [ "$GIT_COMMITTER_NAME" = "uid77225" ]; then
  GIT_COMMITTER_EMAIL="tue@haste.dk";
  GIT_COMMITTER_NAME="Tue Haste Andersen";
fi;
if [ "$GIT_AUTHOR_NAME" = "uklotzde" ]; then
  GIT_AUTHOR_EMAIL="uwe_klotz@web.de";
  GIT_AUTHOR_NAME="Uwe Klotz";
fi;
if [ "$GIT_COMMITTER_NAME" = "uklotzde" ]; then
  GIT_COMMITTER_EMAIL="uwe_klotz@web.de";
  GIT_COMMITTER_NAME="Uwe Klotz";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "vininim@gmail.com" ]; then
  GIT_AUTHOR_EMAIL="vininim@gmail.com";
  GIT_AUTHOR_NAME="Vinícius Dias dos Santos";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "vininim@gmail.com" ]; then
  GIT_COMMITTER_EMAIL="vininim@gmail.com";
  GIT_COMMITTER_NAME="Vinícius Dias dos Santos";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "l0rdt@gmx.it" ]; then
  GIT_AUTHOR_EMAIL="l0rdt@gmx.it";
  GIT_AUTHOR_NAME="Vittorio";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "l0rdt@gmx.it" ]; then
  GIT_COMMITTER_EMAIL="l0rdt@gmx.it";
  GIT_COMMITTER_NAME="Vittorio";
fi;
if [ "$GIT_AUTHOR_EMAIL" = "vrince@gmail.com" ]; then
  GIT_AUTHOR_EMAIL="vrince@gmail.com";
  GIT_AUTHOR_NAME="vrince";
fi;
if [ "$GIT_COMMITTER_EMAIL" = "vrince@gmail.com" ]; then
  GIT_COMMITTER_EMAIL="vrince@gmail.com";
  GIT_COMMITTER_NAME="vrince";
fi;
if [ "$GIT_AUTHOR_NAME" = "wesleyprofox" ]; then
  GIT_AUTHOR_EMAIL="wesley@ubuntu.com";
  GIT_AUTHOR_NAME="Wesley Stessens";
fi;
if [ "$GIT_COMMITTER_NAME" = "wesleyprofox" ]; then
  GIT_COMMITTER_EMAIL="wesley@ubuntu.com";
  GIT_COMMITTER_NAME="Wesley Stessens";
fi;
if [ "$GIT_AUTHOR_NAME" = "who8877" ]; then
  GIT_AUTHOR_EMAIL="jsully@scs.ryerson.ca";
  GIT_AUTHOR_NAME="John Sully";
fi;
if [ "$GIT_COMMITTER_NAME" = "who8877" ]; then
  GIT_COMMITTER_EMAIL="jsully@scs.ryerson.ca";
  GIT_COMMITTER_NAME="John Sully";
fi;
if [ "$GIT_AUTHOR_NAME" = "zestoi@djism.com" ]; then
  GIT_AUTHOR_EMAIL="zestoi@djism.com";
  GIT_AUTHOR_NAME="zestoi";
fi;
if [ "$GIT_COMMITTER_NAME" = "zestoi@djism.com" ]; then
  GIT_COMMITTER_EMAIL="zestoi@djism.com";
  GIT_COMMITTER_NAME="zestoi";
fi;

export GIT_AUTHOR_NAME
export GIT_AUTHOR_EMAIL
export GIT_COMMITTER_NAME
export GIT_COMMITTER_EMAIL'

