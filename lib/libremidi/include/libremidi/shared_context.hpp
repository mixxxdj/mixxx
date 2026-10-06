#pragma once
#include <libremidi/api.hpp>
#include <libremidi/configurations.hpp>

NAMESPACE_LIBREMIDI
{

class shared_context
{
public:
  shared_context() = default;
  virtual ~shared_context() = default;
  shared_context(const shared_context&) = delete;
  shared_context(shared_context&&) = delete;
  shared_context& operator=(const shared_context&) = delete;
  shared_context& operator=(shared_context&&) = delete;

  virtual void start_processing() = 0;
  virtual void stop_processing() = 0;
};

struct shared_configurations
{
  std::shared_ptr<shared_context> context;
  observer_api_configuration observer;
  input_api_configuration in;
  output_api_configuration out;
};

LIBREMIDI_EXPORT
shared_configurations create_shared_context(libremidi::API api, std::string_view client_name);

}
