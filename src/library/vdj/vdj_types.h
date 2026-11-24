#pragma once
#include <QtCore>
#include <optional>

namespace vdj {

	enum class PoiUnit { Samples441, Millis, Beats };

	struct VdjTime {
		enum class Kind { None, LegacySignedInt, YmdhmString };
		Kind kind = Kind::None;
		qint64 legacyValue = 0;
		QString ymdhm;
	};

	struct Poi {
		enum class PoiType { Hotcue, Loop, Beatgrid, AutomixIn, AutomixOut, Mix, Marker, Unknown };
		PoiType type = PoiType::Unknown;
		double posValue = 0.0;         // raw position in chosen unit
		PoiUnit posUnit = PoiUnit::Millis;
		double lengthValue = 0.0;      // for loops (0 if not applicable)
		PoiUnit lengthUnit = PoiUnit::Millis;
		std::optional<int> cueslot;     // cue slot
		std::optional<int> number;     // Poi number
		QString name;
		std::optional<int> color;      // if present (index/packed)
	};

	struct Track {
		QString filePath;
		std::optional<qint64> fileSize;

		// tags / display
		QString title, artist, album, genre, year, comment, composer, tag;
		std::optional<int> color, cover;

		// analysis / scan
		std::optional<double> bpm;     // normalized to float BPM
		QString key;                   // final preferred key (tag>detected)
		std::optional<double> lengthSeconds;
		std::optional<int> playCount, rating, bitrateKbps, gain;

		VdjTime firstSeen, firstPlay, lastPlay;

		// poi list (hotcues, loops, beatgrid anchors, automix, etc.)
		QVector<Poi> pois;
	};

	struct FavoriteFolder {
		QString name, folderPath;
		std::optional<int> order;
	};

	struct Database {
		QString version;
		QVector<Track> tracks;
		QVector<FavoriteFolder> favorites;
	};

	// Helpers
	inline double samplesToSeconds(qint64 samples) { return double(samples) / 44100.0; }
	inline double bpmFromCenti(int bpmCenti) { return double(bpmCenti) / 100.0; }

} // namespace vdj
