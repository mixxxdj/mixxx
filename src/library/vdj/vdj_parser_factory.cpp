#include "vdj_parser_factory.h"
#include "vdj_modern_parser.h"
#include "vdj_v6_parser.h"

using namespace vdj;

namespace vdj {
	class V6Adapter final : public IDatabaseParser {
	public:
		explicit V6Adapter(QIODevice* d) : dev(d) {}
		bool parse(Database& out, QString* error) override {
			VdjV6Parser p(dev);
			return p.parse(out, error);
		}
	private:
		QIODevice* dev;
	};

	class ModernAdapter final : public IDatabaseParser {
	public:
		explicit ModernAdapter(QIODevice* d) : dev(d) {}
		bool parse(Database& out, QString* error) override {
			VdjModernParser p(dev);
			return p.parse(out, error);
		}
	private:
		QIODevice* dev;
	};

	// fast content sniff
	bool looksModernHead(const QByteArray& head) {
		const auto h = head.toLower();
		return h.contains("<poi") || h.contains("<tag") || h.contains("unit=\"beat\"");
	}

	std::unique_ptr<IDatabaseParser> vdj::makeParser(QIODevice* dev, VdjParserTypeEnum parserType) {
		switch (parserType) {
            case V6:
                return std::make_unique<V6Adapter>(dev);
            case Modern:
                return std::make_unique<ModernAdapter>(dev);
			}
        }


	std::unique_ptr<IDatabaseParser> vdj::makeParser(QIODevice* dev, const QFileInfo& fi) {
		QString dbVersionString = "Determined database version:";

		const auto base = fi.fileName().toLower();
		if (base.contains("local database v6")) {
			qDebug().noquote() << dbVersionString << "v6 (based upon filename)";
			return std::make_unique<V6Adapter>(dev);
		}

		// Peek a little to guess
		if (auto* f = qobject_cast<QFile*>(dev)) {
			auto pos = f->pos();
			const QByteArray head = f->peek(8192);
			// If nothing read (non-seekable), fall back to modern sniff later
			if (looksModernHead(head)) {
				qDebug().noquote() << dbVersionString << "Modern (header)";
				return std::make_unique<ModernAdapter>(dev);
			}
			// else: still ambiguous – try to read root Version quickly
			// (QXmlStreamReader on a dup device is more work; keep it simple)
			f->seek(pos);
		}

		// Default bias: old. If it fails in practice, swap default to V6.
		qDebug().noquote() << dbVersionString << "Modern (fallback)";
		return std::make_unique<ModernAdapter>(dev);
	}

} // namespace