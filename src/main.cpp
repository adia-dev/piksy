#include <core/application.hpp>
#include <core/logger.hpp>
#include <exception>

int main() {
    try {
        piksy::core::Application app;
        app.run();
    } catch (const std::runtime_error& ex) {
        piksy::core::Logger::fatal("Application exited with a runtime exception:\nException: %s",
                                   ex.what());
        return -1;
    } catch (const std::exception& ex) {
        piksy::core::Logger::fatal("Application exited with an exception: %s", ex.what());

        return -1;
    }
}
