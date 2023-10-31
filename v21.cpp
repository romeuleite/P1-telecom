#include <math.h>
#include "v21.hpp"


void V21_RX::demodulate(const float *in_analog_samples, unsigned int n)
{
    unsigned int digital_samples[n];

    int  L = SAMPLES_PER_SYMBOL;
    float T = SAMPLING_PERIOD;

    float buffer_circular[SAMPLES_PER_SYMBOL + 1] = {};
    int bit_counter = 0, bit = 0;

    //Abaixo, temos a implementação das equações do Tretter, que são os filtros passa-bandas em torno das frequências de marca (omega1) e espaço (omega0).
    float r = 0.99;
    for (int i = 0; i < n; i++) {
        buffer_circular[bit] = in_analog_samples[i];
        bit = (bit + 1) % (L + 1);
        float samples_past = buffer_circular[bit];

        float v0r = in_analog_samples[i] - pow(r, L)*cos(omega_space*L*T)*samples_past + r*cos(omega_space*T)*v0r_past - r*sin(omega_space*T)*v0i_past;
        float v0i = -pow(r, L)*sin(omega_space*L*T)*samples_past + r*cos(omega_space*T)*v0i_past + r*sin(omega_space*T)*v0r_past;
        float v1r = in_analog_samples[i] - pow(r, L)*cos(omega_mark*L*T)*samples_past + r*cos(omega_mark*T)*v1r_past - r*sin(omega_mark*T)*v1i_past;
        float v1i = -pow(r, L)*sin(omega_mark*L*T)*samples_past + r*cos(omega_mark*T)*v1i_past + r*sin(omega_mark*T)*v1r_past;

        v0r_past = v0r;
        v0i_past = v0i;
        v1r_past = v1r;
        v1i_past = v1i;

        //Abaixo, definimos um sinal de decisão, que se for positivo indica que devemos gerar nível lógico alto (porque o sinal tinha mais energia na frequência de marca), e se for negativo, indica nível lógico baixo (porque o sinal tinha mais energia na frequência de espaço).
        float decision = (v1r*v1r + v1i*v1i) - (v0r*v0r + v0i*v0i);
        
        // Filtro Butterworth gerado no Octave com corte em 300Hz
        float filtered_decision = 0.00037507*decision + 0.00075014*decision_past + 0.00037507*decision_past_past + 1.9445*filtered_decision_past - 0.9460*filtered_decision_past_past;

        decision_past_past = decision_past;
        decision_past = decision;
        filtered_decision_past_past = filtered_decision_past;
        filtered_decision_past = filtered_decision;

        //Estratégia para detectar a ausência de uma portadora
        if(std::abs(filtered_decision) > 60.00)
        {
            bit_counter = 50;
        }
        else if(bit_counter > 0 && std::abs(filtered_decision) < 50.00)
        {
            bit_counter--;
        }

        if(bit_counter > 0)
            digital_samples[i] =  filtered_decision >= 00.00;
        else
            digital_samples[i] = 1;
    }

    get_digital_samples(digital_samples, n);
}

void V21_TX::modulate(const unsigned int *in_digital_samples, float *out_analog_samples, unsigned int n)
{
    while (n--) {
        *out_analog_samples++ = sin(phase);
        phase += (*in_digital_samples++ ? omega_mark : omega_space) * SAMPLING_PERIOD;

        // evita que phase cresça indefinidamente, o que causaria perda de precisão
        phase = remainder(phase, 2*M_PI);
    }
}
