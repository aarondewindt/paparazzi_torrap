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
#include "firmwares/rotorcraft/navigation.h"
#include "generated/flight_plan.h"

struct video_listener *listener = NULL;

int8_t direction = 1;
int8_t turn_dir = 1;
int8_t turn = 0;
int8_t torrap_active = 0;

float yaw_degrees_per_period = 10.0;
uint16_t trajectory_confidence   = 1;
float max_distance = 1.35;


void torrap_init() {
	// Initialize video listener
	listener = cv_add_to_device(&TORRAP_CAMERA, torrap_image_handler);
}

struct image_t *torrap_image_handler(struct image_t *img) {
	// Get direction estimate
	struct TorrapDirectionResult* dir_result = torrap_direction_detection(img);
	direction = dir_result->direction;
	turn = dir_result->turn;
	return img;
}

void torrap_start() {}

void torrap_stop() {}

void torrap_periodic() {
//	printf("Periodic running\n");

	if (torrap_active) {
		if(!turn){
				  float move_distance = fmin(max_distance, 0.05 * trajectory_confidence);
				  moveWaypointForward(WP_GOAL, move_distance);
				  moveWaypointForward(WP_TRAJECTORY, 1.25 * move_distance);
				  nav_set_heading_towards_waypoint(WP_GOAL);
				  trajectory_confidence += 1;
				  turn_dir = 0;
			  }
			  else{
				  if (turn_dir == 0) {
					  turn_dir = direction;
				  }
				  float move_distance = 0.05;
				  waypoint_set_here_2d(WP_GOAL);
				  waypoint_set_here_2d(WP_TRAJECTORY);
				  increase_nav_heading(&nav_heading, turn_dir * yaw_degrees_per_period);
				  if(trajectory_confidence > 5){
					  trajectory_confidence -= 4;
				  }
				  else{
					  trajectory_confidence = 1;
				  }
			  }
	} else {
//		moveWaypointForward(WP_GOAL, 1.);
		moveWaypointForward(WP_TRAJECTORY, 1.);
	}


}


/*
 * Increases the NAV heading. Assumes heading is an INT32_ANGLE. It is bound in this function.
 */
uint8_t increase_nav_heading(int32_t *heading, float incrementDegrees)
{
  struct Int32Eulers *eulerAngles   = stateGetNedToBodyEulers_i();
  int32_t newHeading = eulerAngles->psi + ANGLE_BFP_OF_REAL( incrementDegrees / 180.0 * M_PI);
  // Check if your turn made it go out of bounds...
  INT32_ANGLE_NORMALIZE(newHeading); // HEADING HAS INT32_ANGLE_FRAC....
  *heading = newHeading;
//  printf("Increasing heading to %f\n", ANGLE_FLOAT_OF_BFP(*heading) * 180 / M_PI);
  return false;
}


int32_t border_target_heading = 0;
uint8_t border_turn(int32_t *heading, float incrementDegrees) {
	struct Int32Eulers *eulerAngles   = stateGetNedToBodyEulers_i();
	int32_t newHeading = eulerAngles->psi + ANGLE_BFP_OF_REAL( incrementDegrees / 180.0 * M_PI);
	// Check if your turn made it go out of bounds...
	INT32_ANGLE_NORMALIZE(newHeading); // HEADING HAS INT32_ANGLE_FRAC....
//	*heading = newHeading;
	border_target_heading = newHeading;
	//  printf("Increasing heading to %f\n", ANGLE_FLOAT_OF_BFP(*heading) * 180 / M_PI);
	return false;
}

uint8_t border_turn_target_heading_reached() {
	struct Int32Eulers *eulerAngles   = stateGetNedToBodyEulers_i();
	if (abs(eulerAngles->psi - border_target_heading) < ANGLE_BFP_OF_REAL( 10 / 180.0 * M_PI)) {
		return true;
	}else {
		return false;
	}
}


/*
 * Calculates coordinates of a distance of 'distanceMeters' forward w.r.t. current position and heading
 */
uint8_t calculateForwards(struct EnuCoor_i *new_coor, float distanceMeters)
{
  struct EnuCoor_i *pos             = stateGetPositionEnu_i(); // Get your current position
  struct Int32Eulers *eulerAngles   = stateGetNedToBodyEulers_i();
  // Calculate the sine and cosine of the heading the drone is keeping
  float sin_heading                 = sinf(ANGLE_FLOAT_OF_BFP(eulerAngles->psi));
  float cos_heading                 = cosf(ANGLE_FLOAT_OF_BFP(eulerAngles->psi));
  // Now determine where to place the waypoint you want to go to
  new_coor->x                       = pos->x + POS_BFP_OF_REAL(sin_heading * (distanceMeters));
  new_coor->y                       = pos->y + POS_BFP_OF_REAL(cos_heading * (distanceMeters));
//  printf("Calculated %f m forward position. x: %f  y: %f based on pos(%f, %f) and heading(%f)\n", distanceMeters, POS_FLOAT_OF_BFP(new_coor->x), POS_FLOAT_OF_BFP(new_coor->y), POS_FLOAT_OF_BFP(pos->x), POS_FLOAT_OF_BFP(pos->y), ANGLE_FLOAT_OF_BFP(eulerAngles->psi)*180/M_PI);
  return false;
}

/*
 * Sets waypoint 'waypoint' to the coordinates of 'new_coor'
 */
uint8_t moveWaypoint(uint8_t waypoint, struct EnuCoor_i *new_coor)
{
//  printf("Moving waypoint %d to x:%f y:%f\n", waypoint, POS_FLOAT_OF_BFP(new_coor->x), POS_FLOAT_OF_BFP(new_coor->y));
  waypoint_set_xy_i(waypoint, new_coor->x, new_coor->y);
  return false;
}

/*
 * Calculates coordinates of distance forward and sets waypoint 'waypoint' to those coordinates
 */
uint8_t moveWaypointForward(uint8_t waypoint, float distanceMeters)
{
  struct EnuCoor_i new_coor;
  calculateForwards(&new_coor, distanceMeters);
  moveWaypoint(waypoint, &new_coor);
  return false;
}



///*
// * Sets the variable 'incrementForAvoidance' randomly positive/negative
// */
//uint8_t chooseRandomIncrementAvoidance()
//{
//  // Randomly choose CW or CCW avoiding direction
//  int r = rand() % 2;
//  if (r == 0) {
//    incrementForAvoidance = 10.0;
//    VERBOSE_PRINT("Set avoidance increment to: %f\n", incrementForAvoidance);
//  } else {
//    incrementForAvoidance = -10.0;
//    VERBOSE_PRINT("Set avoidance increment to: %f\n", incrementForAvoidance);
//  }
//  return false;
//}





