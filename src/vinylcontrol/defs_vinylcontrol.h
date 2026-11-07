#pragma once

#include <QString>

const QString kVinylPrefKey = QStringLiteral("[VinylControl]");

const QString kVCGroup = QString("[Channel%1]");
constexpr int kVinylStatusDisabled = 0;
constexpr int kVinylStatusOk = 1;
constexpr int kVinylStatusWarning = 2;
constexpr int kVinylStatusError = 3;

const QString kVinylAlgoriddimA = QStringLiteral("Algoriddim djay PRO AI 12\", Side A");
const QString kVinylAlgoriddimB =
        QStringLiteral("Algoriddim djay PRO AI 12\", Side B (no stem support)");
const QString kVinylMixVibes7Inch = QStringLiteral("MixVibes 7\"");
const QString kVinylMixVibesDvs = QStringLiteral("MixVibes DVS V2 Vinyl");
const QString kVinylPioneerA = QStringLiteral("Pioneer RekordBox DVS Control Vinyl, Side A");
const QString kVinylPioneerB = QStringLiteral("Pioneer RekordBox DVS Control Vinyl, Side B");
const QString kVinylSeratoCd = QStringLiteral("Serato CD");
const QString kVinylSeratoCv02SideA = QStringLiteral("Serato CV02 Vinyl, Side A");
const QString kVinylSeratoCv02SideB = QStringLiteral("Serato CV02 Vinyl, Side B");
const QString kVinylTraktorScratchMk1SideA = QStringLiteral("Traktor Scratch MK1 Vinyl, Side A");
const QString kVinylTraktorScratchMk1SideB = QStringLiteral("Traktor Scratch MK1 Vinyl, Side B");
const QString kVinylTraktorScratchMk2Cd = QStringLiteral("Traktor Scratch MK2 CD (beta)");
const QString kVinylTraktorScratchMk2SideA =
        QStringLiteral("Traktor Scratch MK2 Vinyl, Side A (beta)");
const QString kVinylTraktorScratchMk2SideB =
        QStringLiteral("Traktor Scratch MK2 Vinyl, Side B (beta)");

constexpr const char* kVinylAlgoriddimAXwaxName = "algoriddim_a";
constexpr const char* kVinylAlgoriddimBXwaxName = "algoriddim_b";
constexpr const char* kVinylMixVibes7InchXwaxName = "mixvibes_7inch";
constexpr const char* kVinylMixVibesDvsXwaxName = "mixvibes_v2";
constexpr const char* kVinylPioneerAXwaxName = "pioneer_a";
constexpr const char* kVinylPioneerBXwaxName = "pioneer_b";
constexpr const char* kVinylSeratoCdXwaxName = "serato_cd";
constexpr const char* kVinylSeratoCv02SideAXwaxName = "serato_2a";
constexpr const char* kVinylSeratoCv02SideBXwaxName = "serato_2b";
constexpr const char* kVinylTraktorScratchMk1SideAXwaxName = "traktor_a";
constexpr const char* kVinylTraktorScratchMk1SideBXwaxName = "traktor_b";
constexpr const char* kVinylTraktorScratchMk2CdXwaxName = "traktor_mk2_cd";
constexpr const char* kVinylTraktorScratchMk2SideAXwaxName = "traktor_mk2_a";
constexpr const char* kVinylTraktorScratchMk2SideBXwaxName = "traktor_mk2_b";

constexpr const char* kVinylDefaultXwaxName = kVinylSeratoCv02SideAXwaxName;

// Default recommended lead-in times for various vinyl types.
constexpr int kVinylAlgoriddimALeadIn = 0;
constexpr int kVinylAlgoriddimBLeadIn = 0;
constexpr int kVinylMixVibes7InchLeadIn = 0;
constexpr int kVinylMixVibesDvsLeadIn = 0;
constexpr int kVinylPioneerALeadIn = 10;
constexpr int kVinylPioneerBLeadIn = 10;
constexpr int kVinylSeratoCdLeadIn = 0;
constexpr int kVinylSeratoCv02SideALeadIn = 28;
constexpr int kVinylSeratoCv02SideBLeadIn = 28;
constexpr int kVinylTraktorScratchMk1SideALeadIn = 10;
constexpr int kVinylTraktorScratchMk1SideBLeadIn = 10;
constexpr int kVinylTraktorScratchMk2CdLeadIn = 10;
constexpr int kVinylTraktorScratchMk2SideALeadIn = 10;
constexpr int kVinylTraktorScratchMk2SideBLeadIn = 10;

const QString kVinylSpeed33 = QStringLiteral("33.3 RPM");
const QString kVinylSpeed45 = QStringLiteral("45 RPM");

const QString kVinylPitchFilterLegacy = QStringLiteral("Alpha-Beta filter (fast)");
const QString kVinylPitchFilterKalman = QStringLiteral("Kalman filter (accurate)");

constexpr double kVinylSpeed33Num = (100.0 / 3.0);
constexpr double kVinylSpeed45Num = 45.0;

constexpr int kVcModeAbsolute = 0;
constexpr int kVcModeRelative = 1;
constexpr int kVcModeConstant = 2;

constexpr int kRelativeCueOff = 0;
constexpr int kRelativeCueOneCue = 1;
constexpr int kRelativeCueHotCue = 2;

constexpr int kVcDefaultLeadInTime = 0;

// Period for updating the vinyl quality scopes, ~15 Hz
constexpr int kVinylScopeUpdatePeriodMs = 66;
constexpr int kVinylScopeSize = 100;

constexpr int kMaximumVinylControlInputs = 4;
