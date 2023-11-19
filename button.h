/* Copyright 2023, Darran A Lofthouse
 *
 * This file is part of pico-input.
 *
 * pico-security is free software: you can redistribute it and/or modify it under the terms 
 * of the GNU General Public License as published by the Free Software Foundation, either 
 * version 3 of the License, or (at your option) any later version.
 *
 * pico-security is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with pico-security.
 * If  not, see <https://www.gnu.org/licenses/>. 
 */

#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>

#include "pico/util/queue.h"

#include "event.h"
#include "input.h"

typedef struct {
    uint8_t gpio_number;
    pull_t pull;
    void *user_data;
} button_definition_t;

/**
 *  Register a set of buttons to be scanned. 
 */
void button_registration(button_definition_t *button_definition, uint8_t button_count, uint32_t scan_interval_ms, queue_t *event_queue, input_context_t *input_context);

#endif