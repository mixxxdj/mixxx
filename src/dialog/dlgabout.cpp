#include "dialog/dlgabout.h"
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

    QString s_devTeam = tr("Mixxx %1 Development Team").arg(mixxxVersion);
    QString s_contributions = tr("With contributions from:");
    QString s_specialThanks = tr("And special thanks to:");
    QString s_pastDevs = tr("Past Developers");
    QString s_pastContribs = tr("Past Contributors");

    QStringList thisReleaseDevelopers;
    thisReleaseDevelopers
            << "RJ Ryan"
            << "Owen Williams"
            << "Sean Pappalardo"
            << "Daniel Sch&uuml;rmann"
            << "S. Brandt"
            << "Ilkka Tuohela"
            << "Max Linke"
            << "Marcos Cardinot"
            << "Nicu Badescu";

    QStringList thisReleaseContributors;
    thisReleaseContributors
            << "Alex Barker"
            << "Matthew Mikolay"
            << "Thanasis Liappis"
            << "Daniel Lindenfelser"
            << "Andrey Smelov"
            << "Alban Bedel"
            << "Stefan N&uuml;rnberger"
            << "Steven Boswell"
            << "Jo&atilde;o Reys Santos"
            << "Carl Pillot"
            << "Vedant Agarwala"
            << "Nazar Gerasymchuk"
            << "Federico Briata"
            << "Leo Combes"
            << "Florian Kiekh&auml;fer"
            << "Michael Sawyer"
            << "Tuukka Pasanen"
            << "Uwe Klotz"
            << "Quentin Faidide"
            << "Peter G. Marczis"
            << "Khyrul Bashar"
            << "Johannes Obermayr"
            << "Kevin Lee"
            << "Evan Radkoff"
            << "Lee Matos"
            << "Jean Claveau"
            << "Nino MP"
            << "Ryan Kramer"
            << "Zak Reynolds"
            << "Dennis Rohner"
            << "Juha Pitk&auml;nen"
            << "Kevin Wern"
            << "Varun Jewalikar"
            << "Dennis Wallace"
            << "Keith Salisbury"
            << "Irina Grosu"
            << "Callum Styan"
            << "Rahul Behl"
            << "Markus Baertschi"
            << "Nico Schl&ouml;mer"
            << "Don Dennis"
            << "Alexandru Jercaianu"
            << "Nils Goroll"
            << "Marco Angerer"
            << "Ferran Pujol Camins"
            << "Markus Kl&ouml;sges"
            << "S&eacute;bastien Blaisot"
            << "Vladim&iacute;r Dudr"
            << "Thorsten Munsch"
            << "Emile Vrijdags"
            << "Be"
            << "Neale Pickett"
            << "St&eacute;phane Guillou"
            << "Russ Mannex"
            << "Brendan Austin"
            << "Lorenz Drescher"
            << "David Guglielmi"
            << "James Atwill"
            << "Chlo&eacute; Avrillon"
            << "Hendrik Reglin"
            << "Pavel Potocek"
            << "Joan Marc&egrave; i Igual"
            << "Serge Ukolov"
            << "Patric Schmitz"
            << "Timothy Rae"
            << "Roland Schwarz"
            << "Jan Ypma"
            << "Leigh Scott"
            << "William Lemus"
            << "Andreas M&uuml;ller"
            << "Josep Maria Antol&iacute;n Segura"
            << "Sam Cross"
            << "Joey Pabalinas"
            << "Nimit Bhardwaj"
            << "Pavel Sokolov"
            << "Devananda van der Veen";  

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
            << "Thomas Vincent";

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
            << "Neale Pickett"
            << "Aaron Mavrinac"
            << "Markus H&auml;rer"
            << "Scott Stewart"
            << "Nimatek";

    QString sectionTemplate = QString(
        "<p align=\"center\"><b>%1</b></p><p align=\"center\">%2</p>");
    QStringList sections;
    sections << sectionTemplate.arg(s_devTeam,
                                    thisReleaseDevelopers.join("<br>"))
             << sectionTemplate.arg(s_contributions,
                                    thisReleaseContributors.join("<br>"))
             << sectionTemplate.arg(s_pastDevs,
                                    pastDevelopers.join("<br>"))
             << sectionTemplate.arg(s_pastContribs,
                                    pastContributors.join("<br>"))
             << sectionTemplate.arg(s_specialThanks,
                                    specialThanks.join("<br>"));
    textBrowser->setHtml(sections.join(""));
}
