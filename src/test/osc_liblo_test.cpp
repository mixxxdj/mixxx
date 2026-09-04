// tests/osc_liblo_test.cpp
#include <lo/lo.h>

#include <atomic>
#include <cassert>
#include <chrono>
#include <thread>

int main() {
    std::atomic<bool> message_received(false);

    // Create an OSC server on a random port
    lo_server_thread st = lo_server_thread_new(nullptr, nullptr);
    assert(st && "Failed to create liblo server thread");

    // Define a handler for OSC messages at path "/test"
    lo_server_thread_add_method(
            st,
            "/test",
            "s",
            [](const char* path,
                    const char* types,
                    lo_arg** argv,
                    int argc,
                    void* data,
                    void* user_data) -> int {
                const char* msg = &argv[0]->s;
                if (std::string(msg) == "hello") {
                    *static_cast<std::atomic<bool>*>(user_data) = true;
                }
                return 0;
            },
            &message_received);

    lo_server_thread_start(st);

    // Create client pointing to our server
    int port = lo_server_thread_get_port(st);
    lo_address addr = lo_address_new(nullptr, std::to_string(port).c_str());
    assert(addr && "Failed to create lo_address");

    // Send an OSC message
    int result = lo_send(addr, "/test", "s", "hello");
    assert(result != -1 && "Failed to send OSC message");

    // Wait briefly for the handler to process the message
    for (int i = 0; i < 50 && !message_received.load(); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    lo_server_thread_stop(st);
    lo_free_address(addr);
    lo_server_thread_free(st);

    return message_received.load() ? 0 : 1;
}
