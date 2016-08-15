#include <QProcess>
#include <QDebug>

#include "utils.h"
#include "defs.h"

namespace utils {
    void clearTerminal() {
        bool termEnvVarDefined = QProcessEnvironment::systemEnvironment().contains("TERM");

        if (!termEnvVarDefined) {
            // TODO(Tomasito) Find a way of clearing the screen when TERM environment variable is not defined, that is
            // ...            different than my current approach :)
            qDebug() << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
            return;
        }

        QProcess::execute("clear");
    }

    QString keycodeToKeyname(int keycode) {
        switch (keycode) {
            // Numeric row
            case TLDE: return "<TLDE>";
            case AE01: return "<AE01>";
            case AE02: return "<AE02>";
            case AE03: return "<AE03>";
            case AE04: return "<AE04>";
            case AE05: return "<AE05>";
            case AE06: return "<AE06>";
            case AE07: return "<AE07>";
            case AE08: return "<AE08>";
            case AE09: return "<AE09>";
            case AE10: return "<AE10>";
            case AE11: return "<AE11>";
            case AE12: return "<AE12>";

            // Upper row
            case AD01: return "<AD01>";
            case AD02: return "<AD02>";
            case AD03: return "<AD03>";
            case AD04: return "<AD04>";
            case AD05: return "<AD05>";
            case AD06: return "<AD06>";
            case AD07: return "<AD07>";
            case AD08: return "<AD08>";
            case AD09: return "<AD09>";
            case AD10: return "<AD10>";
            case AD11: return "<AD11>";
            case AD12: return "<AD12>";

            // Home row
            case AC01: return "<AC01>";
            case AC02: return "<AC02>";
            case AC03: return "<AC03>";
            case AC04: return "<AC04>";
            case AC05: return "<AC05>";
            case AC06: return "<AC06>";
            case AC07: return "<AC07>";
            case AC08: return "<AC08>";
            case AC09: return "<AC09>";
            case AC10: return "<AC10>";
            case AC11: return "<AC11>";
            case BKSL: return "<BKSL>";

            // Lower row
            case LSGT: return "<LSGT>";
            case AB01: return "<AB01>";
            case AB02: return "<AB02>";
            case AB03: return "<AB03>";
            case AB04: return "<AB04>";
            case AB05: return "<AB05>";
            case AB06: return "<AB06>";
            case AB07: return "<AB07>";
            case AB08: return "<AB08>";
            case AB09: return "<AB09>";
            case AB10: return "<AB10>";

            default: return "";
        }
    }

    QString inputLocaleName() {
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
        return QApplication::keyboardInputLocale().name();
#else
        // Use the default config for local keyboard
        QInputMethod* pInputMethod = QGuiApplication::inputMethod();
        return pInputMethod ? pInputMethod->locale().name() :
                QLocale(QLocale::English).name();
#endif
    }
}