#ifndef LAYOUTSFILEHANDLER_H
#define LAYOUTSFILEHANDLER_H

#include <QFile>
#include <QDir>
#include "layout.h"

typedef QList<QStringList> LayoutNamesData;
typedef KeyboardLayoutPointer (*GetLayout_t)(std::string layoutName);

class LayoutsFileHandler {
  public:
    LayoutsFileHandler();
    virtual ~LayoutsFileHandler();

    void open(QString& layoutsPath, QList<Layout>& layouts);
    void save(QFile& f, QList<Layout>& layouts);

  private:
    LayoutNamesData getLayoutNames(QFile& cppFile);

    // Prepends definitions to layouts.h in order for this tool to be able to compile
    // the layouts. It adds this:
    //
    //  struct KbdKeyChar {
    //      char16_t character;
    //      bool is_dead;
    //  };
    //
    //  typedef const KbdKeyChar (*KeyboardLayoutPointer)[2];
    void prependDefs(QFile& cppFile);

    // Appends function to layouts.h to retrieve layouts. Function could look like this:
    //
    // extern "C" KeyboardLayoutPointer getLayout(std::string layoutName) {
    //    if (layoutName == "en_US") return en_US;
    //    if (layoutName == "es_ES") return es_ES;
    //    if (layoutName == "fr_FR") return fr_FR;
    //    else {
    //        return nullptr;
    //    }
    // }
    //
    // If forInternUse is false, it doesn't add 'extern "C" '
    void appendGetLayoutsFunction(QFile& cppFile,
                                  const LayoutNamesData& layoutNames,
                                  bool forInternUse);

    // Remove sections surrounded by SKIP_HEAD and SKIP_TAIL comments
    void removeSkipParts(QFile& file);

    void compileLayoutsFile(const QString& cppPath, GetLayout_t& pFunction, void*& handle);

    void createHeaderFile(const QString& path);

    // Overwrite given file with QStringList where each QString represents one line.
    void overwriteFile(QFile& file, const QStringList& lines);

    // Append given QStringList to given file
    void appendToFile(QFile& file, const QStringList& lines);

    // Prepend given QStringList to given file
    void prependToFile(QFile& file, const QStringList& lines);
};

#endif // LAYOUTSFILEHANDLER_H
