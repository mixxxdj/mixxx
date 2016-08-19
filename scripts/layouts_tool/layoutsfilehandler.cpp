#include <iostream>
#include <dlfcn.h>
#include <QDebug>

#include "layoutsfilehandler.h"

const QString LayoutsFileHandler::INDENT = "    ";

const QStringList LayoutsFileHandler::HEADER_COMMENT = QStringList()
        << "/*************************************************************************"
        << "** This code was generated with layoutstool                             **"
        << "**                                                                      **"
        << "** WARNING: Changes to this file may be overridden by the tool!         **"
        << "**                                                                      **"
        << "**          If you want to add or delete layouts, please use the tool.  **"
        << "**          Layoutstool can be found in mixxx/scripts/layouts_tool and  **"
        << "**          build with build.sh. The executable will be placed in       **"
        << "**          mixxx/scripts/layouts_tool/bin                              **"
        << "**                                                                      **"
        << "** NOTE:    Layoutstool does only work on Linux (make sure you have GCC **"
        << "**          and CMake installed in order to successfully build and run  **"
        << "**          the tool.                                                   **"
        << "*************************************************************************/";

const QString LayoutsFileHandler::SKIP_HEAD = "/* @SKIP */";
const QString LayoutsFileHandler::SKIP_TAIL = "/* @/SKIP */";

/******************
*** Header file ***
******************/

const QStringList LayoutsFileHandler::INCLUDE_GUARD_HEAD = QStringList()
        << "#ifndef LAYOUTS_H"
        << "#define LAYOUTS_H";

const QString LayoutsFileHandler::KBDKEYCHAR_PROTOTYPE =
        "struct KbdKeyChar;";

const QString LayoutsFileHandler::KBDLAYOUTPOINTER_TYPEDEF =
        "typedef const KbdKeyChar (*KeyboardLayoutPointer)[2];";

const QString LayoutsFileHandler::GETLAYOUT_FUNCTION_PROTOTYPE =
        "KeyboardLayoutPointer getLayout(std::string layoutName);";

const QString LayoutsFileHandler::INCLUDE_GUARD_TAIL = "#endif // LAYOUTS_H";


/**************************
*** Implementation file ***
**************************/

const QString LayoutsFileHandler::INCLUDE_STRING = "#include <string>";

const QStringList LayoutsFileHandler::KBDKEYCHAR_IMPLEMENTATION = QStringList()
        << "struct KbdKeyChar {"
        << INDENT + "char16_t character;"
        << INDENT + "bool is_dead;"
        << "};";

LayoutsFileHandler::LayoutsFileHandler() {}

LayoutsFileHandler::~LayoutsFileHandler() {}

void LayoutsFileHandler::open(QString& cppPath, QList<Layout>& layouts) {
    if (cppPath.isEmpty()) {
        return;
    }

    QFileInfo check_file(cppPath);
    if (!check_file.exists() || !check_file.isFile()) {
        cppPath = "";
        qDebug() << "Not loading layouts. Path doesn't exist: " << cppPath;
        return;
    }

    QFile f(cppPath);
    LayoutNamesData layoutNames = getLayoutNames(f);

    // Remove old code and add new code in order for the file to compile
    removeSkipParts(f);
    prependDefs(f);
    appendGetLayoutsFunction(f, layoutNames, true);

    // Compile the file and get the function pointer to the getLayout
    // function (and the handle to be able to close it when we are done)
    GetLayout_t getLayout = nullptr;
    void* handle = nullptr;
    compileLayoutsFile(cppPath,
                       getLayout,
                       handle);

    for (const QStringList& names : layoutNames) {
        const QString& varName = names[0];
        const QString& name = names[1];
        KeyboardLayoutPointer layoutData = getLayout(varName.toLatin1().data());

        // Construct layout object and append to layouts
        Layout layout(varName, name, layoutData);
        layouts.append(layout);
    }

    // Close layouts library
    qDebug() << "Closing layouts library...\n";
    dlclose(handle);

    // Write a clean version of the loaded layouts (remove definitions
    // noise added in prependDefs)
    save(f, layouts);
}

void LayoutsFileHandler::compileLayoutsFile(const QString& cppPath, GetLayout_t& pGetLayoutFn, void*& handle) {
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

    // Remove so file
    system(QString("rm %1").arg(soPath).toLatin1().data());
}

void LayoutsFileHandler::prependDefs(QFile& cppFile) {
    QStringList lines;

    // Include string for getLayout(std::string name) : KeyboardLayoutPointer
    lines.append(INCLUDE_STRING);

    // Add KbdKeyChar struct declaration
    lines.append(KBDKEYCHAR_IMPLEMENTATION);

    // Add KeyboardLayoutPointer declaration
    lines.append(KBDLAYOUTPOINTER_TYPEDEF);

    // Load each line of file into QStringList
    QStringList fileLines;
    if (cppFile.open(QIODevice::ReadOnly)) {
        QTextStream in(&cppFile);
        while (!in.atEnd()) {
            fileLines.append(in.readLine());
        }
        cppFile.close();
    }

    lines += fileLines;

    // Overwrite file with prepended definitions
    if (cppFile.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        QTextStream stream(&cppFile);
        for (auto it = lines.begin(); it != lines.end(); ++it) {
            stream << *it << "\n";
        }
        cppFile.close();
    }
}

void LayoutsFileHandler::appendGetLayoutsFunction(QFile& cppFile,
                                                  const LayoutNamesData& layoutNames,
                                                  bool forInternUse) {

    // Generate new 'getLayout' function definition
    QStringList fnLines;
    fnLines.append("");
    fnLines.append(SKIP_HEAD);
    fnLines.append(QString("%1KeyboardLayoutPointer getLayout(std::string layoutName) {")
                           .arg(forInternUse ? ("extern \"C\" ") : "")
    );

    // Create one if-statement per layout
    for (const QStringList& names : layoutNames)
        fnLines.append(
                QString(INDENT + "if (layoutName == \"%1\") return layouts::%1;").arg(names[0])
        );

    // Create 'else' if there is an 'if', if no 'if', return nullptr directly
    if (!layoutNames.isEmpty()) {
        fnLines.append(INDENT + "else {");
        fnLines.append(INDENT + INDENT + "return nullptr;");
        fnLines.append(INDENT + "}");
    } else {
        fnLines.append(INDENT + "// There are no layouts in this file, so I can't return any");
        fnLines.append(INDENT + "return nullptr;");
    }
    fnLines.append("}");
    fnLines.append(SKIP_TAIL);

    // Rewrite file from buffer
    if (cppFile.open(QIODevice::ReadWrite | QIODevice::Append)) {
        QTextStream stream(&cppFile);
        for (auto it = fnLines.begin(); it != fnLines.end(); ++it) {
            stream << *it << "\n";
        }
        cppFile.close();
    }
}

LayoutNamesData LayoutsFileHandler::getLayoutNames(QFile& cppFile) {
    LayoutNamesData names;

    if (cppFile.open(QIODevice::ReadOnly)) {
        QRegExp bracketRegex("\\[");
        QString type = "static const KbdKeyChar";
        QTextStream in(&cppFile);

        QString prevLine;
        while (!in.atEnd()) {

            // Get current line and trim to get rid of indentation
            QString line = in.readLine().trimmed();

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

void LayoutsFileHandler::save(QFile& f, QList<Layout>& layouts) {
    QStringList lines;

    // Assemble header file
    const QFileInfo fileInfo(f.fileName());
    const QString name = fileInfo.baseName();
    QDir dir = fileInfo.absoluteDir();
    QString headerName = name + ".h";
    QString hPath = dir.filePath(headerName);
    createHeaderFile(hPath);

    // Add comment telling that this file was generated
    lines.append(HEADER_COMMENT);
    lines.append("");

    // Include header file (avoid this section from compiling when loading next time)
    lines.append(SKIP_HEAD);
    lines.append(QString("#include \"%1\"").arg(headerName));
    lines.append("");
    lines.append(KBDKEYCHAR_IMPLEMENTATION);
    lines.append(SKIP_TAIL);
    lines.append("");

    // Open namespace
    lines.append("namespace layouts {");

    // Add layouts and add indentation for namespace
    for (const Layout& layout : layouts) {
        QStringList layoutCode = layout.generateCode();
        for (const QString& line : layoutCode) {
            lines.append(INDENT + line);
        }

        if (&layout != &layouts.last()) {
            lines.append("");
        }
    }

    // Close namespace
    lines.append("}");

    if (f.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        QTextStream stream(&f);
        for (auto it = lines.begin(); it != lines.end(); ++it) {
            stream << *it << "\n";
        }
        f.close();
    }

    appendGetLayoutsFunction(f, getLayoutNames(f), false);
}

void LayoutsFileHandler::createHeaderFile(const QString& path) {
    QFile f(path);
    QStringList lines;

    // Assemble header file body
    lines.append(HEADER_COMMENT);
    lines.append("");
    lines.append(INCLUDE_GUARD_HEAD);
    lines.append("");
    lines.append(INCLUDE_STRING);
    lines.append("");
    lines.append(KBDKEYCHAR_PROTOTYPE);
    lines.append(KBDLAYOUTPOINTER_TYPEDEF);
    lines.append(GETLAYOUT_FUNCTION_PROTOTYPE);
    lines.append("");
    lines.append(INCLUDE_GUARD_TAIL);

    if (f.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        QTextStream stream(&f);
        for (auto it = lines.begin(); it != lines.end(); ++it) {
            stream << *it << "\n";
        }
        f.close();
    }
}

void LayoutsFileHandler::removeSkipParts(QFile& f) {
    QStringList code;

    // Store all lines in given file into code filtering out
    // lines in between SKIP_HEAD and SKIP_TAIL
    if (f.open(QIODevice::ReadOnly)) {
        QTextStream in(&f);
        bool skip = false;
        while (!in.atEnd()) {
            QString line = in.readLine();
            QString trimmedLine = line.trimmed();

            // If we are within beginLayoutComment and endLayoutComment,
            // do not add those lines in
            if (trimmedLine.startsWith(SKIP_HEAD)) skip = true;
            if (!skip) code.append(line);
            if (trimmedLine.startsWith(SKIP_TAIL)) skip = false;
        }
        f.close();
    }

    // Rewrite file
    if (f.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        QTextStream stream(&f);
        for (auto it = code.begin(); it != code.end(); ++it) {
            stream << *it << "\n";
        }
        f.close();
    }
}
