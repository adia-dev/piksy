#include <core/application/application.hpp>
#include <iostream>

int main() {
  try {
    piksy::Application::run();
  } catch (...) {
    std::cerr << "Application exited with an exception\n";
    piksy::Application::shutdown();
    return -1;
  }

  return 0;
}
