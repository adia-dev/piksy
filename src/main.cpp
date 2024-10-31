#include <core/application/application.hpp>
#include <iostream>

int main() {
  try {
    piksy::Application::run();
  } catch (const std::runtime_error &ex) {
    std::cerr << "Application exited with a runtime exception:\nException: "
              << ex.what() << std::endl;
    return -1;
  } catch (...) {
    std::cerr << "Application exited with an unexpected exception\n";
    return -1;
  }
}
