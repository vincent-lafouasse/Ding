//
// Created by vincent lafouasse on 29/03/2025.
//

#ifndef INTEGRATOR_HPP
#define INTEGRATOR_HPP

#include <array>
#include <cstddef>

template <std::size_t nChannels, typename FloatType>
class Integrator {
   public:
    Integrator(FloatType inertia_weight, FloatType forward_weight)
        : inertia_weight_(inertia_weight), forward_weight_(forward_weight) {}
    ~Integrator() = default;

    void setTarget(FloatType target) { this->target_ = target; }

    FloatType get(std::size_t channel) {
        memory_[channel] =
            inertia_weight_ * memory_[channel] + forward_weight_ * target_;
        return memory_[channel];
    }

    FloatType get() { return this->get(0); }

   private:
    FloatType target_{};
    FloatType inertia_weight_;
    FloatType forward_weight_;
    std::array<FloatType, nChannels> memory_{};
};

#endif  // INTEGRATOR_HPP
