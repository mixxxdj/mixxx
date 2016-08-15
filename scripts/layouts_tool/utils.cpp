//
// Created by tomasito665 on 15-8-16.
//

#include <QProcess>
#include <QDebug>

#include "utils.h"

namespace utils {
    void clearTerminal() {
        bool termEnvVarDefined = QProcessEnvironment::systemEnvironment().contains("TERM");

        if (!termEnvVarDefined) {
            // TODO(Tomasito) Find a way of clearing the screen when TERM environment variable is not defined
            qDebug() << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
            return;
        }

        QProcess::execute("clear");
    }
}