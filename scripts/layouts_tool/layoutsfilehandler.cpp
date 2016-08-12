#include <iostream>
#include <dlfcn.h>
#include <QDebug>

#include "layoutsfilehandler.h"
#include "layout.h"

LayoutsFileHandler::LayoutsFileHandler() {}

LayoutsFileHandler::~LayoutsFileHandler() {}

void LayoutsFileHandler::open(const QString cppPath) {
    QFile f(cppPath);

    LayoutNamesData layoutNames = getLayoutNames(f);
    appendGetLayoutsFunction(f, layoutNames);

    GetLayout_t getLayout = nullptr;
    void *handle = nullptr;

    compileLayoutsFile(cppPath, getLayout, handle);

    // TODO(Tomasito) Create Layout objects into memory

    // Close layouts library
    qDebug() << "Closing layouts library...\n";
    dlclose(handle);
}

void LayoutsFileHandler::compileLayoutsFile(const QString cppPath, GetLayout_t &pGetLayoutFn, void *&handle) {
    const QDir dir = QFileInfo(cppPath).absoluteDir();
    const QString soPath = dir.filePath("layouts.so");

    // Compile layouts
    system(("gcc -std=c++11 -shared -o " + soPath + " -fPIC " + cppPath).toLatin1().data());

    // Open layouts
    qDebug() << "Opening " << soPath;
    handle = dlopen(soPath.toLatin1().data(), RTLD_LAZY);
    if (!handle) {
        qFatal("Could not open layouts library");
    }

    // Load getLayout function
    qDebug() << "Loading getLayout function symbol...";
    pGetLayoutFn = (GetLayout_t) dlsym(handle, "getLayout");
    if (!pGetLayoutFn) {
        qCritical() << dlerror();
        dlclose(handle);
        qFatal("Couldn't load symbol 'getLayout'");
    }
}

void LayoutsFileHandler::appendGetLayoutsFunction(QFile &cppFile, const LayoutNamesData &layoutNames) {
    QStringList fn;

    fn.append("");
    fn.append("/* @START GENERATED */");
    fn.append("extern \"C\" KeyboardLayoutPointer getLayout(std::string layoutName) {");
    for (QStringList names : layoutNames) {
        fn.append("    if (layoutName == \"" + names[0] + "\") return " + names[0] + ";");
    }
    fn.append("    else {");
    fn.append("        return nullptr;");
    fn.append("    }");
    fn.append("}");
    fn.append("/* @END GENERATED */");

    if (cppFile.open(QIODevice::ReadWrite | QIODevice::Append)) {
        QTextStream stream(&cppFile);
        for (QStringList::Iterator it = fn.begin(); it != fn.end(); ++it) {
            stream << *it << "\n";
        }
        cppFile.close();
    }
}

LayoutNamesData LayoutsFileHandler::getLayoutNames(QFile &cppFile) {
    LayoutNamesData names;

    if (cppFile.open(QIODevice::ReadOnly)) {
        QRegExp bracketRegex("\\[");
        QString type = "static const KbdKeyChar";
        QTextStream in(&cppFile);

        QString prevLine;
        while (!in.atEnd()) {
            QString line = in.readLine();

            if (!line.startsWith(type)) {
                prevLine = line;
                continue;
            }

            // Retrieve variable name by chopping line
            line = line.mid(24);
            line = line.mid(0, bracketRegex.indexIn(line));

            QString varName = line;
            QString name = prevLine.startsWith("//") ? prevLine.mid(3) : "";

            // Create QStringList and append it to names
            QStringList currentLayoutNames;
            currentLayoutNames.append(varName);
            currentLayoutNames.append(name);
            names.append(currentLayoutNames);

            prevLine = line;
        }

        cppFile.close();
    }

    return names;
}

void LayoutsFileHandler::save(QFile &file) {

}
