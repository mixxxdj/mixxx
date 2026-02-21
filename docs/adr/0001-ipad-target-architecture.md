# ADR 0001: Целевая архитектура iPad-приложения Mixxx

- **Статус:** Accepted
- **Дата:** 2026-02-19
- **Решение:** Выбрать вариант A — сохранить C++ аудио-ядро Mixxx и реализовать thin native iOS shell (SwiftUI/UIKit + Objective-C++ bridge).

## Контекст

Mixxx обладает зрелым, производительным и функционально насыщенным C++ ядром (движок воспроизведения, маршрутизация аудио, библиотека треков). Цель iPad MVP — как можно быстрее получить рабочий DJ-опыт на iOS с минимальным риском регрессий в качестве аудио.

Рассматривались два варианта:

- **Вариант A:** переиспользовать C++ core, добавить thin native iOS shell (SwiftUI/UIKit) и мост Objective-C++ для интеграции с Swift.
- **Вариант B:** переписать core на Swift с использованием AudioKit/AVAudioEngine.

## Опции

### Вариант A: C++ core + thin iOS shell

**Описание:**
- Переиспользовать существующие подсистемы из `src/engine`, `src/soundio`, `src/library`.
- Добавить стабильный C ABI-слой как контракт между iOS-приложением и C++ ядром.
- Интегрировать C ABI в Swift через Objective-C++ bridge.

**Плюсы:**
- Максимальное переиспользование проверенного кода Mixxx.
- Более короткий путь к MVP.
- Ниже риск деградации DSP/latency/feature parity.
- Проще поддерживать общую бизнес-логику между desktop и iPad.

**Минусы:**
- Необходимость поддерживать межъязыковой слой и ABI-совместимость.
- Усложнение отладки на границе Swift ↔ Objective-C++ ↔ C++.

### Вариант B: переписать core на Swift/AudioKit/AVAudioEngine

**Описание:**
- Реализовать аудио-движок, микшер, deck control, waveform pipeline и библиотеку заново на Swift-стеке.

**Плюсы:**
- Нативный стек iOS без C++ bridge.
- Теоретически проще найм/онбординг iOS-разработчиков.

**Минусы:**
- Очень высокая стоимость и длительность.
- Риск функциональных расхождений и регрессий качества звука.
- Долгий период до feature parity с текущим Mixxx.

## Решение

Принят **Вариант A** как базовая архитектура для iPad MVP.

## Архитектурные границы (boundaries)

### Остаётся в C++

1. **Audio engine (`src/engine`)**
   - Deck graph, transport, sync/timestretch/pitch,
   - кроссфейдер/микшерные расчёты,
   - эффектные цепочки и обработка буферов,
   - генерация данных для waveform/level meters.

2. **Sound I/O backend (`src/soundio`)**
   - Низкоуровневая конфигурация аудио-устройств,
   - управление output/prelisten маршрутами,
   - realtime callback и буферные политики.

3. **Library core (`src/library`)**
   - Модель коллекции, метаданные, крейты/плейлисты,
   - поиск/фильтрация,
   - загрузка треков в deck.

### Переходит в Swift (iOS shell)

1. **Presentation/UI слой (SwiftUI/UIKit)**
   - Deck UI, transport controls, waveform presentation,
   - browser/library screens, touch gestures,
   - настройки iOS-приложения.

2. **App lifecycle & platform integration**
   - scene/app lifecycle,
   - интеграция с iOS-сервисами (audio session/category/interruptions, background policy в рамках платформенных ограничений),
   - permissions UX.

3. **State binding / orchestration**
   - Подписка на события C ABI,
   - преобразование engine state → observable view models,
   - command dispatch из UI в C ABI.

## C ABI-слой для варианта A

Вводится стабильный C ABI, который изолирует Swift от внутренних C++ типов и структуры проекта.

### Принципы C ABI

- **Opaque handles** вместо прямых C++ классов.
- **Версионирование API** (`mixxx_abi_version`).
- **Плоские C-структуры** для параметров/состояний (POD).
- **Явное владение памятью** и жизненным циклом объектов (`create/destroy`, `retain/release`, где необходимо).
- **Threading contract:** какие вызовы realtime-safe, какие только с main/control thread.
- **Коды ошибок и диагностика** через enum + message callback.

### Предлагаемые ABI-домены

1. **Engine ABI** (поверх `src/engine`)
   - lifecycle engine,
   - управление деками (load/play/pause/seek/rate),
   - mixer controls (gain/EQ/crossfader),
   - waveform & meters snapshot API,
   - transport/timing события.

2. **SoundIO ABI** (поверх `src/soundio`)
   - перечисление доступных конфигураций I/O,
   - выбор main output и prelisten,
   - старт/стоп аудио,
   - мониторинг xrun/latency/runtime статуса.

3. **Library ABI** (поверх `src/library`)
   - запросы коллекции/поиска,
   - чтение метаданных,
   - операции с плейлистами/крейтами (MVP-минимум),
   - резолв track-id → URI/путь для deck load.

## Критерии успеха MVP

MVP считается успешным, если на iPad реализовано и подтверждено тестами/демо:

1. **2 деки**
   - Независимая загрузка/воспроизведение треков на двух деках.

2. **Кроссфейдер и базовый микшер**
   - Рабочий кроссфейдер между deck A/B,
   - deck gain и master output control.

3. **Waveform (основной сценарий)**
   - Отображение waveform для обеих дек,
   - позиция воспроизведения и визуальное обновление в realtime.

4. **Prelisten (cue/headphones path)**
   - Отдельный prelisten маршрут (при поддерживаемом audio route/интерфейсе),
   - переключение cue для каждой деки.

5. **Latency budget**
   - End-to-end latency в целевом диапазоне для живого DJ-использования,
   - отсутствие систематических xruns/глитчей в типичных сессиях MVP.

6. **Стабильность ABI-контракта**
   - iOS shell работает только через C ABI,
   - отсутствуют прямые зависимости Swift от внутренних C++ классов.

## Последствия и следующие шаги

1. Создать RFC/спеку C ABI v1 (заголовки, модели данных, ошибки, threading contract).
2. Реализовать минимальный vertical slice:
   - `load track -> play -> crossfade -> prelisten -> waveform update`.
3. Подготовить интеграционные тесты ABI и smoke-тесты на iPad-устройствах.
4. Зафиксировать политику ABI-совместимости (minor/major изменения, deprecation window).
