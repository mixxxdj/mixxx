
#include <lo/lo.h>

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

int osc_message_handler(const char* path,
        const char* types,
        lo_arg** argv,
        int argc,
        lo_message msg,
        void* user_data) {
    (void)path;
    (void)types;
    (void)argv;
    (void)argc;
    (void)msg;

    bool* messageReceived = static_cast<bool*>(user_data);
    *messageReceived = true;
    std::cout << "[OSC TEST] Message received" << std::endl;
    return 0;
}

int main() {
    const int oscPortIn = 12345;
    std::atomic<bool> messageReceived{false};

    // Start server thread
    lo_server_thread st = lo_server_thread_new_with_proto(std::to_string(oscPortIn).c_str(),
            LO_UDP,
            nullptr);
    if (!st) {
        std::cerr << "[OSC TEST] Failed to start server on port " << oscPortIn << std::endl;
        return 1;
    }

    lo_server s = lo_server_thread_get_server(st);
    (void)s;
    lo_server_thread_add_method(st, NULL, NULL, osc_message_handler, &messageReceived);
    (void)lo_server_thread_start(st);

    std::cout << "[OSC TEST] Receiver started on port " << oscPortIn << std::endl;

    // Send a test message to self
    lo_address t = lo_address_new("localhost", std::to_string(oscPortIn).c_str());
    lo_send(t, "/test", NULL);

    // Wait up to 5 seconds for the message
    for (int i = 0; i < 50; ++i) {
        if (messageReceived.load()) {
            std::cout << "[OSC TEST] Test succeeded: message received" << std::endl;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (!messageReceived.load()) {
        std::cout << "[OSC TEST] Test failed: message not received in 5 seconds" << std::endl;
    }

    lo_server_thread_stop(st);
    lo_server_thread_free(st);
    lo_address_free(t);

    return messageReceived.load() ? 0 : 1;
}
