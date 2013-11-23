/***************************************************************************
                          dlgabout.cpp  -  description
                             -------------------
    begin                : Mon Nov 19 2007
    copyright            : (C) 2007 by Albert Santoni
    email                : gamegod at users.sf.net
 ***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "dlgabout.h"

#include "defs_version.h"

DlgAbout::DlgAbout(QWidget* parent) :  QDialog(parent), Ui::DlgAboutDlg() {
    setupUi(this);

    QString buildBranch, buildRevision;
#ifdef BUILD_BRANCH
    buildBranch = BUILD_BRANCH;
#endif
#ifdef BUILD_REV
    buildRevision = BUILD_REV;
#endif

    QStringList version;
    version.append(VERSION);
    if (!buildBranch.isEmpty() || !buildRevision.isEmpty()) {
        QStringList buildInfo;
        buildInfo.append("build");
        if (!buildBranch.isEmpty()) {
            buildInfo.append(buildBranch);
        }
        if (!buildRevision.isEmpty()) {
            buildInfo.append(QString("r%1").arg(buildRevision));
        }
        version.append(QString("(%1)").arg(buildInfo.join(" ")));
    }
    version_label->setText(version.join(" "));

    QString s_devTeam = QString(tr("Mixxx %1 Development Team")).arg(VERSION);
    QString s_contributions = tr("With contributions from:");
    QString s_specialThanks = tr("And special thanks to:");
    QString s_pastDevs = tr("Past Developers");
    QString s_pastContribs = tr("Past Contributors");

    QString credits = QString("<p align=\"center\"><b>%1</b></p>"
"<p align=\"center\">"
"Albert Santoni<br>"
"RJ Ryan<br>"
"Sean Pappalardo<br>"
"Phillip Whelan<br>"
"Tobias Rafreider<br>"
"S. Brandt<br>"
"Bill Good<br>"
"Owen Williams<br>"
"Vittorio Colao<br>"
"Daniel Sch&uuml;rmann<br>"
"Thomas Vincent<br>"
"Ilkka Tuohela<br>"
"Max Linke<br>"

"</p>"
"<p align=\"center\"><b>%2</b></p>"
"<p align=\"center\">"
"Mark Hills<br>"
"Andre Roth<br>"
"Robin Sheat<br>"
"Mark Glines<br>"
"Mathieu Rene<br>"
"Miko Kiiski<br>"
"Brian Jackson<br>"
"Andreas Pflug<br>"
"Bas van Schaik<br>"
"J&aacute;n Jockusch<br>"
"Oliver St&ouml;neberg<br>"
"Jan Jockusch<br>"
"C. Stewart<br>"
"Bill Egert<br>"
"Zach Shutters<br>"
"Owen Bullock<br>"
"Graeme Mathieson<br>"
"Sebastian Actist<br>"
"Jussi Sainio<br>"
"David Gnedt<br>"
"Antonio Passamani<br>"
"Guy Martin<br>"
"Anders Gunnarsson<br>"
"Alex Barker<br>"
"Mikko Jania<br>"
"Juan Pedro Bol&iacute;var Puente<br>"
"Linus Amvall<br>"
"Irwin C&eacute;spedes B<br>"
"Micz Flor<br>"
"Daniel James<br>"
"Mika Haulo<br>"
"Matthew Mikolay<br>"
"Tom Mast<br>"
"Miko Kiiski<br>"
"Vin&iacute;cius Dias dos Santos<br>"
"Joe Colosimo<br>"
"Shashank Kumar<br>"
"Till Hofmann<br>"
"Peter V&aacute;gner<br>"
"Thanasis Liappis<br>"
"Jens Nachtigall<br>"
"Scott Ullrich<br>"
"Jonas &Aring;dahl<br>"
"Jonathan Costers<br>"
"Daniel Lindenfelser<br>"
"Maxime Bochon<br>"
"Akash Shetye<br>"
"Pascal Bleser<br>"
"Florian Mahlknecht<br>"
"Ben Clark<br>"
"Tom Gascoigne<br>"
"Neale Pickett<br>"
"Aaron Mavrinac<br>"
"Markus H&auml;rer<br>"
"Andrey Smelov<br>"
"Scott Stewart<br>"
"Nimatek<br>"
"Alban Bedel<br>"
"Stefan N&uuml;rnberger<br>"
"Steven Boswell<br>"
"Jo&atilde;o Reys Santos<br>"
"Carl Pillot<br>"
"Vedant Agarwala<br>"

"</p>"
"<p align=\"center\"><b>%3</b></p>"
"<p align=\"center\">"
"Vestax<br>"
"Stanton<br>"
"Hercules<br>"
"EKS<br>"
"Echo Digital Audio<br>"
"JP Disco<br>"
"Google Summer of Code<br>"
"Adam Bellinson<br>"
"Alexandre Bancel<br>"
"Melanie Thielker<br>"
"Julien Rosener<br>"
"Pau Arum&iacute;<br>"
"David Garcia<br>"
"Seb Ruiz<br>"
"Joseph Mattiello<br>"
"</p>"

"<p align=\"center\"><b>%4</b></p>"
"<p align=\"center\">"
"Tue Haste Andersen<br>"
"Ken Haste Andersen<br>"
"Cedric Gestes<br>"
"John Sully<br>"
"Torben Hohn<br>"
"Peter Chang<br>"
"Micah Lee<br>"
"Ben Wheeler<br>"
"Wesley Stessens<br>"
"Nathan Prado<br>"
"Zach Elko<br>"
"Tom Care<br>"
"Pawel Bartkiewicz<br>"
"Nick Guenther<br>"
"Adam Davison<br>"
"Garth Dahlstrom<br>"
"</p>"

"<p align=\"center\"><b>%5</b></p>"
"<p align=\"center\">"
"Ludek Hor&#225;cek<br>"
"Svein Magne Bang<br>"
"Kristoffer Jensen<br>"
"Ingo Kossyk<br>"
"Mads Holm<br>"
"Lukas Zapletal<br>"
"Jeremie Zimmermann<br>"
"Gianluca Romanin<br>"
"Tim Jackson<br>"
"Stefan Langhammer<br>"
"Frank Willascheck<br>"
"Jeff Nelson<br>"
"Kevin Schaper<br>"
"Alex Markley<br>"
"Oriol Puigb&oacute;<br>"
"Ulrich Heske<br>"
"James Hagerman<br>"
"quil0m80<br>"
"Martin Sakm&#225;r<br>"
"Ilian Persson<br>"
"Dave Jarvis<br>"
"Thomas Baag<br>"
"Karlis Kalnins<br>"
"Amias Channer<br>"
"Sacha Berger<br>"
"James Evans<br>"
"Martin Sakmar<br>"
"Navaho Gunleg<br>"
"Gavin Pryke<br>"
"Michael Pujos<br>"
"Claudio Bantaloukas<br>"
"Pavol Rusnak<br>"
"Bruno Buccolo<br>"
"Ryan Baker<br>"
    "</p>").arg(s_devTeam,s_contributions,s_specialThanks,s_pastDevs,s_pastContribs);

    textBrowser->setHtml(credits);
}

DlgAbout::~DlgAbout() {
}

void DlgAbout::slotApply() {
}

void DlgAbout::slotUpdate() {
}

