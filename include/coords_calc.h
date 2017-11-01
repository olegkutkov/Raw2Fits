/* 
   coords_calc.h

   Copyright 2017  Oleg Kutkov <elenbert@gmail.com>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  
 */

#ifndef __COORDS_CALC_H__
#define __COORDS_CALC_H__

float coord_any_to_float_deg(const char *str);
void deg_to_sexigesimal_str(float deg, char *dst);
float coordinates_to_deg(short hour, short min, short sec, short msec);
void coordinates_to_sexigesimal_str(short hour, short min, short sec, short msec, char *dst);

#endif

