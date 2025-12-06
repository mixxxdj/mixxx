# 432Hz Tuning Support / 432Hz Stimmung Unterstützung / Soporte de afinación 432Hz

## English

### Overview
Mixxx now includes comprehensive support for tracks tuned to 432Hz concert pitch (as opposed to the standard 440Hz). This feature set includes automatic detection, visual indication, and automatic pitch adjustment to play all tracks in 432Hz tuning.

### Features

#### 1. Automatic 432Hz Detection
- **Location**: Preferences → Key Detection → "Detect 432Hz tuning (experimental)"
- The Queen Mary Key-Detector analyzes tracks at both 440Hz and 432Hz tuning frequencies
- The analysis with more consistent key detection (fewer key changes) is considered correct
- The detected tuning is stored in the track metadata

#### 2. Visual Indicator in Library
- Tracks detected as 432Hz display a golden sparkle symbol (✧) in the Key column
- The symbol appears at the right side of the key value
- Symbol color: Golden (#DAA520) / Bright gold (#FFD700) when selected

#### 3. 432Hz Pitch Lock
- **Location**: Preferences → Decks → "Play all tracks in 432Hz tuning"
- When enabled, all tracks are automatically adjusted to play in 432Hz tuning:
  - **440Hz tracks**: Automatically pitched down by ~31.77 cents (-0.31767 semitones)
  - **432Hz tracks**: Played without pitch adjustment
- This allows DJs to maintain consistent 432Hz tuning across their entire set

### Technical Details
- Pitch adjustment formula: log₂(432/440) × 12 ≈ -0.31767 semitones
- Detection uses dual chromagram analysis comparing consistency at both tuning frequencies
- 432Hz detection requires 20% better consistency to avoid false positives
- Settings stored in Mixxx configuration under `[Key]` section

### Usage Workflow
1. Enable 432Hz detection in Preferences → Key Detection
2. Analyze your music library (right-click tracks → Analyze)
3. Tracks tuned to 432Hz will show the ✧ symbol
4. Optionally enable "Play all tracks in 432Hz tuning" in Deck preferences
5. Mix freely - pitch adjustment happens automatically

---

## Deutsch

### Übersicht
Mixxx unterstützt jetzt umfassend Titel, die auf 432Hz Kammerton A gestimmt sind (im Gegensatz zum Standard 440Hz). Diese Funktionen umfassen automatische Erkennung, visuelle Anzeige und automatische Tonhöhenanpassung, um alle Titel in 432Hz-Stimmung abzuspielen.

### Funktionen

#### 1. Automatische 432Hz-Erkennung
- **Ort**: Einstellungen → Tonart-Erkennung → "Detect 432Hz tuning (experimental)"
- Der Queen Mary Key-Detector analysiert Titel sowohl bei 440Hz als auch bei 432Hz Stimmfrequenz
- Die Analyse mit konsistenterer Tonarterkennung (weniger Tonartwechsel) gilt als korrekt
- Die erkannte Stimmung wird in den Track-Metadaten gespeichert

#### 2. Visuelle Anzeige in der Bibliothek
- Als 432Hz erkannte Titel zeigen ein goldenes Funkelsymbol (✧) in der Tonart-Spalte
- Das Symbol erscheint rechts neben dem Tonartwert
- Symbolfarbe: Golden (#DAA520) / Helles Gold (#FFD700) wenn ausgewählt

#### 3. 432Hz Tonhöhensperre
- **Ort**: Einstellungen → Decks → "Play all tracks in 432Hz tuning"
- Wenn aktiviert, werden alle Titel automatisch angepasst, um in 432Hz-Stimmung abgespielt zu werden:
  - **440Hz-Titel**: Automatisch um ~31,77 Cent herabgepitcht (-0,31767 Halbtöne)
  - **432Hz-Titel**: Werden ohne Tonhöhenanpassung abgespielt
- Dies ermöglicht DJs, eine konsistente 432Hz-Stimmung über ihr gesamtes Set beizubehalten

### Technische Details
- Tonhöhenanpassungsformel: log₂(432/440) × 12 ≈ -0,31767 Halbtöne
- Erkennung verwendet duale Chromagramm-Analyse zum Vergleich der Konsistenz bei beiden Stimmfrequenzen
- 432Hz-Erkennung benötigt 20% bessere Konsistenz, um Fehlerkennungen zu vermeiden
- Einstellungen werden in der Mixxx-Konfiguration unter dem Abschnitt `[Key]` gespeichert

### Nutzungsablauf
1. Aktiviere 432Hz-Erkennung in Einstellungen → Tonart-Erkennung
2. Analysiere deine Musikbibliothek (Rechtsklick auf Titel → Analysieren)
3. Auf 432Hz gestimmte Titel zeigen das ✧-Symbol
4. Optional aktiviere "Play all tracks in 432Hz tuning" in den Deck-Einstellungen
5. Mixe frei - die Tonhöhenanpassung erfolgt automatisch

---

## Español

### Descripción General
Mixxx ahora incluye soporte completo para pistas afinadas a 432Hz de tono de concierto (en oposición al estándar de 440Hz). Este conjunto de funciones incluye detección automática, indicación visual y ajuste automático de tono para reproducir todas las pistas en afinación 432Hz.

### Características

#### 1. Detección Automática de 432Hz
- **Ubicación**: Preferencias → Detección de Tonalidad → "Detect 432Hz tuning (experimental)"
- El detector de tonalidad Queen Mary analiza pistas a frecuencias de afinación de 440Hz y 432Hz
- El análisis con detección de tonalidad más consistente (menos cambios de tonalidad) se considera correcto
- La afinación detectada se almacena en los metadatos de la pista

#### 2. Indicador Visual en la Biblioteca
- Las pistas detectadas como 432Hz muestran un símbolo de destello dorado (✧) en la columna de Tonalidad
- El símbolo aparece en el lado derecho del valor de tonalidad
- Color del símbolo: Dorado (#DAA520) / Oro brillante (#FFD700) cuando está seleccionado

#### 3. Bloqueo de Tono 432Hz
- **Ubicación**: Preferencias → Decks → "Play all tracks in 432Hz tuning"
- Cuando está habilitado, todas las pistas se ajustan automáticamente para reproducirse en afinación 432Hz:
  - **Pistas 440Hz**: Automáticamente reducidas en ~31.77 centésimas (-0.31767 semitonos)
  - **Pistas 432Hz**: Reproducidas sin ajuste de tono
- Esto permite a los DJs mantener una afinación 432Hz consistente en todo su set

### Detalles Técnicos
- Fórmula de ajuste de tono: log₂(432/440) × 12 ≈ -0.31767 semitonos
- La detección utiliza análisis dual de cromograma comparando consistencia en ambas frecuencias de afinación
- La detección de 432Hz requiere 20% mejor consistencia para evitar falsos positivos
- Configuraciones almacenadas en la configuración de Mixxx bajo la sección `[Key]`

### Flujo de Trabajo de Uso
1. Habilita la detección de 432Hz en Preferencias → Detección de Tonalidad
2. Analiza tu biblioteca musical (clic derecho en pistas → Analizar)
3. Las pistas afinadas a 432Hz mostrarán el símbolo ✧
4. Opcionalmente habilita "Play all tracks in 432Hz tuning" en las preferencias de Deck
5. Mezcla libremente - el ajuste de tono ocurre automáticamente

---

## Implementation Details / Implementierungsdetails / Detalles de Implementación

### Modified Files / Geänderte Dateien / Archivos Modificados

**Core Detection:**
- `src/proto/keys.proto` - Added `is_432hz` field
- `src/track/keys.h/cpp` - Added `is432Hz()` getter
- `src/track/track.h/cpp` - Added `is432Hz()` method
- `src/analyzer/plugins/analyzerqueenmarykey.h/cpp` - Dual 440Hz/432Hz analysis
- `src/analyzer/analyzerkey.cpp` - 432Hz detection integration
- `src/track/keyfactory.h/cpp` - Added `is432Hz` parameter

**UI and Display:**
- `src/library/trackmodel.h` - Added `k432HzRole`
- `src/library/basetracktablemodel.cpp` - Handle 432Hz data role
- `src/library/tabledelegates/keydelegate.cpp` - Display ✧ symbol

**Settings:**
- `src/preferences/keydetectionsettings.h` - 432Hz detection preference
- `src/preferences/dialog/dlgprefkeydlg.ui` - Detection checkbox
- `src/preferences/dialog/dlgprefkey.h/cpp` - Settings handling
- `src/preferences/dialog/dlgprefdeckdlg.ui` - Pitch lock checkbox
- `src/preferences/dialog/dlgprefdeck.h/cpp` - Pitch lock settings

**Playback Engine:**
- `src/mixer/basetrackplayer.h/cpp` - Added `file_is_432hz` control
- `src/engine/controls/keycontrol.h/cpp` - Pitch adjustment logic

### License / Lizenz / Licencia
This feature is part of Mixxx and follows the same license as the main project (GPL-2.0+).
