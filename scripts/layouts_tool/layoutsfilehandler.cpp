#include <iostream>
#include <dlfcn.h>
#include <QDebug>

#include "layoutsfilehandler.h"

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
    const QString kKbdCharPrototype = "struct KbdKeyChar;";
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
        const QString& variableName = names[0];
        const QString& name = names[1];
        KeyboardLayoutPointer layoutData = getLayout(variableName.toLatin1().data());

        // Construct layout object and append to layouts
        Layout layout(variableName, name, layoutData);
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

    // Load each line of file into QStringList
    QStringList codeInFile;
    if (cppFile.open(QIODevice::ReadOnly)) {
        QTextStream in(&cppFile);
        while (!in.atEnd()) {
            codeInFile << in.readLine();
        }
        cppFile.close();
    }

    code += codeInFile;

    // Overwrite file with prepended definitions
    if (cppFile.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        QTextStream stream(&cppFile);
        for (auto it = code.begin(); it != code.end(); ++it) {
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
    fnLines << ""
            << kSkipCommentHead
            << QString("%1KeyboardLayoutPointer getLayout(std::string layoutName) {")
                    .arg(forInternUse ? ("extern \"C\" ") : "");

    // Add one if-statement per layout
    for (const QStringList& names : layoutNames)
        fnLines << QString(kIndent + "if (layoutName == \"%1\") return layouts::%1;").arg(names[0]);

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

            QString variableName = line;
            QString name = prevLine.startsWith("//") ? prevLine.mid(3) : "";

            // Create QStringList and append it to names
            QStringList currentLayoutNames;
            currentLayoutNames.append(variableName);
            currentLayoutNames.append(name);
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

            // KbdKeyChar struct implementation
            << kKbdCharImplementation
            << kSkipCommentTail
            << "";

    // Open namespace
    code << "namespace layouts {";

    // Add layouts and add indentation for namespace
    for (const Layout& layout : layouts) {
        QStringList layoutCode = layout.generateCode();
        for (const QString& line : layoutCode) {
            code << kIndent + line;
        }

        if (&layout != &layouts.last()) {
            code << "";
        }
    }

    // Close namespace
    code << "}";

    if (f.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        QTextStream stream(&f);
        for (auto it = code.begin(); it != code.end(); ++it) {
            stream << *it << "\n";
        }
        f.close();
    }

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
         << kKbdCharPrototype
         << kKbdLayoutPointerTypedef
         << kGetLayoutFunctionPrototype
         << ""
         << kIncludeGuardTail;

    if (f.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        QTextStream stream(&f);
        for (auto it = code.begin(); it != code.end(); ++it) {
            stream << *it << "\n";
        }
        f.close();
    }
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

    // Rewrite file
    if (f.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        QTextStream stream(&f);
        for (auto it = code.begin(); it != code.end(); ++it) {
            stream << *it << "\n";
        }
        f.close();
    }
}
