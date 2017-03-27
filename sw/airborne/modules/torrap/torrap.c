/*
 * Copyright (C) Torrap Team
 *
 * This file is part of paparazzi
 *
 * paparazzi is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * paparazzi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with paparazzi; see the file COPYING.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
/**
 * @file "modules/torrap/torrap.c"
 * @author Torrap Team
 * Module for the mavlab competition.
 */

#include <stdio.h>
#include "modules/torrap/torrap.h"
#include "modules/torrap/torrap_direction_detection.h"
#include "torrap_direction_detection.h"
#include "modules/computer_vision/lib/vision/image.h"

struct video_listener *listener = NULL;

int8_t direction = 0;

void torrap_init() {
	// Initialize video listener
	listener = cv_add_to_device(&TORRAP_CAMERA, torrap_image_handler);
}

struct image_t *torrap_image_handler(struct image_t *img) {
	// Get direction estimate
	struct TorrapDirectionResult* dir_result = torrap_direction_detection(img);
	direction = dir_result->direction;
	return img;
}

void torrap_start() {}

void torrap_stop() {}

void torrap_periodic() {
//	printf("Direction: %d\n", direction);

}


