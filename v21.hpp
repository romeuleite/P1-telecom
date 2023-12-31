#ifndef V21_HPP
#define V21_HPP

#include <functional>
#include "config.hpp"

class V21_RX
{
public:
    V21_RX(float omega_mark, float omega_space, std::function<void(const unsigned int *, unsigned int)> get_digital_samples)
        :omega_mark(omega_mark),omega_space(omega_space),get_digital_samples(get_digital_samples) {};
    void demodulate(const float *in_analog_samples, unsigned int n);
private:
    float omega_mark, omega_space;
    std::function<void(const unsigned int *, unsigned int)> get_digital_samples;
    float v0i_past = 00.00, v0r_past = 00.00, v1i_past = 00.00, v1r_past = 00.00;
    float decision_past = 00.00, decision_past_past = 00.00, filtered_decision_past = 00.00, filtered_decision_past_past = 00.00;
};

class V21_TX
{
public:
    V21_TX(float omega_mark, float omega_space) :omega_mark(omega_mark),omega_space(omega_space),phase(0.f) {};
    void modulate(const unsigned int *in_digital_samples, float *out_analog_samples, unsigned int n);
private:
    float omega_mark, omega_space;
    float phase;
};

#endif
