#include "uart.hpp"

//teste

void UART_RX::put_samples(const unsigned int *buffer, unsigned int n)
{
    static unsigned int bit_counter = 0; 
    static unsigned int noisy_counter = 0;
    static uint8_t byte = 0;

    //Máquina de estados 
    static enum class State {
        SINCRONIZING,
        RECEIVING,
        WAITING
    } state = State::WAITING;

    for (unsigned int i = 0; i < n; i++) {
        unsigned int sample = buffer[i];

        switch (state) {
            case State::WAITING:
                if (sample == 0) {
                    state = State::SINCRONIZING;
                }
            break;

            // Verifica se 25 dentre as ultimas 30 amostras 
            // lidas possuem nível lógico baixo
            case State::SINCRONIZING:
                bit_counter++;

                if (sample == 0) {
                    if (bit_counter == 30) {
                        state = State::RECEIVING;
                        byte = 0;
                        noisy_counter = 0;
                    }
                } else {
                    noisy_counter++;

                    if (noisy_counter > 5) {
                        state = State::WAITING;
                        bit_counter = 0;
                        noisy_counter = 0;
                    }
                }

            break;

            // Lê um bit de dado a cada 160 amaostras, iniciando a
            // partir do bit_counter = 80 (meio do primeiro intervalo de dados)
            case State::RECEIVING:
                bit_counter++;

                if ((bit_counter % SAMPLES_PER_SYMBOL - (SAMPLES_PER_SYMBOL / 2)) == 0) { 
                    byte |= (sample << (bit_counter / SAMPLES_PER_SYMBOL - 1));

                    if (bit_counter >= (9 * SAMPLES_PER_SYMBOL)) {
                        state = State::WAITING;
                        bit_counter = 0;
                        noisy_counter = 0;
                        get_byte(byte);
                    }
                }

            break;
        }
    }
}

void UART_TX::put_byte(uint8_t byte)
{
    samples_mutex.lock();
    put_bit(0);  // start bit
    for (int i = 0; i < 8; i++) {
        put_bit(byte & 1);
        byte >>= 1;
    }
    put_bit(1);  // stop bit
    samples_mutex.unlock();
}

void UART_TX::get_samples(unsigned int *buffer, unsigned int n)
{
    samples_mutex.lock();
    std::vector<unsigned int>::size_type i = 0;
    while (!samples.empty() && i < n) {
        buffer[i++] = samples.front();
        samples.pop_front();
    }
    samples_mutex.unlock();

    while (i < n) {
        // idle
        buffer[i++] = 1;
    }
}

void UART_TX::put_bit(unsigned int bit)
{
    for (int i = 0; i < SAMPLES_PER_SYMBOL; i++) {
        samples.push_back(bit);
    }
}
