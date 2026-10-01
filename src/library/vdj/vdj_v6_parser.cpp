#include "vdj_v6_parser.h"

using namespace vdj;

VdjV6Parser::VdjV6Parser(QIODevice* pDev) : m_xml(pDev) {}

std::optional<int> VdjV6Parser::optInt(const QString& stringValue) {
	bool ok = false;
	int value = stringValue.toInt(&ok);
	return ok ? std::optional<int>(value) : std::nullopt;
}
std::optional<qint64> VdjV6Parser::optI64(const QString& stringValue) {
	bool ok = false;
	qint64 value = stringValue.toLongLong(&ok);
	return ok ? std::optional<qint64>(value) : std::nullopt;
}
std::optional<double> VdjV6Parser::optDouble(const QString& stringValue) {
	bool ok = false;
	double value = stringValue.toDouble(&ok);
	return ok ? std::optional<double>(value) : std::nullopt;
}
bool VdjV6Parser::allDigits(const QString& stringValue) {
	for (auto character : stringValue) {
		if (!character.isDigit()) return false;
	}
	return true;
}
VdjTime VdjV6Parser::parseVdjTime(const QString& stringValue) {
	VdjTime timeResult;
	if (stringValue.isEmpty()) return timeResult;
	if (stringValue.size() == 10 && allDigits(stringValue)) {
		timeResult.kind = VdjTime::Kind::YmdhmString;
		timeResult.ymdhm = stringValue;
		return timeResult;
	}
	bool ok = false;
	qint64 longlongValue = stringValue.toLongLong(&ok);
	if (ok) {
		timeResult.kind = VdjTime::Kind::LegacySignedInt;
		timeResult.legacyValue = longlongValue;
	}
	return timeResult;
}

bool VdjV6Parser::parse(Database& db, QString* pErrorString) {
	while (!m_xml.atEnd()) {
		auto token = m_xml.readNext();
		if (token == QXmlStreamReader::StartElement) {
			const auto localname = m_xml.name();
			if (localname == QLatin1String("VirtualDJ_Database")) {
				auto attrs = m_xml.attributes();
				db.version = attrs.value("Version").toString();
			}
			else if (localname == QLatin1String("Song")) {
				Track track;
				readSongAttributes(track);
				if (!readSongBody(track)) {
					if (pErrorString) {
						*pErrorString = QStringLiteral("Malformed <Song> body");
					}
					return false;
				}
				// Key-choice: Infos.KeyTag > FAME.Key (if present)
				if (track.key.isEmpty()) {
					// do nothing - for now
				}
				db.tracks.push_back(std::move(track));
			}
			else if (localname == QLatin1String("FavoriteFolder")) {
				readFavoriteFolder(db);
			}
			else {
				// unknown node under root
				skipElement();
			}
		}
	}
	if (m_xml.hasError()) {
		if (pErrorString) {
			*pErrorString = m_xml.errorString();
		}
		return false;
	}
	return true;
}

void VdjV6Parser::readSongAttributes(Track& track) {
	auto attrs = m_xml.attributes();
	track.filePath = attrs.value("FilePath").toString();
	track.fileSize = optI64(attrs.value("FileSize").toString());
}

bool VdjV6Parser::readSongBody(Track& track) {
	while (!m_xml.atEnd()) {
		auto token = m_xml.readNext();
		if (token == QXmlStreamReader::EndElement && m_xml.name() == QLatin1String("Song"))
			return true;

		if (token != QXmlStreamReader::StartElement) continue;

		const auto ln = m_xml.name();
		if (ln == QLatin1String("Display")) { 
			readDisplayNode(track); 
		}
		else if (ln == QLatin1String("Infos")) {
			readInfosNode(track);
		}
		else if (ln == QLatin1String("BPM")) {
			readBpmNode(track);
		}
		else if (ln == QLatin1String("FAME")) {
			readFameNode(track);
		}
		else if (ln == QLatin1String("Automix")) {
			readAutomixNode(track);
		}
		else if (ln == QLatin1String("Cue")) {
			readCueNode(track);
		}
		else if (ln == QLatin1String("Comment")) {
			readCommentNode(track);
		}
		else {
			skipElement(); // unknown child
		}
	}
	return false;
}

void VdjV6Parser::readDisplayNode(Track& track) {
	auto a = m_xml.attributes();
	// NB: in v6 is "Author" the artist
	const auto author = a.value("Author").toString();
	const auto title = a.value("Title").toString();
	if (!track.artist.size()) {
		track.artist = author;
	}
	if (!track.title.size()) {
		track.title = title;
	}

	const auto genre = a.value("Genre").toString();
	const auto album = a.value("Album").toString();
	const auto composer = a.value("Composer").toString();
	const auto year = a.value("Year").toString();
	const auto tag = a.value("Tag").toString();

	if (!track.genre.size()) {
		track.genre = genre;
	}
	if (!track.album.size()) {
		track.album = album;
	}
	if (!track.composer.size()) {
		track.composer = composer;
	}
	if (!track.year.size()) {
		track.year = year;
	}
	if (!track.tag.size()) {
		track.tag = tag;
	}

	if (!track.color) {
		track.color = optInt(a.value("Color").toString());
	}
	if (!track.cover) {
		track.cover = optInt(a.value("Cover").toString());
	}

	skipElement();
}

void VdjV6Parser::readInfosNode(Track& track) {
	auto attrs = m_xml.attributes();

	// Length in samples @ 44.1k → seconds
	if (!track.lengthSeconds) {
		if (auto songLengthValue = optI64(attrs.value("SongLength").toString()))
			track.lengthSeconds = samplesToSeconds(*songLengthValue);
	}

	if (!track.bitrateKbps) {
		track.bitrateKbps = optInt(attrs.value("Bitrate").toString());
	}
	if (!track.playCount) {
		track.playCount = optInt(attrs.value("PlayCount").toString());
	}
	if (!track.gain) {
		track.gain = optInt(attrs.value("Gain").toString());
	}

	track.firstSeen = parseVdjTime(attrs.value("FirstSeen").toString());
	track.firstPlay = parseVdjTime(attrs.value("FirstPlay").toString());
	track.lastPlay = parseVdjTime(attrs.value("LastPlay").toString());

	// KeyTag can be prefered to FAME.Key
	const auto keyTag = attrs.value("KeyTag").toString();
	if (!keyTag.isEmpty()) {
		track.key = keyTag;
	}

	// BpmTag (string/float/int). When present and no BPM from <BPM/>:
	if (!track.bpm) {
		const auto bpmTag = attrs.value("BpmTag").toString();
		if (!bpmTag.isEmpty()) {
			if (auto d = optDouble(bpmTag)) {
				track.bpm = *d;
			}
			else if (auto i = optInt(bpmTag)) {
				track.bpm = double(*i);
			}
		}
	}

	skipElement();
}

void VdjV6Parser::readBpmNode(Track& track) {
	auto attrs = m_xml.attributes();
	// v6 uses centi-BPM
	if (!track.bpm) {
		if (auto ci = optInt(attrs.value("Bpm").toString())) {
			track.bpm = bpmFromCenti(*ci);
		}
	}
	// Phase (grid offset) is v6-intern; no need to store
	skipElement();
}

void VdjV6Parser::readFameNode(Track& t) {
	auto attrs = m_xml.attributes();
	// if no "key" available in Infos.KeyTag, use FAME.Key
	if (t.key.isEmpty()) {
		const auto fameKey = attrs.value("Key").toString();
		if (!fameKey.isEmpty()) {
			t.key = fameKey;
		}
	}
	// Other elements are skipped
	skipElement();
}

void VdjV6Parser::readAutomixNode(Track& t) {
	auto attrs = m_xml.attributes();

	auto addMixPoi = [&](const char* pLabel, std::optional<qint64> pos, Poi::PoiType typeOverride = Poi::PoiType::Mix) {
		if (!pos) return;
		Poi poi;
		poi.type = typeOverride;
		poi.name = QString::fromLatin1(pLabel);
		poi.posUnit = PoiUnit::Samples441;
		poi.posValue = double(*pos); // raw samples; conversion to sec is done later if necessary
		t.pois.push_back(std::move(poi));
		};

	// Read fields (samples @ 44.1k)
	const auto CutStart = optI64(attrs.value("CutStart").toString());
	const auto CutEnd = optI64(attrs.value("CutEnd").toString());
	const auto FadeStart = optI64(attrs.value("FadeStart").toString());
	const auto FadeEnd = optI64(attrs.value("FadeEnd").toString());
	const auto RealStart = optI64(attrs.value("RealStart").toString());
	const auto RealEnd = optI64(attrs.value("RealEnd").toString());
	const auto TempoStart = optI64(attrs.value("TempoStart").toString());
	const auto TempoEnd = optI64(attrs.value("TempoEnd").toString());

	// Store known markers as PoiType::Mix with descriptive names:
	addMixPoi("Automix.CutStart", CutStart);
	addMixPoi("Automix.CutEnd", CutEnd);
	addMixPoi("Automix.FadeStart", FadeStart);
	addMixPoi("Automix.FadeEnd", FadeEnd);
	addMixPoi("Automix.RealStart", RealStart);
	addMixPoi("Automix.RealEnd", RealEnd);
	addMixPoi("Automix.TempoStart", TempoStart);
	addMixPoi("Automix.TempoEnd", TempoEnd);

	// Extra: synthesis of "AutomixIn"/"AutomixOut" for compatibility with modern POI-way:
	// In = (FadeStart ? FadeStart : RealStart), Out = (FadeEnd ? FadeEnd : RealEnd)
	if (FadeStart || RealStart) {
		addMixPoi("Automix.In", FadeStart ? FadeStart : RealStart, Poi::PoiType::AutomixIn);
	}
	if (FadeEnd || RealEnd) {
		addMixPoi("Automix.Out", FadeEnd ? FadeEnd : RealEnd, Poi::PoiType::AutomixOut);
	}

	skipElement();
}

void VdjV6Parser::readCueNode(Track& track) {
	auto attrs = m_xml.attributes();

	Poi poi;
	poi.type = Poi::PoiType::Hotcue;           // v6 <Cue> = hotcue-ish
	poi.cueslot = optInt(attrs.value("Num").toString());
	poi.number = optInt(attrs.value("Num").toString());
	poi.name = attrs.value("Name").toString();
	poi.posUnit = PoiUnit::Samples441;      // v6 uses samples for Pos
	if (auto s = optI64(attrs.value("Pos").toString())) {
		poi.posValue = double(*s);
	}

	track.pois.push_back(std::move(poi));
	skipElement();
}

void VdjV6Parser::readCommentNode(Track& track) {
	// store inner text (raw) as track.comment
	QString commentString;
	while (!m_xml.atEnd()) {
		auto token = m_xml.readNext();
		if (token == QXmlStreamReader::EndElement &&
				m_xml.name() == QLatin1String("Comment")) {
			break;
		}
		if (token == QXmlStreamReader::Characters ||
				token == QXmlStreamReader::EntityReference) {
			commentString += m_xml.text().toString();
		}
	}
	if (!commentString.isEmpty()) {
		track.comment = commentString;
	}
}

void VdjV6Parser::readFavoriteFolder(Database& out) {
	auto a = m_xml.attributes();
	FavoriteFolder folder;
	folder.name = a.value("Name").toString();
	folder.folderPath = a.value("FolderPath").toString();
	folder.order = optInt(a.value("Order").toString());
	skipElement();
	out.favorites.push_back(std::move(folder));
}

void VdjV6Parser::skipElement() {
	int depth = 1;
	while (!m_xml.atEnd() && depth > 0) {
		auto token = m_xml.readNext();
		if (token == QXmlStreamReader::StartElement) ++depth;
		else if (token == QXmlStreamReader::EndElement) --depth;
	}
}
