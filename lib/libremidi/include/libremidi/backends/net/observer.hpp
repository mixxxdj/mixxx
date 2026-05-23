#pragma once
#include <libremidi/backends/dummy.hpp>

NAMESPACE_LIBREMIDI::net
{
using net_observer_configuration = libremidi::dummy_configuration;
using observer = libremidi::observer_dummy;
}

NAMESPACE_LIBREMIDI::net_ump
{
using net_observer_configuration = libremidi::dummy_configuration;
using observer = libremidi::observer_dummy;
}
