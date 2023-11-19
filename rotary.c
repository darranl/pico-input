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

#include <stdlib.h>

#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/util/queue.h"

#include "rotary.h"

// This implementation is just about two pins on a rotary encoder and subsequently handling 
// timer events to check the states and add emit events to the provided queue.
//
// Buttons are handled within button.h

struct input_context
{   
    repeating_timer_t timer;
    queue_t *event_queue;
    uint8_t first_gpio;
    uint8_t second_gpio;
    uint last_pos;
    uint last_rest;
    void *user_data;
};

static bool timer_callback(repeating_timer_t *rt);
static uint rotary_read(uint first_gpio, uint second_gpio);

void rotary_registration(rotary_definition_t *rotary_definition, uint32_t scan_interval_ms, queue_t *event_queue, input_context_t *input_context)
{
    struct input_context *local_context = (struct input_context*) calloc(1, sizeof(struct input_context));
    local_context->first_gpio = rotary_definition->first_gpio;
    local_context->second_gpio = rotary_definition->second_gpio;
    local_context->user_data = rotary_definition->user_data;
    local_context->event_queue = event_queue;

    gpio_init(local_context->first_gpio);
    rotary_definition->pull == pull_up ? gpio_pull_up(local_context->first_gpio) : gpio_pull_down(local_context->first_gpio);
    gpio_init(local_context->second_gpio);
    rotary_definition->pull == pull_up ? gpio_pull_up(local_context->second_gpio) : gpio_pull_down(local_context->second_gpio);

    local_context->last_pos = local_context->last_rest = rotary_read(local_context->first_gpio, local_context->second_gpio);
    if (local_context->last_rest != 0b00 && local_context->last_rest != 0b11)
    {
        // At a mid point so pick an arbitrary rest point.
        local_context->last_rest = 0x00;
    }

    add_repeating_timer_ms(scan_interval_ms, timer_callback, local_context, &local_context->timer);
    input_context->context_data = local_context;
}

static uint rotary_read(uint first_gpio, uint second_gpio)
{
    uint response = gpio_get(first_gpio);
    response = response << 1;
    response = response | gpio_get(second_gpio);

    return response;
}

static bool timer_callback(repeating_timer_t *rt)
{
    struct input_context *local_context = (struct input_context *)rt->user_data;
    uint current_pos = rotary_read(local_context->first_gpio, local_context->second_gpio);

    switch (local_context->last_pos)
    {
    case 0b00:
    case 0b11:
        if (current_pos == 0b10 || current_pos == 0b01)
        {
            // A rotation has begun but not completed.
            local_context->last_pos = current_pos;
        }
        break;
    case 0b01:
    case 0b10:
        if ((local_context->last_rest == 0b00 && current_pos == 0b11) || 
                (local_context->last_rest == 0b11 && current_pos == 0b00))
        {
            // The boolean expression here is the two transitions that would lead to a positive
            // rotation on returning to a rest position.
            bool current_state = (local_context->last_pos == 0b01 && current_pos == 0b11) || 
                                     (local_context->last_pos == 0b10 && current_pos == 0b00);
            button_event_t event = {
                current_state,
                local_context->user_data
            };

            queue_try_add(local_context->event_queue, &event);
            local_context->last_rest = local_context->last_pos = current_pos;
        }
        else if (local_context->last_rest == current_pos)
        {
            // Just moved back half a step.
            local_context->last_pos = current_pos;
        }
        break;
    }

    return true;
}
