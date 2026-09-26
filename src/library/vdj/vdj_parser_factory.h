#pragma once
#include <QtCore>
#include "vdj_types.h"

// Forward decls of concrete parsers
namespace vdj {
    class VdjV6Parser;       
    class VdjModernParser;   

    enum VdjParserTypeEnum {
        Modern,
        V6
    };


    class IDatabaseParser {
    public:
        virtual ~IDatabaseParser() = default;
        virtual bool parse(Database& out, QString* error = nullptr) = 0;
    };

    std::unique_ptr<IDatabaseParser> makeParser(QIODevice* dev, const QFileInfo& fi);
    std::unique_ptr<IDatabaseParser> makeParser(QIODevice* dev, VdjParserTypeEnum parserType);

} // namespace vdj
