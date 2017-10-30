/* 
   coords_calc.c
    - auxilarity functions for coordinates calculations

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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "coords_calc.h"

static float parse_sexigesimal_str(const char *str)
{
	int tok_cnt = 0;
	const char s[2] = ":";
	char *token;
	float splitted[3];

	token = strtok((char *)str, s);

	while( token != NULL ) {
		if (tok_cnt >= 3) {
			return 0;
		}

		splitted[tok_cnt] = atof(token);
		token = strtok(NULL, s);

		tok_cnt++;
	}

	if (splitted[0] < 0) {
		splitted[1] *= -1;
		splitted[2] *= -1;
	}

	return splitted[0] + splitted[1] / 60 + splitted[2] / 3600;
}

static float parse_deg_str(const char *str)
{
	return atof(str);
}

float coord_any_to_float_deg(const char *str)
{
	if (strlen(str) == 0) {
		return 0;
	}

	if (strchr(str, ':') != NULL) {
		return parse_sexigesimal_str(str);
	} else if (strchr(str, '.') != NULL) {
		return parse_deg_str(str);
	}

	return 0;
}

inline double frac(const double x)
{
	return x - floor(x);
}

void deg_to_sexigesimal_str(float deg, char *dst)
{
	char buf[15] = { 0 };	
	int hour = deg / 15;
	int min = abs((int)(frac(deg / 15) * 60));
	float sec = abs((int)(frac(frac(deg / 15) * 60) * 60));

	snprintf(buf, 14, "%i:%i:%.5f", hour, min, sec);
	strcpy(dst, buf);
}

