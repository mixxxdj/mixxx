#include "dialog/dlgabout.h"

#include <QFile>

#include "defs_urls.h"
#include "moc_dlgabout.cpp"
#include "util/color/color.h"
#include "util/version.h"

DlgAbout::DlgAbout(QWidget* parent) : QDialog(parent), Ui::DlgAboutDlg() {
    setupUi(this);

    QString mixxxVersion = Version::version();
    QString buildBranch = Version::developmentBranch();
    QString buildRevision = Version::developmentRevision();

    QStringList version;
    version.append(mixxxVersion);

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

    QFile licenseFile(":/LICENSE");
    if (!licenseFile.open(QIODevice::ReadOnly)) {
        qWarning() << "LICENSE file not found";
    } else {
        licenseText->setPlainText(licenseFile.readAll());
    }

    QString s_devTeam = tr("Mixxx %1 Development Team").arg(mixxxVersion);
    QString s_contributions = tr("With contributions from:");
    QString s_specialThanks = tr("And special thanks to:");
    QString s_pastDevs = tr("Past Developers");
    QString s_pastContribs = tr("Past Contributors");

    QStringList thisReleaseDevelopers;
    thisReleaseDevelopers
            << "RJ Skerry-Ryan"
            << "Owen Williams"
            << "Sean Pappalardo"
            << "Daniel Sch&uuml;rmann"
            << "S. Brandt"
            << "Uwe Klotz"
            << "Be"
            << "S&eacute;bastien Blaisot"
            << "ronso0"
            << "Jan Holthuis";

    // This list should contains all contributors committed
    // code to the Mixxx core within the past two years.
    // New Contributors are added at the end.
    QStringList recentContributors;
    recentContributors
            << "Tuukka Pasanen"
            << "Nino MP"
            << "Nico Schl&ouml;mer"
            << "Ferran Pujol Camins"
            << "Joan Marc&egrave; i Igual"
            << "Josep Maria Antol&iacute;n Segura"
            << "Daniel Poelzleithner"
            << "St&eacute;phane Lepin"
            << "Stefan Weber"
            << "Kshitij Gupta"
            << "Matthew Nicholson"
            << "Jamie Gifford"
            << "luzpaz"
            << "Sebastian Reu&szlig;e"
            << "Pawe&#322; Goli&#324;ski"
            << "beenisss"
            << "Bernd Binder"
            << "Pradyuman"
            << "Nikolaus Einhauser"
            << "Nik Martin"
            << "Kerrick Staley"
            << "Raphael Graf"
            << "Nik Martin"
            << "YunQiang Su"
            << "Sebastian Hasler"
            << "Philip Gottschling"
            << "Melissa"
            << "Ned Haughton"
            << "Adam Szmigin"
            << "Cristiano Lacerda"
            << "Sergey Ukolov"
            << "Ketan Lambat"
            << "Evan Dekker"
            << "Edward Kigwana"
            << "Simon Harst"
            << "Harshit Maurya"
            << "Janek Fischer"
            << "St&eacute;phane Lepin"
            << "J&eacute;r&ocirc;me Blanchi"
            << "Chris Hills"
            << "David Lowenfels"
            << "Matthieu Bouron"
            << "Nathan Korth"
            << "Kristiyan Katsarov"
            << "J&ouml;rg Wartenberg"
            << "Sanskar Bajpai"
            << "Edward Millen"
            << "Frank Breitling"
            << "Christian"
            << "Geraldo Nascimento"
            << "Albert Aparicio";

    QStringList specialThanks;
    specialThanks
            << "Mark Hills"
            << "Oscillicious"
            << "Vestax"
            << "Stanton"
            << "Hercules"
            << "EKS"
            << "Echo Digital Audio"
            << "JP Disco"
            << "Google Summer of Code"
            << "Adam Bellinson"
            << "Alexandre Bancel"
            << "Melanie Thielker"
            << "Julien Rosener"
            << "Pau Arum&iacute;"
            << "David Garcia"
            << "Seb Ruiz"
            << "Joseph Mattiello";

    QStringList pastDevelopers;
    pastDevelopers
            << "Tue Haste Andersen"
            << "Ken Haste Andersen"
            << "Cedric Gestes"
            << "John Sully"
            << "Torben Hohn"
            << "Peter Chang"
            << "Micah Lee"
            << "Ben Wheeler"
            << "Wesley Stessens"
            << "Nathan Prado"
            << "Zach Elko"
            << "Tom Care"
            << "Pawel Bartkiewicz"
            << "Nick Guenther"
            << "Adam Davison"
            << "Garth Dahlstrom"
            << "Albert Santoni"
            << "Phillip Whelan"
            << "Tobias Rafreider"
            << "Bill Good"
            << "Vittorio Colao"
            << "Thomas Vincent"
            << "Ilkka Tuohela"
            << "Max Linke"
            << "Marcos Cardinot"
            << "Nicu Badescu";

    QStringList pastContributors;
    pastContributors
            << "Ludek Hor&#225;cek"
            << "Svein Magne Bang"
            << "Kristoffer Jensen"
            << "Ingo Kossyk"
            << "Mads Holm"
            << "Lukas Zapletal"
            << "Jeremie Zimmermann"
            << "Gianluca Romanin"
            << "Tim Jackson"
            << "Stefan Langhammer"
            << "Frank Willascheck"
            << "Jeff Nelson"
            << "Kevin Schaper"
            << "Alex Markley"
            << "Oriol Puigb&oacute;"
            << "Ulrich Heske"
            << "James Hagerman"
            << "quil0m80"
            << "Martin Sakm&#225;r"
            << "Ilian Persson"
            << "Dave Jarvis"
            << "Thomas Baag"
            << "Karlis Kalnins"
            << "Amias Channer"
            << "Sacha Berger"
            << "James Evans"
            << "Martin Sakmar"
            << "Navaho Gunleg"
            << "Gavin Pryke"
            << "Michael Pujos"
            << "Claudio Bantaloukas"
            << "Pavol Rusnak"
            << "Bruno Buccolo"
            << "Ryan Baker"
            << "Andre Roth"
            << "Robin Sheat"
            << "Mark Glines"
            << "Mathieu Rene"
            << "Miko Kiiski"
            << "Brian Jackson"
            << "Andreas Pflug"
            << "Bas van Schaik"
            << "J&aacute;n Jockusch"
            << "Oliver St&ouml;neberg"
            << "Jan Jockusch"
            << "C. Stewart"
            << "Bill Egert"
            << "Zach Shutters"
            << "Owen Bullock"
            << "Graeme Mathieson"
            << "Sebastian Actist"
            << "Jussi Sainio"
            << "David Gnedt"
            << "Antonio Passamani"
            << "Guy Martin"
            << "Anders Gunnarsson"
            << "Mikko Jania"
            << "Juan Pedro Bol&iacute;var Puente"
            << "Linus Amvall"
            << "Irwin C&eacute;spedes B"
            << "Micz Flor"
            << "Daniel James"
            << "Mika Haulo"
            << "Tom Mast"
            << "Miko Kiiski"
            << "Vin&iacute;cius Dias dos Santos"
            << "Joe Colosimo"
            << "Shashank Kumar"
            << "Till Hofmann"
            << "Peter V&aacute;gner"
            << "Jens Nachtigall"
            << "Scott Ullrich"
            << "Jonas &Aring;dahl"
            << "Jonathan Costers"
            << "Maxime Bochon"
            << "Akash Shetye"
            << "Pascal Bleser"
            << "Florian Mahlknecht"
            << "Ben Clark"
            << "Tom Gascoigne"
            << "Aaron Mavrinac"
            << "Markus H&auml;rer"
            << "Scott Stewart"
            << "Nimatek"
            << "Matthew Mikolay"
            << "Thanasis Liappis"
            << "Daniel Lindenfelser"
            << "Andrey Smelov"
            << "Alban Bedel"
            << "Steven Boswell"
            << "Jo&atilde;o Reys Santos"
            << "Carl Pillot"
            << "Vedant Agarwala"
            << "Nazar Gerasymchuk"
            << "Federico Briata"
            << "Leo Combes"
            << "Florian Kiekh&auml;fer"
            << "Michael Sawyer"
            << "Quentin Faidide"
            << "Peter G. Marczis"
            << "Khyrul Bashar"
            << "Johannes Obermayr"
            << "Kevin Lee"
            << "Evan Radkoff"
            << "Lee Matos"
            << "Ryan Kramer"
            << "Zak Reynolds"
            << "Dennis Rohner"
            << "Juha Pitk&auml;nen"
            << "Varun Jewalikar"
            << "Dennis Wallace"
            << "Keith Salisbury"
            << "Irina Grosu"
            << "Callum Styan"
            << "Rahul Behl"
            << "Markus Baertschi"
            << "Don Dennis"
            << "Alexandru Jercaianu"
            << "Nils Goroll"
            << "Marco Angerer"
            << "Thorsten Munsch"
            << "Emile Vrijdags"
            << "St&eacute;phane Guillou"
            << "Russ Mannex"
            << "Brendan Austin"
            << "Lorenz Drescher"
            << "David Guglielmi"
            << "James Atwill"
            << "Alex Barker"
            << "Jean Claveau"
            << "Kevin Wern"
            << "Vladim&iacute;r Dudr"
            << "Neale Pickett"
            << "Chlo&eacute; Avrillon"
            << "Hendrik Reglin"
            << "Serge Ukolov"
            << "Patric Schmitz"
            << "Roland Schwarz"
            << "Jan Ypma"
            << "Andreas M&uuml;ller"
            << "Sam Cross"
            << "Joey Pabalinas"
            << "Stefan N&uuml;rnberger"
            << "Markus Kl&ouml;sges"
            << "Pavel Potocek"
            << "Timothy Rae"
            << "Leigh Scott"
            << "William Lemus"
            << "Nimit Bhardwaj"
            << "Pavel Sokolov"
            << "Devananda van der Veen"
            << "Tatsuyuki Ishi"
            << "Kilian Feess"
            << "Conner Phillips"
            << "Artyom Lyan"
            << "Johan Lasperas"
            << "Olaf Hering"
            << "Eduardo Acero"
            << "Thomas Jarosch";

    QString sectionTemplate = QString(
        "<p align=\"center\"><b>%1</b></p><p align=\"center\">%2</p>");
    QStringList sections;
    sections << sectionTemplate.arg(s_devTeam,
                                    thisReleaseDevelopers.join("<br>"))
             << sectionTemplate.arg(s_contributions,
                                    recentContributors.join("<br>"))
             << sectionTemplate.arg(s_pastDevs,
                                    pastDevelopers.join("<br>"))
             << sectionTemplate.arg(s_pastContribs,
                                    pastContributors.join("<br>"))
             << sectionTemplate.arg(s_specialThanks,
                                    specialThanks.join("<br>"));
    textBrowser->setHtml(sections.join(""));

    textWebsiteLink->setText(
            QString("<a style=\"color:%1;\" href=\"%2\">%3</a>")
                    .arg(Color::blendColors(palette().link().color(),
                                 palette().text().color())
                                    .name(),
                            MIXXX_WEBSITE_URL,
                            tr("Official Website")));

    connect(buttonBox, &QDialogButtonBox::accepted, this, &DlgAbout::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &DlgAbout::reject);
}
