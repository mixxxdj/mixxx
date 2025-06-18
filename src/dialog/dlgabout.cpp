#include "dialog/dlgabout.h"

#include <QDebug>
#include <QFile>
#include <QLocale>

#include "defs_urls.h"
#include "moc_dlgabout.cpp"
#include "util/color/color.h"
#include "util/desktophelper.h"
#include "util/versionstore.h"

DlgAbout::DlgAbout()
        : QDialog(nullptr),
          Ui::DlgAboutDlg() {
    setupUi(this);
    setWindowIcon(QIcon(MIXXX_ICON_PATH));

    mixxx_icon->load(QString(MIXXX_ICON_PATH));
    mixxx_logo->load(QString(MIXXX_LOGO_PATH));

    version_label->setText(VersionStore::applicationName() +
            QStringLiteral(" ") + VersionStore::version());
    git_version_label->setText(VersionStore::gitVersion());
    qt_version_label->setText(VersionStore::qtVersion());
    platform_label->setText(VersionStore::platform());
    QLocale locale;
    date_label->setText(locale.toString(VersionStore::date().toLocalTime(), QLocale::LongFormat));

    QFile licenseFile(":/LICENSE");
    if (!licenseFile.open(QIODevice::ReadOnly)) {
        qWarning() << "LICENSE file not found";
    } else {
        licenseText->setPlainText(licenseFile.readAll());
    }

    QString s_devTeam =
            tr("Mixxx %1.%2 Development Team")
                    .arg(QString::number(
                                 VersionStore::versionNumber().majorVersion()),
                            QString::number(VersionStore::versionNumber()
                                                    .minorVersion()));
    QString s_contributions = tr("With contributions from:");
    QString s_specialThanks = tr("And special thanks to:");
    QString s_pastDevs = tr("Past Developers");
    QString s_pastContribs = tr("Past Contributors");

    QStringList thisReleaseDevelopers;
    thisReleaseDevelopers
            << "RJ Skerry-Ryan"
            << "Owen Williams"
            << "Daniel Sch&uuml;rmann"
            << "S&eacute;bastien Blaisot"
            << "ronso0"
            << "Jan Holthuis"
            << "Nikolaus Einhauser"
            << "Ferran Pujol Camins"
            << "J&ouml;rg Wartenberg"
            << "Fredrik Wieczerkowski"
            << "Maarten de Boer"
            << "Antoine Colombier"
            << "Evelynne Veys";

    // This list should contains all contributors committed
    // code to the Mixxx core within the past two years.
    // New Contributors are added at the end.
    QStringList recentContributors;
    recentContributors
            << "Be"
            << "Uwe Klotz"
            << "D&aacute;vid Szak&aacute;llas"
            << "Christian"
            << "Geraldo Nascimento"
            << "Allen Wittenauer"
            << "Raphael Bigal"
            << "Filok"
            << "Tobias Oszlanyi (OsZ)"
            << "Neil Naveen"
            << "David Chocholat&yacute;"
            << "Jakob Leifhelm"
            << "Jakub Kopa&nacute;ko"
            << "Saksham Hans"
            << "Robbert van der Helm"
            << "Andrew Burns"
            << "Michael Wigard"
            << "Alexandre Bique"
            << "Milkii Brewster"
            << "djantti"
            << "Eugene Erokhin"
            << "Ben Duval"
            << "Nicolau Leal Werneck"
            << "David Guglielmi"
            << "Chris H. Meyer"
            << "Mariano Ntrougkas"
            << "Daniel Fernandes"
            << "Gr&eacute;goire Locqueville"
            << "grizeldi"
            << "codingspiderfox"
            << "Ashnidh Khandelwal"
            << "Sergey"
            << "Raphael Quast"
            << "Christophe Henry"
            << "Lukas Waslowski"
            << "Marcin Cie&#x15B;lak" // &#x15B; = &sacute; in HTML 5.0
            << "HorstBaerbel"
            << "gqzomer"
            << "Bacadam"
            << "Leon Eckardt"
            << "Th&eacute;odore Noel"
            << "Aquassaut"
            << "Morgan Nunan"
            << "FrankwaP"
            << "Markus Kohlhase"
            << "Daniel Fernandes"
            << "Frank Grimy"
            << "Al Hadebe"
            << "Emilien Colombier"
            << "DJ aK"
            << "Sam Whited"
            << "Ryan Bell"
            << "Nicolas Parlant"
            << "Ralf Pachali"
            << "Patrick Taels"
            << "armaan"
            << "Karam Assany"
            << "Anmol Mishra"
            << "Alec Peng"
            << "Arthur Vimond"
            << "Johan Jnn"
            << "Shiraz McClennon"
            << "Lubosz Sarnecki"
            << "Falk Ebert"
            << "13dixi37"
            << "endcredits33"
            << "Jakob Stolberg"
            << "evoixmr"
            << "Jos&eacute; Carlos Cuevas"
            << "cucucat"
            << "Hetarth Jodha";

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
            << "Nicu Badescu"
            << "Sean Pappalardo"
            << "S. Brandt";

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
            << "Thomas Jarosch"
            << "Nico Schl&ouml;mer"
            << "Joan Marc&egrave; i Igual"
            << "Stefan Weber"
            << "Matthew Nicholson"
            << "Jamie Gifford"
            << "Sebastian Reu&szlig;e"
            << "Pawe&#322; Goli&#324;ski"
            << "beenisss"
            << "Tuukka Pasanen"
            << "Josep Maria Antol&iacute;n Segura"
            << "St&eacute;phane Lepin"
            << "Bernd Binder"
            << "Pradyuman"
            << "Nik Martin"
            << "Kerrick Staley"
            << "Raphael Graf"
            << "YunQiang Su"
            << "Melissa"
            << "Ned Haughton"
            << "Cristiano Lacerda"
            << "Ketan Lambat"
            << "Edward Kigwana"
            << "Simon Harst"
            << "J&eacute;r&ocirc;me Blanchi"
            << "Chris Hills"
            << "David Lowenfels"
            << "Matthieu Bouron"
            << "Nathan Korth"
            << "Edward Millen"
            << "Frank Breitling"
            << "Albert Aparicio"
            << "Pierre Le Gall"
            << "David Baker"
            << "Justin Kourie"
            << "Waylon Robertson"
            << "Ball&oacute; Gy&ouml;rgy"
            << "Pino Toscano"
            << "Alexander Horner"
            << "Michael Ehlen"
            << "Alice Midori"
            << "h67ma"
            << "Vincent Duez-Dellac"
            << "Somesh Metri"
            << "Doteya"
            << "olafklingt"
            << "Nino MP"
            << "Daniel Poelzleithner"
            << "luzpaz"
            << "Sebastian Hasler"
            << "Kshitij Gupta"
            << "Evan Dekker"
            << "Harshit Maurya"
            << "Janek Fischer"
            << "Matthias Beyer"
            << "Kristiyan Katsarov"
            << "Sanskar Bajpai"
            << "Javier Vilarroig"
            << "Gary Tunstall"
            << "Viktor Gal"
            << "Maty&aacute;&scaron; Bobek"
            << "Mr. Rincewind"
            << "Stefan N&uuml;rnberger"
            << "motific"
            << "Philip Gottschling"
            << "Adam Szmigin"
            << "tcoyvwac"
            << "Fatih Emre YILDIZ"
            << "Javier Vilalta"
            << "Fabian Wolter"
            << "Matteo Gheza"
            << "Michael Bacarella"
            << "Bilal Ahmed Karbelkar"
            << "Alice Psykose"
            << "Florian Goth"
            << "Chase Durand"
            << "John Last";

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
    if (std::rand() % 6) {
        if (!Color::isDimColor(palette().text().color())) {
            btnDonate->setIcon(QIcon(":/images/heart_icon_light.svg"));
        } else {
            btnDonate->setIcon(QIcon(":/images/heart_icon_dark.svg"));
        }
    } else {
        btnDonate->setIcon(QIcon(":/images/heart_icon_rainbow.svg"));
    }
    btnDonate->setText(tr("Donate"));
    connect(btnDonate, &QPushButton::clicked, this, [] {
        mixxx::DesktopHelper::openUrl(QUrl(MIXXX_DONATE_URL));
    });

    connect(buttonBox, &QDialogButtonBox::accepted, this, &DlgAbout::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &DlgAbout::reject);
}
