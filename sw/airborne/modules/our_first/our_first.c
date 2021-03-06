/*
 * Copyright (C) Torrap
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
 * @file "modules/our_first/our_first.c"
 * @author Torrap
 * Our first module
 */

#include "modules/our_first/our_first.h"
#include <stdio.h>


 void first_init() {
	 printf("Hello World\n");
 }

void first_periodic() {
	printf("Hello World again, 0000000000000000000000000000000000000000000000000000\n");
}


int spentLongTimeInBlock(int timeInBlock){
  return timeInBlock > 5;
}
