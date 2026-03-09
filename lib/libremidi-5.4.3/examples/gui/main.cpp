#include <libremidi/configurations.hpp>
#include <libremidi/libremidi.hpp>

#include <QApplication>
#include <QHBoxLayout>
#include <QListWidget>
#include <QMessageBox>
#include <QSplitter>
#include <QWidget>

#if defined(_WIN32) && __has_include(<winrt/base.h>)
  #include <winrt/base.h>
#endif

Q_DECLARE_METATYPE(libremidi::port_information);

struct port_equal
{
  bool operator()(const libremidi::port_information& lhs, const libremidi::port_information& rhs) const noexcept
  {
    return lhs.api == rhs.api && lhs.port_name == rhs.port_name;
  }
};
struct port_name_sort
{
  bool operator()(const libremidi::port_information& lhs, const libremidi::port_information& rhs) const noexcept
  {
    return std::tie(lhs.api, lhs.port_name) < std::tie(rhs.api, rhs.port_name);
  }
};

int main(int argc, char** argv)
{
#if defined(_WIN32) && __has_include(<winrt/base.h>)
  // Necessary for using WinUWP and WinMIDI, must be done as early as possible in your main()
  // In addition, we are using Qt in this example which requires COM to be in single-thread mode.
  winrt::init_apartment(winrt::apartment_type::single_threaded);
#endif
  using namespace libremidi;
  auto in_api = libremidi::midi1::in_default_configuration();
  auto out_api = libremidi::midi1::out_default_configuration();
  auto observer_api = libremidi::midi1::observer_default_configuration();

  // Create the GUI
  QApplication app{argc, argv};
  QSplitter main;
  QListWidget inputs, outputs, messages;
  main.addWidget(&inputs);
  main.addWidget(&messages);
  main.addWidget(&outputs);

  main.show();

  std::map<libremidi::input_port, QListWidgetItem*, port_name_sort> input_items;
  std::map<libremidi::output_port, QListWidgetItem*, port_name_sort> output_items;

  // Define the observer callbacks which will fill the list widgets with the input & output devices
  observer_configuration conf{.input_added = [&](const input_port& p) {
    auto item = new QListWidgetItem{QString::fromStdString(p.display_name)};
    item->setData(Qt::UserRole, QVariant::fromValue(p));
    input_items[p] = item;

    inputs.addItem(item);
  }, .input_removed = [&](const input_port& p) {
    if (auto it = input_items.find(p); it != input_items.end())
    {
      for(int i = 0; i < inputs.count(); i++) {
        if(auto item = inputs.item(i); item == it->second) {
          delete inputs.takeItem(i);
          break;
        }
      }
      input_items.erase(it);
    }
  }, .output_added = [&](const output_port& p) {
    auto item = new QListWidgetItem{QString::fromStdString(p.display_name)};
    item->setData(Qt::UserRole, QVariant::fromValue(p));
    output_items[p] = item;

    outputs.addItem(item);
  }, .output_removed = [&](const output_port& p) {
    if (auto it = output_items.find(p); it != output_items.end())
    {
      for(int i = 0; i < outputs.count(); i++) {
        if(auto item = outputs.item(i); item == it->second) {
          delete outputs.takeItem(i);
          break;
        }
      }
      output_items.erase(it);
    }
  }};

  // Create the libremidi structures
  observer obs{conf, observer_api};
  libremidi::midi_out out{{}, out_api};

  auto input_callback = [&](const libremidi::message& m) {
    // We move things to the main thread as the API may run its callbacks in some random thread
    QMetaObject::invokeMethod(qApp, [&, m] {
      QString msg;
      msg += QString::number(m.timestamp);
      msg += ": ";
      msg += QString::number(m.bytes.size());
      msg += ": ";

      for (auto byte : m.bytes)
      {
        msg += QString::number(byte, 16);
        msg += ' ';
      }
      messages.addItem(msg);
      if (messages.count() > 10)
        delete messages.takeItem(0);

      // Forward to the output port
      if (out.is_port_open())
      {
        out.send_message(m);
      }
    });
  };

  libremidi::midi_in in{{.on_message = input_callback}, in_api};

  // Connect gui changes to port changes
  QObject::connect(
      &inputs, &QListWidget::currentItemChanged, [&](QListWidgetItem* selected, QListWidgetItem*) {
    in.close_port();
    for (auto& [port, item] : input_items)
    {
      if (item == selected)
      {
        in.open_port(port);
        if (!in.is_port_open())
        {
          QMessageBox::warning(
              &main, QString("Error !"),
              QString("Could not connect to input:\n%1\n%2")
                  .arg(port.display_name.c_str())
                  .arg(port.port_name.c_str()));
        }
        return;
      }
    }
  });

  QObject::connect(
      &outputs, &QListWidget::currentItemChanged,
      [&](QListWidgetItem* selected, QListWidgetItem*) {
    out.close_port();
    for (auto& [port, item] : output_items)
    {
      if (item == selected)
      {
        out.open_port(port);
        if (!out.is_port_open())
        {
          QMessageBox::warning(
              &main, QString("Error !"),
              QString("Could not connect to output:\n%1\n%2")
                  .arg(port.display_name.c_str())
                  .arg(port.port_name.c_str()));
        }
        return;
      }
    }
  });
  return app.exec();
}
