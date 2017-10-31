/* 
   main_cli.c
    - entry point of the non-gui application

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
#include <stdlib.h>
#include <getopt.h>

static int verbose_flag;

static struct option cmd_long_options[] =
{
	/* These options set a flag. */
	{"verbose", no_argument,       &verbose_flag, 1},
	{"brief",   no_argument,       &verbose_flag, 0},
	/* These options don’t set a flag.
	We distinguish them by their indices. */
	{"add",     no_argument,       0, 'a'},
	{"append",  no_argument,       0, 'b'},
	{"delete",  required_argument, 0, 'd'},
	{"create",  required_argument, 0, 'c'},
	{"file",    required_argument, 0, 'f'},
	{0, 0, 0, 0}
};

int main(int argc, char **argv)
{
	int c;

	


	return 0;
}

