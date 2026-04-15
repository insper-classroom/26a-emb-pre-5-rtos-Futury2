/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"

const int BTN_PIN_R = 28;
const int BTN_PIN_Y = 21;

const int LED_PIN_R = 5;
const int LED_PIN_Y = 10;

QueueHandle_t xQueueLedR;
QueueHandle_t xQueueLedY;

SemaphoreHandle_t xSemaphore_r;
SemaphoreHandle_t xSemaphore_y;

void btn_callback(uint gpio, uint32_t events) {
    if (events == GPIO_IRQ_EDGE_FALL && gpio == BTN_PIN_R) {
        xSemaphoreGiveFromISR(xSemaphore_r, 0);
    } else if (events == GPIO_IRQ_EDGE_FALL && gpio == BTN_PIN_Y) {
        xSemaphoreGiveFromISR(xSemaphore_y, 0);
    }
}

void led_r_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);
    gpio_put(LED_PIN_R, 0);

    uint8_t ativo = 0;
    uint8_t msg = 0;

    while (true) {
        if (xQueueReceive(xQueueLedR, &msg, 0) == pdTRUE) {
            ativo = msg;
        }

        if (ativo) {
            gpio_put(LED_PIN_R, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        } else {
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

void btn_r_task(void *p) {
    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);

    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true, &btn_callback);

    uint8_t ativo = 0;

    while (true) {
        if (xSemaphoreTake(xSemaphore_r, portMAX_DELAY) == pdTRUE) {
            ativo = !ativo;
            xQueueOverwrite(xQueueLedR, &ativo);
        }
    }
}

void led_y_task(void *p) {
    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);
    gpio_put(LED_PIN_Y, 0);

    uint8_t ativo = 0;
    uint8_t msg = 0;

    while (true) {
        if (xQueueReceive(xQueueLedY, &msg, 0) == pdTRUE) {
            ativo = msg;
        }

        if (ativo) {
            gpio_put(LED_PIN_Y, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(LED_PIN_Y, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        } else {
            gpio_put(LED_PIN_Y, 0);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

void btn_y_task(void *p) {
    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);

    gpio_set_irq_enabled_with_callback(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true, &btn_callback);

    uint8_t ativo = 0;

    while (true) {
        if (xSemaphoreTake(xSemaphore_y, portMAX_DELAY) == pdTRUE) {
            ativo = !ativo;
            xQueueOverwrite(xQueueLedY, &ativo);
        }
    }
}

int main() {
    stdio_init_all();
     printf("Start RTOS\n"); 

    xQueueLedR = xQueueCreate(1, sizeof(uint8_t));
    xQueueLedY = xQueueCreate(1, sizeof(uint8_t));

    xSemaphore_r = xSemaphoreCreateBinary();
    xSemaphore_y = xSemaphoreCreateBinary();

    xTaskCreate(led_r_task, "LED_R", 256, NULL, 1, NULL);
    xTaskCreate(btn_r_task, "BTN_R", 256, NULL, 1, NULL);
    xTaskCreate(led_y_task, "LED_Y", 256, NULL, 1, NULL);
    xTaskCreate(btn_y_task, "BTN_Y", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true) {
    }

    return 0;
}