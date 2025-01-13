#pragma once

#include "utilities/json.hpp"

namespace piksy {
namespace rendering {
struct Frame {
    Frame(int x, int y, int w, int h) : x(x), y(y), w(w), h(h), data(nlohmann::json::object()) {}

    int x, y, w, h;
    bool flipped = false;

    nlohmann::json data;

    template <typename T>
    void set_data(const std::string& key, T value) {
        data[key] = value;
    }

    template <typename T>
    T get_data(const std::string& key, T default_value = T{}) const {
        try {
            return data.contains(key) ? data[key].get<T>() : default_value;
        } catch (const nlohmann::json::exception&) {
            return default_value;
        }
    }

    bool has_data(const std::string& key) const { return data.contains(key); }

    void remove_data(const std::string& key) {
        if (data.contains(key)) {
            data.erase(key);
        }
    }
};
}  // namespace rendering
}  // namespace piksy
