#pragma once
#include <Arduino.h>
#include <driver/rmt_rx.h>
#include <driver/rmt_tx.h>

// =========================
// Pin / Frame Configuration
// =========================
#define CH1_GPIO           ((gpio_num_t)7)   // ESP32-S2 GPIO7

#define NUM_CHANNELS        4

static const gpio_num_t CHANNEL_GPIOS[NUM_CHANNELS] = { GPIO_NUM_7, GPIO_NUM_8, GPIO_NUM_9, GPIO_NUM_10 };

#define RX_FRAMES          3
#define RX_BUFFER_SIZE     64
#define DUMMY_FRAME        "11000000111111111110111000000000110000001111111111101110"
#define FRAME_TIMEOUT_US   400000    // 400 ms

// =========================
// TX timing (us)
// =========================
#define HEADER_LOW_US      5900
#define HEADER_HIGH_US     1800
#define BIT0_LOW_US         580
#define BIT0_HIGH_US        500
#define BIT1_LOW_US         580
#define BIT1_HIGH_US       1600
#define FOOTER_LOW_US      7800

// =========================
// RX Timing (us)
// =========================
#define HEADER_LOW_MIN     100
#define HEADER_LOW_MAX     650
#define HEADER_HIGH_MIN   1600
#define HEADER_HIGH_MAX   2200
#define FOOTER_LOW_MIN    7700
#define FOOTER_LOW_MAX    8000

#define BIT_LOW_MIN        250
#define BIT_LOW_MAX        620
#define BIT_HIGH_0_MIN     450
#define BIT_HIGH_0_MAX     900
#define BIT_HIGH_1_MIN    1450
#define BIT_HIGH_1_MAX    2200
