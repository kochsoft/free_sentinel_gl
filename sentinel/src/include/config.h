/**
 * Sentinel Gl -- an OpenGL based remake of the Firebird classic the Sentinel.
 * Copyright (C) May 25th, 2015 Markus-Hermann Koch, mhk@markuskoch.eu
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * */

#ifndef MHK_CONFIG_H
#define MHK_CONFIG_H

#define PI 3.1415926535

#define DEFAULT_VERBOSITY MESSAGE
#define DEFAULT_DEPTH_BUFFER_SIZE 24
#define DEFAULT_FRAMERATE 24
#define DEFAULT_OPENING_MIN 20.0
#define DEFAULT_OPENING_ANGLE 45.0
#define DEFAULT_OPENING_MAX 70.0
#define DEFAULT_NEAR_PLANE 0.4
#define DEFAULT_FAR_PLANE 200.0
// Rest light in shadow for diffuse lighting fragment shaders.
#define DEFAULT_LIGHT_AMBIENCE 0.35

// Maximum offset for e.g. Dialog_setup_game::generate_campaign_code. Repitition
// value for a given combination of n-values during campaign code generation.
#define DEFAULT_CAMPAIGN_MAX_OFFSET 7

// Default time it takes to manifest or disintegrate an object in seconds.
#define DEFAULT_FADING_TIME 1.0
#define DEFAULT_ENERGY_UNITS 8
// Hyperdrive coil chargin time in ms.
#define DEFAULT_HYPERDRIVE_CHARGING_TIME 2500.0
#define DEFAULT_MEANIE_SPEED_FACTOR 4.0

// Default increase/decrease of FOV when pressing '+' or '-' keys.
#define DEFAULT_DELTA_ZOOM 3.0
#endif
