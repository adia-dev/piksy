#include <core/application.hpp>
#include <core/logger.hpp>

int main() {
    try {
        piksy::core::Application app;
        app.run();
    } catch (const std::runtime_error& ex) {
        piksy::core::Logger::fatal("Application exited with a runtime exception:\nException: %s",
                                   ex.what());
        return -1;
    } catch (...) {
        piksy::core::Logger::fatal("Application exited with an unknown exception.");
        return -1;
    }
}
