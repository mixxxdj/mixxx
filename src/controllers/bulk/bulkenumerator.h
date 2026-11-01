#pragma once

#include <QFile>
#include <QList>
#include <QPair>
#include <QSet>
#include <QStringList>
#include <cstdint>

#include "controllers/controllerenumerator.h"

struct libusb_context;

/// Locate supported USB bulk controllers
///
/// The list of supported devices (vendor/product id) and their endpoints is
/// sourced from the controller mapping XML files instead of a hardcoded list.
class BulkEnumerator : public ControllerEnumerator {
    Q_OBJECT
  public:
    explicit BulkEnumerator(const QStringList& mappingSearchPaths);
    ~BulkEnumerator() override;

    QList<Controller*> queryDevices() override;

  private:
    QList<Controller*> m_devices;
    QSet<QPair<std::uint16_t, std::uint16_t>> m_supportedDevices;
#ifndef __ANDROID__
    libusb_context* m_pContext;
#endif

    bool is_interesting(std::uint16_t idVendor, std::uint16_t idProduct);
    void readProductInfo(QFile& file);
    void loadSupportedDevices(const QStringList& searchPaths);
};
