
#include <lo/lo.h>

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

// Handler for incoming OSC messages
// int osc_message_handler(const char* path,
//        const char* types,
//        lo_arg** argv,
//        int argc,
//        lo_message msg,
//        void* user_data) {
//    auto* flag = static_cast<std::atomic<bool>*>(user_data);
//    flag->store(true);
//    std::cout << "[OSC TEST] Message received at path: " << path << std::endl;
//    return 0;
//}

// int osc_message_handler(const char* path /*path*/,
//         const char* /*types*/,
//         lo_arg** /*argv*/,
//         int /*argc*/,
//         lo_message /*msg*/,
//         void* user_data) {
//     auto* flag = static_cast<bool*>(user_data);
//     *flag = true;
//     return 0;
// }

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

// #include <lo/lo.h>
//
// #include <atomic>
// #include <chrono>
// #include <iostream>
// #include <thread>
//
// std::atomic<bool> messageReceived(false);
//
// int osc_message_handler(const char* path, const char* types, lo_arg** argv,
// int argc, lo_message msg, void* user_data) {
//     std::cout << "[OSC] Message received on path: " << path << "\n";
//     messageReceived.store(true);
//     return 0;
// }
//
// int main() {
//     const char* port = "12345";
//
//     // Create OSC server
//     lo_server_thread st = lo_server_thread_new(port, nullptr);
//     lo_server_thread_add_method(st, NULL, NULL, osc_message_handler,
//     nullptr); lo_server_thread_start(st); std::cout << "[OSC] Receiver
//     started on port " << port << "\n";
//
//     // Send a test message
//     lo_address taddr = lo_address_new("localhost", port);
//     lo_send(taddr, "/test", "s", "hello");
//     lo_address_free(taddr);
//
//     // Wait up to 10 seconds for a message
//     for (int i = 0; i < 100; ++i) {
//         if (messageReceived.load())
//             break;
//         std::this_thread::sleep_for(std::chrono::milliseconds(100));
//     }
//
//     if (messageReceived.load()) {
//         std::cout << "[OSC] Test result: message was received!\n";
//     } else {
//         std::cout << "[OSC] Test result: no message received.\n";
//     }
//
//     lo_server_thread_stop(st);
//     lo_server_thread_free(st);
//
//     return 0;
// }
