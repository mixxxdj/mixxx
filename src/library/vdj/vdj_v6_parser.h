#pragma once
#include <QtCore>
#include "vdj_types.h"

namespace vdj {

    /// Parser for "VirtualDJ Local Database v6" styled XML.
    /// reads v6-structure and maps to normalised vdj::Database model.
    class VdjV6Parser {
    public:
        explicit VdjV6Parser(QIODevice* pDev);
        /// Parse XML in 'db'. Returns false on failure and sets 'errorString'.
        bool parse(Database& db, QString* pErrorString = nullptr);

    private:
        QXmlStreamReader m_xml;

        // helpers
        static std::optional<int>    optInt(const QString& stringValue);
        static std::optional<qint64> optI64(const QString& stringValue);
        static std::optional<double> optDouble(const QString& stringValue);

        static bool allDigits(const QString& stringValue);
        static VdjTime parseVdjTime(const QString& stringValue);

        // Song parsing
        void readSongAttributes(Track& track);        // Song attrs (FilePath, FileSize)
        bool readSongBody(Track& track);              // Children: Display/Infos/BPM/FAME/Automix/Cue/Comment

        void readDisplayNode(Track& track);           // <Display .../>
        void readInfosNode(Track& track);             // <Infos .../>
        void readBpmNode(Track& track);               // <BPM .../>
        void readFameNode(Track& track);              // <FAME .../>
        void readAutomixNode(Track& track);           // <Automix .../>
        void readCueNode(Track& track);               // <Cue .../>
        void readCommentNode(Track& track);           // <Comment>...</Comment>

        void readFavoriteFolder(Database& db);        // <FavoriteFolder .../>

        // plumbing
        void skipElement();                           // consume current start-element until matching end
    };

} // namespace vdj
