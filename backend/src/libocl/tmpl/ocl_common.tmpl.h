/*
 * Copyright © 2012 - 2014 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef __OCL_COMMON_H__
#define __OCL_COMMON_H__

#include "ocl_types.h"

/////////////////////////////////////////////////////////////////////////////
// Common Functions
/////////////////////////////////////////////////////////////////////////////
OVERLOADABLE float step(float edge, float x);
OVERLOADABLE float max(float a, float b);
OVERLOADABLE float min(float a, float b);
OVERLOADABLE float mix(float x, float y, float a);
OVERLOADABLE float clamp(float v, float l, float u);

OVERLOADABLE float degrees(float radians);
OVERLOADABLE float radians(float degrees);
OVERLOADABLE float smoothstep(float e0, float e1, float x);

OVERLOADABLE float sign(float x);

// Half half version.
OVERLOADABLE half step(half edge, half x);
OVERLOADABLE half max(half a, half b);
OVERLOADABLE half min(half a, half b);
OVERLOADABLE half mix(half x, half y, half a);
OVERLOADABLE half clamp(half v, half l, half u);

OVERLOADABLE half degrees(half radians);
OVERLOADABLE half radians(half degrees);
OVERLOADABLE half smoothstep(half e0, half e1, half x);

OVERLOADABLE half sign(half x);

OVERLOADABLE double step(double edge, double x);
OVERLOADABLE double max(double a, double b);
OVERLOADABLE double min(double a, double b);
OVERLOADABLE double mix(double x, double y, double a);
OVERLOADABLE double clamp(double v, double l, double u);
OVERLOADABLE double degrees(double radians);
OVERLOADABLE double radians(double degrees);
OVERLOADABLE double smoothstep(double e0, double e1, double x);
OVERLOADABLE double sign(double x);

