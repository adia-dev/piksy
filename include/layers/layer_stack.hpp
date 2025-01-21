#pragma once

#include <algorithm>
#include <core/logger.hpp>
#include <layers/layer.hpp>
#include <memory>
#include <vector>

namespace piksy {
namespace layers {

class LayerStack {
   public:
    LayerStack() = default;
    ~LayerStack() {
        for (auto it = m_layers.rbegin(); it != m_layers.rend(); ++it) {
            (*it)->on_detach();
        }
    }

    void push_layer(std::unique_ptr<Layer> layer) {
        layer->on_attach();
        m_layers.emplace(m_layers.begin() + m_layerInsertIndex, std::move(layer));
        m_layerInsertIndex++;
    }

    template <typename T, typename... Args>
    void push_layer(Args&&... args) {
        static_assert(std::is_base_of<Layer, T>::value, "T must derive from Layer");
        auto layer = std::make_unique<T>(std::forward<Args>(args)...);
        Layer* rawLayerPtr = layer.get();

        m_layers.emplace(m_layers.begin() + m_layerInsertIndex, std::move(layer));
        m_layerInsertIndex++;

        rawLayerPtr->on_attach();
    }

    void push_overlay(std::unique_ptr<Layer> overlay) {
        overlay->on_attach();
        m_layers.emplace_back(std::move(overlay));
    }

    template <typename T, typename... Args>
    void push_overlay(Args&&... args) {
        static_assert(std::is_base_of<Layer, T>::value, "T must derive from Layer");
        auto layer = std::make_unique<T>(std::forward<Args>(args)...);
        Layer* rawOverlay = layer.get();

        m_layers.emplace_back(std::move(layer));

        rawOverlay->on_attach();
    }

    void pop_layer(Layer* layer) {
        auto it = std::find_if(m_layers.begin(), m_layers.begin() + m_layerInsertIndex,
                               [&](const std::unique_ptr<Layer>& l) { return l.get() == layer; });
        if (it != (m_layers.begin() + m_layerInsertIndex)) {
            (*it)->on_detach();
            m_layers.erase(it);
            m_layerInsertIndex--;
        } else {
            core::Logger::warn("pop_layer(): Layer not found among normal layers!");
        }
    }

    void pop_overlay(Layer* overlay) {
        auto it = std::find_if(m_layers.begin() + m_layerInsertIndex, m_layers.end(),
                               [&](const std::unique_ptr<Layer>& l) { return l.get() == overlay; });
        if (it != m_layers.end()) {
            (*it)->on_detach();
            m_layers.erase(it);
        } else {
            core::Logger::warn("pop_overlay(): Overlay not found!");
        }
    }

    std::vector<std::unique_ptr<Layer>>& layers() { return m_layers; }

   private:
    std::vector<std::unique_ptr<Layer>> m_layers;
    unsigned int m_layerInsertIndex = 0;
};

}  // namespace layers
}  // namespace piksy
