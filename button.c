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

#include "button.h"

// This implementation is just about simple on / off buttons, their registration
// and subsequently handling timer events to check the states and add emit events
// to the provided queue.

struct registered_button
{
    uint8_t gpio_number;
    pull_t pull;
    bool last_state;
    void *user_data;
};

struct input_context
{
    uint8_t button_count;    
    repeating_timer_t timer;
    queue_t *event_queue;
    struct registered_button registered_buttons[];
};

static bool timer_callback(repeating_timer_t *rt);

void button_registration(button_definition_t *button_definition, uint8_t button_count, uint32_t scan_interval_ms, queue_t *event_queue, input_context_t *input_context)
{
    struct input_context *local_context = (struct input_context*) malloc(sizeof(struct input_context) + button_count * sizeof(struct registered_button));    
    // Move to -> syntax as it is now a pointer to the struct.
    local_context->button_count = button_count;
    local_context->event_queue = event_queue;

    for (uint8_t i = 0 ; i < button_count ; i++) {
        button_definition_t current_definition = button_definition[i];
        struct registered_button *current_registered = &local_context->registered_buttons[i];
        
        gpio_init(current_definition.gpio_number);
        current_definition.pull == pull_up ? gpio_pull_up(current_definition.gpio_number) : gpio_pull_down(current_definition.gpio_number);

        bool current_state = gpio_get(current_definition.gpio_number);
        // If pulled up false means it has been pressed to invert the status.
        current_state = current_definition.pull == pull_up ? !current_state : current_state;

        current_registered->gpio_number = current_definition.gpio_number;
        current_registered->pull = current_definition.pull;
        current_registered->last_state = current_state;
        current_registered->user_data = current_definition.user_data;
    }

    add_repeating_timer_ms(scan_interval_ms, timer_callback, local_context, &local_context->timer);
    input_context->context_data = local_context;  
}

static bool timer_callback(repeating_timer_t *rt) 
{
    struct input_context *local_context = (struct input_context*) rt->user_data;

    for (uint8_t i = 0; i< local_context->button_count ; i++) {
        struct registered_button *current_registered = &local_context->registered_buttons[i];
        bool current_state = gpio_get(current_registered->gpio_number);
        // If pulled up false means it has been pressed to invert the status.
        current_state = current_registered->pull == pull_up ? !current_state : current_state;

        if (current_state != current_registered->last_state) {
            button_event_t event = {
                current_state,
                current_registered->user_data
            };
            current_registered->last_state = current_state;
            queue_try_add(local_context->event_queue, &event);
        }
    }

    return true;
}


