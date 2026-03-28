#include "vdj_modern_parser.h"

using namespace vdj;

VdjModernParser::VdjModernParser(QIODevice* pDev) : xml(pDev) {}

std::optional<int> VdjModernParser::optInt(const QString& stringValue) {
	bool ok = false;
	int v = stringValue.toInt(&ok);
	return ok ? std::optional<int>(v) : std::nullopt;
}
std::optional<qint64> VdjModernParser::optI64(const QString& stringValue) {
	bool ok = false;
	qint64 v = stringValue.toLongLong(&ok);
	return ok ? std::optional<qint64>(v) : std::nullopt;
}
std::optional<double> VdjModernParser::optDouble(const QString& stringValue) {
	bool ok = false;
	double v = stringValue.toDouble(&ok);
	return ok ? std::optional<double>(v) : std::nullopt;
}
bool VdjModernParser::allDigits(const QString& stringValue) {
	for (auto character : stringValue) {
		if (!character.isDigit()) return false;
	}
	return true;
}

VdjTime VdjModernParser::parseVdjTime(const QString& stringValue) {
	VdjTime timevalue;
	if (stringValue.isEmpty()) return timevalue;
	if (stringValue.size() == 10 && allDigits(stringValue)) {
		timevalue.kind = VdjTime::Kind::YmdhmString;
		timevalue.ymdhm = stringValue;
		return timevalue;
	}
	bool ok = false; 
	qint64 longlongValue = stringValue.toLongLong(&ok);
	if (ok) { 
		timevalue.kind = VdjTime::Kind::LegacySignedInt; 
		timevalue.legacyValue = longlongValue; 
	}
	return timevalue;
}

bool VdjModernParser::parse(Database& db, QString* pErrorString) {
	/*
	Structure [ according to https://www.virtualdj.com/wiki/VDJ_database.html ]

	<VirtualDJ_Database @Version>
		<Song [1..*]
			@FilePath
			@FileSize
			@Flag
		>
			<Tags [1]
				@Flag
				@Author
				@Title
				@Year
				@Genre
				@Bpm
				@Key
				@Album
				@Composer
				@Label
				@TrackNumber
				@Remix
				@Stars
				@Remixer
				@Grouping
				@User1
				@User2
				@Internal
			/>
			<Infos [1]
				@SongLength
				@Bitrate
				@Cover
				@Color
				@FirstSeen
				@FirstPlay
				@LastPlay
				@PlayCount
				@Corrupted
				@Faked [DEPRECATED]
				@BpmTag [DEPRECATED]
				@KeyTag [DEPRECATED]
				@Gain
				@UserColor
			/>
			<Comment [0..1] />
			<Scan [0..1]
				@Version
				@Flag
				@Volume
				@Bpm
				@AltBpm
				@Key
			/>
			<Poi [0..*]
				@Pos
				@PoiType
				@Point
				@Name
				@Num
				@Bpm
				@Size
				@Color
				@Slot
			/>
			<CustomMix [0..1] />
		</Song>
	</VirtualDJ_Database>
	*/

	while (!xml.atEnd()) {
		auto token = xml.readNext();
		if (token == QXmlStreamReader::StartElement) {
			const auto localname = xml.name();
			if (localname == QLatin1String("VirtualDJ_Database")) {
				auto attrs = xml.attributes();
				db.version = attrs.value("Version").toString();
			}
			else if ((localname.compare(QLatin1String("Song"), Qt::CaseInsensitive) == 0) || (localname.compare(QLatin1String("track"), Qt::CaseInsensitive) == 0)) {
				Track track;
				// modern DBs use many attrs directly on song/track
				auto attrs = xml.attributes();
				track.filePath = attrs.value("FilePath").toString();
				if (track.filePath.isEmpty()) {
					track.filePath = attrs.value("filePath").toString();
				}
				track.fileSize = optI64(attrs.value("FileSize").toString());

				if (!readTrack(track)) {
					if (pErrorString) {
						*pErrorString = "Malformed <Song/track> body";
						return false;
					}
				}
				db.tracks.push_back(std::move(track));
			}
			else if (localname == QLatin1String("FavoriteFolder")) {
				// parse remnants of earlier versions of the database if they exist
				readFavoriteFolder(db);
			}
			else {
				skipElement(); // unknown root child
			}
		}
	}
	if (xml.hasError()) { if (pErrorString) *pErrorString = xml.errorString(); return false; }
	return true;
}

bool VdjModernParser::readTrack(Track& track) {
	while (!xml.atEnd()) {
		auto token = xml.readNext();
		const auto localname = xml.name().toString().toLower();
		if (token == QXmlStreamReader::EndElement &&
			  (localname == QLatin1String("song") || localname == QLatin1String("track"))) {
			return true;
		}

		if (token != QXmlStreamReader::StartElement) continue;

		if (localname == QLatin1String("tag")) {
			readTagNode(track);
		}
		else if (localname == QLatin1String("scan")) {
			readScanNode(track);
		}
		else if (localname == QLatin1String("poi")) {
			readPoiNode(track);
		}
		else if (localname == QLatin1String("comment")) {
			// text node
			QString commentString;
			while (!xml.atEnd()) {
				auto localToken = xml.readNext();
				if (localToken == QXmlStreamReader::EndElement && xml.name() == QLatin1String("comment")) {
					break;
				}
				if (localToken == QXmlStreamReader::Characters ||
				  		localToken == QXmlStreamReader::EntityReference) {
					commentString += xml.text().toString();
				}
			}
			track.comment = commentString;
		}
		else {
			skipElement();
		}
	}
	return false;
}

void VdjModernParser::readScanNode(Track& track) {
	auto attrs = xml.attributes();

	// bpm can be either float or centiBPM
	if (!track.bpm) {
		if (auto bpmDouble = optDouble(attrs.value("Bpm").toString())) {
			track.bpm = *bpmDouble;
		}
		else if (auto bpmCenti = optInt(attrs.value("Bpm").toString())) {
			track.bpm = bpmFromCenti(*bpmCenti);
		}
	}

	if (!track.key.size()) {
		track.key = attrs.value("Key").toString();
	}
	skipElement();
}

void VdjModernParser::readTagNode(Track& track) {
	auto attrs = xml.attributes();

	if (!track.key.size()) {
		track.key = attrs.value("Key").toString();
	}
	track.artist = attrs.value("Author").toString();
	track.album = attrs.value("Album").toString();
	track.composer = attrs.value("Composer").toString();
	track.genre = attrs.value("Genre").toString();
	track.key = attrs.value("Key").toString();
	track.rating = optInt(attrs.value("Stars").toString());
	track.title = attrs.value("Title").toString();
	track.year = attrs.value("Year").toString();

	// bpm can be either float or centiBPM
	if (!track.bpm) {
		if (auto bpmDouble = optDouble(attrs.value("Bpm").toString())) {
			track.bpm = *bpmDouble;
		}
		else if (auto bpmCenti = optInt(attrs.value("Bpm").toString())) {
			track.bpm = bpmFromCenti(*bpmCenti);
		}
	}

	skipElement();
}

void VdjModernParser::readInfoNode(Track& track) {
	auto attrs = xml.attributes();

	// length can be set in ms
	if (!track.lengthSeconds) {
		if (auto ms = optI64(attrs.value("SongLength").toString())) {
			track.lengthSeconds = double(*ms) / 1000.0;
		}
	}

	track.playCount = optInt(attrs.value("PlayCount").toString());
	track.bitrateKbps = optInt(attrs.value("Bitrate").toString());
	track.cover = optInt(attrs.value("Cover").toString());
	track.color = optInt(attrs.value("Color").toString());
	track.firstSeen = parseVdjTime(attrs.value("FirstSeen").toString());
	track.firstPlay = parseVdjTime(attrs.value("FirstPlay").toString());
	track.lastPlay = parseVdjTime(attrs.value("LastPlay").toString());
}

static vdj::Poi::PoiType mapPoiType(QString poiTypeString) {
	poiTypeString = poiTypeString.toLower();
	if (poiTypeString == "hotcue" || poiTypeString == "cue") return vdj::Poi::PoiType::Hotcue;
	if (poiTypeString == "loop") return vdj::Poi::PoiType::Loop;
	if (poiTypeString == "beatgrid" || poiTypeString == "grid") return vdj::Poi::PoiType::Beatgrid;
	if (poiTypeString == "automix-in" || poiTypeString == "mix-in") return vdj::Poi::PoiType::AutomixIn;
	if (poiTypeString == "automix-out" || poiTypeString == "mix-out") return vdj::Poi::PoiType::AutomixOut;
	if (poiTypeString == "mix") return vdj::Poi::PoiType::Mix;
	if (poiTypeString == "marker") return vdj::Poi::PoiType::Marker;
	return vdj::Poi::PoiType::Unknown;
}

void VdjModernParser::readPoiNode(Track& track) {
	Poi poi;
	auto attrs = xml.attributes();
	poi.type = mapPoiType(attrs.value("Type").toString());

	const auto unitStr = attrs.value("Unit").toString().toLower();
	if (unitStr == "beat" || unitStr == "beats") {
		poi.posUnit = PoiUnit::Beats;
	}
	else if (unitStr == "ms" || unitStr == "millis") {
		poi.posUnit = PoiUnit::Millis;
	}
	else {
		poi.posUnit = PoiUnit::Millis; // modern DBs usually use ms as default
	}

	// position: either Pos or Position
	QString position = attrs.value("Pos").toString();
	if (position.isEmpty()) {
		position = attrs.value("Position").toString();
	}
	if (auto positionDouble = optDouble(position)) {
		poi.posValue = *positionDouble;
	}

	// Length
	if (auto lengthDouble = optDouble(attrs.value("Length").toString())) {
		poi.lengthValue = *lengthDouble; 
		poi.lengthUnit = poi.posUnit; 
	}

	poi.cueslot = optInt(attrs.value("Slot").toString());
	poi.number = optInt(attrs.value("Num").toString());
	poi.name = attrs.value("Name").toString();
	if (auto colorInt = optInt(attrs.value("Color").toString())) {
		poi.color = *colorInt;
	}

	track.pois.push_back(std::move(poi));
	skipElement();
}

void VdjModernParser::readFavoriteFolder(Database& db) {
	FavoriteFolder folder;
	auto attrs = xml.attributes();
	folder.name = attrs.value("Name").toString();
	folder.folderPath = attrs.value("FolderPath").toString();
	folder.order = optInt(attrs.value("Order").toString());
	skipElement();
	db.favorites.push_back(std::move(folder));
}

void VdjModernParser::skipElement() {
	int depth = 1;
	while (!xml.atEnd() && depth > 0) {
		auto t = xml.readNext();
		if (t == QXmlStreamReader::StartElement) ++depth;
		else if (t == QXmlStreamReader::EndElement) --depth;
	}
}