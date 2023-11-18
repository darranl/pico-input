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

#ifndef INPUT_H
#define INPUT_H

typedef struct {
    void *context_data;  // The caller doesn't need to know what this is.
} input_context_t;

/// Indicate if this should pull up or down.
typedef enum {
    pull_up, pull_down
} pull_t;

#endif