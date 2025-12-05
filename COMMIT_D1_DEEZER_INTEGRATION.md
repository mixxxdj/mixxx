# Commit D1: Deezer Integration for Mixxx

## Status: 📋 Planning Phase (Research Required)

## ⚠️ WICHTIGER RECHTLICHER HINWEIS / LEGAL NOTICE

**Dieses Feature befindet sich in einer rechtlichen Grauzone.**

- Das Herunterladen von DRM-geschützter Musik kann gegen Urheberrechtsgesetze verstoßen
- Verstößt möglicherweise gegen Deezers Nutzungsbedingungen
- Die Implementierung sollte nur für **persönlichen, nicht-kommerziellen Gebrauch** erfolgen
- Auto-Delete nach Auswurf minimiert das Risiko, ersetzt aber keine legale Lizenzierung

**Empfehlung:** Vor Implementierung rechtliche Situation im jeweiligen Land prüfen.

---

## Konzept-Übersicht

### Ziel
Integration von Deezer-Streaming in Mixxx mit lokalem Caching:
1. Deezer-Bibliothek in Mixxx durchsuchbar machen
2. Tracks bei Bedarf herunterladen (FLAC-Qualität)
3. Nach Auswurf aus Deck sicher löschen (`srm`)

### Architektur

```
┌─────────────────────────────────────────────────────────────────┐
│                         MIXXX                                    │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐      │
│  │   Library    │    │  Deezer      │    │   Deck       │      │
│  │   Browser    │───▶│  Plugin      │───▶│   Player     │      │
│  │              │    │              │    │              │      │
│  │ [Deezer Tab] │    │ - Search API │    │ - Load Track │      │
│  │              │    │ - Download   │    │ - Play       │      │
│  │              │    │ - Cache Mgmt │    │ - Eject→Del  │      │
│  └──────────────┘    └──────────────┘    └──────────────┘      │
│                             │                                    │
│                             ▼                                    │
│                    ┌──────────────┐                             │
│                    │  Local Cache │                             │
│                    │  /tmp/mixxx/ │                             │
│                    │  deezer/     │                             │
│                    └──────────────┘                             │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                    DEEMIX BACKEND                                │
│  (Separater Prozess / Python)                                   │
│                                                                  │
│  - Deezer API Kommunikation                                     │
│  - ARL Token Management                                         │
│  - FLAC Download                                                │
│  - Metadaten                                                    │
└─────────────────────────────────────────────────────────────────┘
```

---

## Referenz-Repositories

### 1. deezer-linux
**Git:** `git@github.com:co-de-lab/deezer-linux.git`

Electron-basierter Deezer Desktop Client für Linux.
- Zeigt wie Deezer API integriert wird
- Desktop-Integration Beispiele

### 2. deemix
**Git:** `git@github.com:co-de-lab/deemix.git`

Python-basiertes Download-Tool für Deezer.
- Download-Logik
- API-Handling
- ARL Token Authentication
- FLAC/MP3 Qualitätsauswahl

### Klonen der Repos
```bash
git clone git@github.com:co-de-lab/deezer-linux.git
git clone git@github.com:co-de-lab/deemix.git
```

---

## Recherche-Aufgaben für Opus

### Phase 1: Deemix verstehen
```bash
# Repos klonen (falls noch nicht geschehen)
git clone git@github.com:co-de-lab/deemix.git
git clone git@github.com:co-de-lab/deezer-linux.git

# Struktur analysieren
ls -la deemix/
cat deemix/README.md

# Python-Module untersuchen
find deemix -name "*.py" | head -20

# API-Handling finden
grep -r "api" deemix --include="*.py" | head -30

# Download-Logik
grep -r "download" deemix --include="*.py" | head -30
```

### Phase 2: Mixxx Plugin-System verstehen
```bash
# Bestehende externe Quellen in Mixxx
grep -r "soundcloud\|beatport\|tidal" src/ --include="*.cpp" --include="*.h"

# Library-Integration
ls src/library/
grep -r "ExternalTrackCollection" src/
```

### Phase 3: Integration planen
- Wie kommuniziert Mixxx mit externen Diensten?
- Gibt es bereits ein Plugin-System?
- Wie werden externe Tracks in die Library eingebunden?

---

## Geplante Commits

### D1: Deezer Library Browser (dieser Commit - Planung)
- Recherche und Architektur

### D2: Deemix Backend Integration
- Python-Backend als separater Prozess
- IPC-Kommunikation (Socket/DBus)
- ARL Token Management UI

### D3: Deezer Search & Browse
- Suchfunktion in Library
- Playlists anzeigen
- Album/Artist Browser

### D4: On-Demand Download
- Track bei Load herunterladen
- Progress-Anzeige
- Cache-Management

### D5: Auto-Delete on Eject
- Sichere Löschung mit `srm`
- Konfigurierbar (behalten/löschen)
- Cache-Größen-Limit

### D6: UI Integration
- Deezer-Spalte in Library
- Login-Dialog
- Einstellungen

---

## Technische Details

### Download-Workflow

```
1. User wählt Track in Deezer-Browser
2. User lädt Track in Deck
   │
   ▼
3. Plugin prüft: Track im Cache?
   │
   ├─ JA ──▶ Lade aus Cache
   │
   └─ NEIN ─▶ 4. Starte Download via Deemix
              │
              ▼
           5. Download FLAC nach /tmp/mixxx/deezer/{track_id}.flac
              │
              ▼
           6. Lade in Deck
              │
              ▼
           7. Track wird abgespielt
              │
              ▼
           8. User wirft Track aus (Eject)
              │
              ▼
           9. Secure Delete: srm /tmp/mixxx/deezer/{track_id}.flac
```

### Secure Delete (`srm`)

```cpp
void DeezerPlugin::secureDeleteTrack(const QString& filePath) {
    // Verwende srm (secure-delete) für sichere Löschung
    QProcess::execute("srm", QStringList() << "-sz" << filePath);
    // -s: simple mode (1 pass)
    // -z: zero-fill after overwrite
}
```

### Cache-Verzeichnis

```cpp
// Temporäres Verzeichnis für Deezer-Downloads
QString DeezerPlugin::getCacheDir() {
    return QDir::tempPath() + "/mixxx/deezer/";
}
```

### ARL Token

Deezer verwendet ARL (Authentication Request License) Token:
- User muss einmalig ARL aus Browser extrahieren
- Token in Mixxx-Einstellungen speichern (verschlüsselt)
- Deemix verwendet Token für API-Zugriff

---

## Abhängigkeiten

### Externe Tools (müssen installiert sein)
- Python 3.x
- deemix Python-Modul
- `srm` (secure-delete Paket)

### Installation Check
```bash
# Prüfen ob srm installiert ist
which srm || echo "Install: sudo apt install secure-delete"

# Prüfen ob Python verfügbar
python3 --version

# Deemix installieren (wenn nötig)
pip install deemix
```

---

## Konfiguration (geplant)

```ini
[Deezer]
Enabled=true
ARLToken=<encrypted>
CacheDir=/tmp/mixxx/deezer
MaxCacheSize=5GB
AutoDeleteOnEject=true
PreferredQuality=FLAC
```

---

## Offene Fragen

1. **Plugin-Architektur**: Hat Mixxx ein Plugin-System oder muss Code direkt integriert werden?

2. **IPC-Methode**: Wie kommuniziert C++ (Mixxx) am besten mit Python (Deemix)?
   - QProcess + stdout/stdin
   - Unix Socket
   - DBus
   - Shared Memory

3. **UI-Integration**: Neuer Tab in Library oder Seitenleiste?

4. **Streaming vs. Download**:
   - Nur Download (aktueller Plan)
   - Oder echter Streaming-Support?

5. **Offline-Modus**: Wie mit fehlender Internetverbindung umgehen?

---

## Risiken

| Risiko | Wahrscheinlichkeit | Auswirkung | Mitigation |
|--------|-------------------|------------|------------|
| Rechtliche Probleme | Hoch | Hoch | Auto-Delete, nur privat nutzen |
| Deezer API-Änderungen | Mittel | Hoch | Deemix-Updates verfolgen |
| ARL Token Expiry | Mittel | Mittel | Token-Refresh UI |
| Performance (Download) | Niedrig | Mittel | Pre-fetch nächster Track |

---

## Nächste Schritte

1. ⏳ **Opus**: Deemix-Codebase analysieren
2. ⏳ **Opus**: Mixxx Plugin/Extension-System recherchieren
3. ⏳ Architektur-Entscheidung: Plugin vs. Integration
4. ⏳ Prototyp: Minimaler Download-Test
5. ⏳ UI-Mockup für Deezer-Browser
