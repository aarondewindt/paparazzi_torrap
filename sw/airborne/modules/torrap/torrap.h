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
 * @file "modules/torrap/torrap.h"
 * @author Torrap Team
 * Module for the mavlab competition.
 */

#ifndef TORRAP_H
#define TORRAP_H

#include "modules/computer_vision/cv.h"
#include <inttypes.h>
#include "state.h"

extern int8_t turn;
extern int8_t torrap_active;
extern int8_t direction;
extern int8_t turn_dir;
extern float max_distance;

extern struct video_listener *listener;
extern float yaw_degrees_per_period;
extern uint16_t trajectory_confidence;

extern void torrap_init();
extern struct image_t *torrap_image_handler(struct image_t *img);
extern void torrap_start();
extern void torrap_stop();
extern void torrap_periodic();

uint8_t border_turn(int32_t *heading, float incrementDegrees);
uint8_t border_turn_target_heading_reached();

extern uint8_t moveWaypointForward(uint8_t, float);
extern uint8_t moveWaypoint(uint8_t, struct EnuCoor_i *);
extern uint8_t increase_nav_heading(int32_t *, float);
extern uint8_t chooseRandomIncrementAvoidance(void);

#endif

