#pragma once
#include <QtCore>
#include "vdj_types.h"

namespace vdj {

    class VdjModernParser {
    public:
        explicit VdjModernParser(QIODevice* pDev);
        bool parse(Database& db, QString* pErrorString = nullptr);

    private:
        QXmlStreamReader xml;

        // helpers
        static std::optional<int>    optInt(const QString& stringValue);
        static std::optional<qint64> optI64(const QString& stringValue);
        static std::optional<double> optDouble(const QString& stringValue);
        static bool allDigits(const QString& stringValue);

        static VdjTime parseVdjTime(const QString& stringValue);

        // node readers
        bool readTrack(Track& track);
        void readScanNode(Track& track);            // <scan ... />
        void readInfoNode(Track& track);            // <info ... />
        void readTagNode(Track& track);             // <tag .../>
        void readPoiNode(Track& track);             // <poi .../>
        void readFavoriteFolder(Database& db);      // <FavoriteFolder .../>

        // plumbing
        void skipElement();
    };

} // namespace vdj
