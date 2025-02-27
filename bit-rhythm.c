#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "ws2812.pio.h"
#include "hardware/pio.h"
#include "hardware/timer.h"

#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6
#define LED_PIN_A 12
#define LED_PIN_B 13
#define LED_PIN_C 14
#define BUZZER_PIN 21
#define WS2812_PIN 7
#define NUM_PIXELS 25
#define IS_RGBW false

#define TON_A 440
#define TON_B 523
#define TON_C 659
#define TON_D 784

static bool smile[NUM_PIXELS] = {
    0, 1, 1, 1, 0,
    1, 0, 0, 0, 1,
    0, 0, 0, 0, 0,
    0, 1, 0, 1, 0,
    0, 1, 0, 1, 0};

static bool sad[NUM_PIXELS] = {
    1, 0, 0, 0, 1,
    0, 1 ,1, 1, 0,
    0, 0, 0, 0, 0,
    0, 1, 0, 1, 0,
    0, 1, 0, 1, 0};

bool led_buffer[NUM_PIXELS];
PIO pio = pio0;
int sm = 0;

void copy_array(bool *dest, const bool *src);
static inline void put_pixel(uint32_t pixel_grb);
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b);
void set_led_pattern(bool *pattern, uint8_t r, uint8_t g, uint8_t b);

void apagar_leds() {
    for (int i = 0; i < NUM_PIXELS; i++) {
        put_pixel(0);
    }
}

void tocar_buzzer(int frequencia, int duracao_ms)
{
    int periodo_us = 500000 / frequencia;
    int meio_ciclo = periodo_us / 2;
    int ciclos = (duracao_ms * 1000) / periodo_us;
    for (int i = 0; i < ciclos; i++)
    {
        gpio_put(BUZZER_PIN, true);
        busy_wait_us(meio_ciclo);
        gpio_put(BUZZER_PIN, false);
        busy_wait_us(meio_ciclo);
    }
}

void musica_acerto()
{
    set_led_pattern(smile, 0, 255, 0);
    tocar_buzzer(TON_A, 200);
    tocar_buzzer(TON_B, 200);
    tocar_buzzer(TON_C, 200);
    tocar_buzzer(TON_D, 400);
    sleep_ms(1000);
    apagar_leds();
}

void musica_erro()
{
    set_led_pattern(sad, 255, 0, 0);
    tocar_buzzer(TON_D, 300);
    tocar_buzzer(TON_C, 300);
    tocar_buzzer(TON_A, 500);
}

void feedback_erro()
{
    for (int i = 0; i < 5; i++)
    {
        gpio_put(LED_PIN_C, true);
        sleep_ms(200);
        gpio_put(LED_PIN_C, false);
        sleep_ms(200);
    }
    musica_erro();
    sleep_ms(1000);
    apagar_leds();
}

void iniciar_jogo()
{
    int sequencia[100];
    int tamanho_sequencia = 0;
    int pontos = 0;

    while (1)
    {
        sequencia[tamanho_sequencia] = rand() % 2;
        tamanho_sequencia++;

        for (int i = 0; i < tamanho_sequencia; i++)
        {
            if (sequencia[i] == 0)
            {
                gpio_put(LED_PIN_A, true);
                tocar_buzzer(TON_A, 300);
                sleep_ms(300);
                gpio_put(LED_PIN_A, false);
            }
            else
            {
                gpio_put(LED_PIN_B, true);
                tocar_buzzer(TON_B, 300);
                sleep_ms(300);
                gpio_put(LED_PIN_B, false);
            }
            sleep_ms(200);
        }

        for (int i = 0; i < tamanho_sequencia; i++)
        {
            int botao = -1;
            while (botao == -1)
            {
                if (gpio_get(BUTTON_A_PIN) == 0)
                {
                    botao = 0;
                    gpio_put(LED_PIN_A, true);
                    tocar_buzzer(TON_A, 300);
                    sleep_ms(300);
                    gpio_put(LED_PIN_A, false);
                }
                else if (gpio_get(BUTTON_B_PIN) == 0)
                {
                    botao = 1;
                    gpio_put(LED_PIN_B, true);
                    tocar_buzzer(TON_B, 300);
                    sleep_ms(300);
                    gpio_put(LED_PIN_B, false);
                }
            }
            if (botao != sequencia[i])
            {
                printf("Erro! Sequência incorreta.\n");
                feedback_erro();
                printf("Você acertou %d pontos!\n", pontos);
                return;
            }
        }
        printf("Você acertou! Pontos: %d\n", pontos);
        musica_acerto();
        pontos++;
        sleep_ms(1000);
    }
}

void copy_array(bool *dest, const bool *src)
{
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        dest[i] = src[i];
    }
}

static inline void put_pixel(uint32_t pixel_grb)
{
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

void set_led_pattern(bool *pattern, uint8_t r, uint8_t g, uint8_t b)
{
    uint32_t color = urgb_u32(r, g, b);
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        if (pattern[i])
        {
            put_pixel(color);
        }
        else
        {
            put_pixel(0);
        }
    }
}

int main()
{
    stdio_init_all();
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);

    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);

    gpio_init(LED_PIN_A);
    gpio_set_dir(LED_PIN_A, GPIO_OUT);

    gpio_init(LED_PIN_B);
    gpio_set_dir(LED_PIN_B, GPIO_OUT);

    gpio_init(LED_PIN_C);
    gpio_set_dir(LED_PIN_C, GPIO_OUT);

    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);

    srand(time_us_64());
    ws2812_program_init(pio, sm, pio_add_program(pio, &ws2812_program), WS2812_PIN, 800000, IS_RGBW);

    iniciar_jogo();

    return 0;
}