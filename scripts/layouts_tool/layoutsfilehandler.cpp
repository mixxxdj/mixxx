#include <iostream>
#include <dlfcn.h>
#include <QDebug>

#include "layoutsfilehandler.h"
#include "utils.h"

// Code snippets used for code generation
namespace {

    // Indentation that will be used for the generated file
    const QString kIndent = "    ";
    const QStringList kHeaderComment = QStringList()
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

    const QString kIncludeString = "#include <string>";

    const QStringList kIncludeYVals = QStringList()
            << "#ifdef __WINDOWS__"
            << "#include <yvals.h>"
            << "#endif";

    // Sections that need to be skipped the next time that
    // the file is loaded by this tool are surrounded by
    // these 'skip' comments.
    const QString kSkipCommentHead = "/* @SKIP */";
    const QString kSkipCommentTail = "/* @/SKIP */";

    // Include guards used for the header file
    const QStringList kIncludeGuardHead = QStringList()
            << "#ifndef LAYOUTS_H"
            << "#define LAYOUTS_H";
    const QString kIncludeGuardTail = "#endif // LAYOUTS_H";

    // KbdKeyChar struct forward declaration and declaration
    const QStringList kKbdCharImplementation = QStringList()
            << "struct KbdKeyChar {"
            << kIndent + "char16_t character;"
            << kIndent + "bool isDead;"
            << "};";

    const QString kKbdLayoutPointerTypedef = "typedef const KbdKeyChar (*KeyboardLayoutPointer)[2];";
    const QString kGetLayoutFunctionPrototype = "KeyboardLayoutPointer getLayout(std::string layoutName);";
}


LayoutsFileHandler::LayoutsFileHandler() {}

LayoutsFileHandler::~LayoutsFileHandler() {}

void LayoutsFileHandler::open(QString& cppPath, QList<Layout>& layouts) {
    if (cppPath.isEmpty()) {
        return;
    }

    QFileInfo check_file(cppPath);
    if (!check_file.exists() || !check_file.isFile()) {
        qDebug() << "Not loading layouts. Path doesn't exist: " << cppPath;
    }

    QFile f(cppPath);
    QList<LayoutNamePair> layoutNames = getLayoutNames(f);

    // Remove old code and add new code in order for the file to compile
    removeSkipParts(f);
    prependDefs(f);
    appendGetLayoutsFunction(f, layoutNames, true);

    qDebug() << "Layout names:";
    for (LayoutNamePair layoutNameData : layoutNames) {
        qDebug() << "VarName: " << layoutNameData.varName << ", Name: " << layoutNameData.name;
    }

    // Compile the file and get the function pointer to the getLayout
    // function (and the handle to be able to close it when we are done)
    GetLayout_t getLayout = nullptr;
    void* handle = nullptr;
    compileLayoutsFile(cppPath,
                       getLayout,
                       handle);

    for (const LayoutNamePair& layoutNamePair : layoutNames) {
        const QString& varName = layoutNamePair.varName;
        const QString& name = layoutNamePair.name;

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
    QStringList code;
    code << kIncludeString
         << kKbdCharImplementation
         << kKbdLayoutPointerTypedef;
    prependToFile(cppFile, code);
}

void LayoutsFileHandler::appendGetLayoutsFunction(QFile& cppFile,
                                                  const QList<LayoutNamePair>& layoutNames,
                                                  bool forInternUse) {
    // Generate new 'getLayout' function definition
    QStringList fnLines;
    fnLines << ""
            << kSkipCommentHead
            << QString("%1KeyboardLayoutPointer getLayout(std::string layoutName) {")
                    .arg(forInternUse ? ("extern \"C\" ") : "");

    // Add one if-statement per layout
    for (const LayoutNamePair& names : layoutNames) {
        fnLines << QString(kIndent + "if (layoutName == \"%1\") return layouts::%1;")
                .arg(names.varName);
    }

    // Add 'else' if there is an 'if', if no 'if', return nullptr directly
    if (!layoutNames.isEmpty()) {
        fnLines << kIndent + "else {"
                << kIndent + kIndent + "return nullptr;"
                << kIndent + "}";
    } else {
        fnLines << kIndent + "// There are no layouts in this file, so I can't return any"
                << kIndent + "return nullptr;";
    }
    fnLines << "}"
            << kSkipCommentTail;

    appendToFile(cppFile, fnLines);
}

QList<LayoutNamePair> LayoutsFileHandler::getLayoutNames(QFile& cppFile) {
    QList<LayoutNamePair> names;

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

            QString variableName = line;
            QString name = prevLine.startsWith("//") ? prevLine.mid(3) : "";

            // Create QStringList and append it to names
            LayoutNamePair currentLayoutNames = {variableName, name};
            names.append(currentLayoutNames);

            prevLine = line;
        }
        cppFile.close();
    }
    return names;
}

void LayoutsFileHandler::save(QFile& f, QList<Layout>& layouts) {
    QStringList code;

    // Assemble header file
    const QFileInfo fileInfo(f.fileName());
    const QString name = fileInfo.baseName();
    QDir dir = fileInfo.absoluteDir();
    QString headerName = name + ".h";
    QString hPath = dir.filePath(headerName);
    createHeaderFile(hPath);

    code
            // Add comment telling that this file was generated
            << kHeaderComment
            << ""
            << kSkipCommentHead

            // Include header file
            << QString("#include \"%1\"").arg(headerName)
            << ""

            // Include yvals for char16_t support on Visual Studio 2013
            << kIncludeYVals
            << ""
            << kSkipCommentTail
            << "";

    // Open namespace
    code << "namespace layouts {";

    // Add layouts and add indentation for namespace
    for (const Layout& layout : layouts) {
        QStringList layoutCode = generateCodeForLayout(layout);
        for (const QString& line : layoutCode) {
            code << kIndent + line;
        }

        if (&layout != &layouts.last()) {
            code << "";
        }
    }

    // Close namespace
    code << "}";

    overwriteFile(f, code);
    appendGetLayoutsFunction(f, getLayoutNames(f), false);
}

void LayoutsFileHandler::createHeaderFile(const QString& path) {
    QFile f(path);
    QStringList code;

    // Assemble header file body
    code << kHeaderComment
         << ""
         << kIncludeGuardHead
         << ""
         << kIncludeString
         << ""
         << kKbdCharImplementation
         << ""
         << kKbdLayoutPointerTypedef
         << kGetLayoutFunctionPrototype
         << ""
         << kIncludeGuardTail;

    overwriteFile(f, code);
}

void LayoutsFileHandler::removeSkipParts(QFile& f) {
    QStringList code;

    // Store all lines in given file into code filtering out
    // lines in between kSkipCommentHead and kSkipCommentTail
    if (f.open(QIODevice::ReadOnly)) {
        QTextStream in(&f);
        bool skip = false;
        while (!in.atEnd()) {
            QString line = in.readLine();
            QString trimmedLine = line.trimmed();

            // If we are within kSkipCommentHead and kSkipCommentTail,
            // do not add those lines in
            if (trimmedLine.startsWith(kSkipCommentHead)) skip = true;
            if (!skip) code << line;
            if (trimmedLine.startsWith(kSkipCommentTail)) skip = false;
        }
        f.close();
    }
    overwriteFile(f, code);
}

void LayoutsFileHandler::overwriteFile(QFile &file, const QStringList &lines) {
    if (file.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        QTextStream stream(&file);
        for (auto it = lines.begin(); it != lines.end(); ++it) {
            stream << *it << "\n";
        }
        file.close();
    }
}

void LayoutsFileHandler::appendToFile(QFile &file, const QStringList &lines) {
    if (file.open(QIODevice::ReadWrite | QIODevice::Append)) {
        QTextStream stream(&file);
        for (auto it = lines.begin(); it != lines.end(); ++it) {
            stream << *it << "\n";
        }
        file.close();
    }
}

void LayoutsFileHandler::prependToFile(QFile &file, const QStringList &lines) {
    // Load each line of file into QStringList
    QStringList codeInFile;
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            codeInFile << in.readLine();
        }
        file.close();
    }
    overwriteFile(file, lines + codeInFile);
}

QStringList LayoutsFileHandler::generateCodeForLayout(const Layout &layout) {
    QStringList code;

    code << QString("// %1").arg(layout.m_name);

    code << QString("static const KbdKeyChar %1[%2][2] = {")
            .arg(layout.m_variableName, QString::number(kLayoutLen));

    for (int i = 0; i < kLayoutLen; i++) {
        int keycode = utils::layoutIndexToKeycode(i);
        QString keyName = utils::keycodeToKeyname(keycode);

        // If this key is the first key of the row, place an extra white
        // line and a comment telling which row we are talking about
        bool firstOfRow = keycode == TLDE || keycode == AD01 || keycode == AC01 || keycode == LSGT;
        if (firstOfRow) {
            QString rowName;
            if (keycode == TLDE)      rowName = "Digits row";
            else if (keycode == AD01) rowName = "Upper row";
            else if (keycode == AC01) rowName = "Home row";
            else if (keycode == LSGT) rowName = "Lower row";

            if (i > 0) code << "";
            code << QString("%1// %2").arg(kIndent + kIndent, rowName);
        }

        const KbdKeyChar& keyCharNoMods = layout.m_data[i][0];
        const KbdKeyChar& keyCharShift = layout.m_data[i][1];

        QString line = QString("%1/* %2 */ ").arg(kIndent + kIndent, keyName);
        line += QString("{%1, %2}").arg(
                utils::createKbdKeyCharLiteral(keyCharNoMods),
                utils::createKbdKeyCharLiteral(keyCharShift)
        );

        // If not last, place a separation comma
        if (i < kLayoutLen - 1) {
            line += ",";
        }
        code << line;
    }
    code << "};";
    return code;
}
