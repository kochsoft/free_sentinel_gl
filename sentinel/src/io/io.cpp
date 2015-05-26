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

#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <map>

#include "io.h"

using std::setw;
using std::setfill;
using std::endl;
using std::stringstream;
using std::istringstream;
using std::ostringstream;
using std::getline;
using std::map;

namespace mhk_gl
{
//> class Png_data. --------------------------------------------------
Png_data::Png_data()
{
	this->width = 0;
	this->height = 0;
	this->bytes_per_pixel = 0;
	this->bit_depth = 0;
	this->color_type = 0;
	this->data = 0;
}

Png_data::Png_data(Png_data& input)
{
	this->width = input.width;
	this->height = input.height;
	this->bytes_per_pixel = input.bytes_per_pixel;
	this->bit_depth = input.bit_depth;
	this->color_type = input.color_type;
	this->data = input.data;
}

Png_data::Png_data(int width, int height, int bytes_per_pixel,
		png_byte bit_depth, png_byte color_type, png_bytep* data)
{
	this->width = width;
	this->height = height;
	this->bytes_per_pixel = bytes_per_pixel;
	this->bit_depth = bit_depth;
	this->color_type = color_type;
	this->data = data;
}

uchar* Png_data::copy_to_linear()
{
	uchar* arr = (uchar*)malloc(sizeof(uchar)*bytes_per_pixel*width*height);
	for (int j=0;j<height;j++) for (int k=0;k<width*bytes_per_pixel;k++)
		arr[j*width*bytes_per_pixel+k] = data[j][k];
	return arr;
}

string Png_data::toString()
{
	ostringstream oss;
	oss << "Width: " << width << ", height: " << height <<
		", bytes per pixel: " << bytes_per_pixel <<
		", bit depth: " << bit_depth <<
		", color_type: " << ((int)color_type) <<
		", data pointer: " << data;
	return oss.str();
}
//< ------------------------------------------------------------------

//> class Io. --------------------------------------------------------
Io::Io(E_DEBUG_LEVEL debug_level, ostream* stdout, ostream* stderr)
{
	this->debug_level = debug_level;
	this->stdout = stdout;
	this->stderr = stderr;
}

string Io::debug_level_to_string(E_DEBUG_LEVEL debug_level)
{
	string dl;
	switch (debug_level)
	{
		case E_DEBUG_LEVEL::VERBOSE: dl = "VERBOSE"; break;
		case E_DEBUG_LEVEL::MESSAGE: dl = "MESSAGE"; break;
		case E_DEBUG_LEVEL::WARNING: dl = "WARNING"; break;
		case E_DEBUG_LEVEL::ERROR  : dl = "ERROR";   break;
		case E_DEBUG_LEVEL::QUIET  : dl = "QUIET";   break;
		default: dl = "UNKNOWN";
	}
	return dl;
}

string Io::get_date_time_string()
{
    time_t t;
	time(&t);
	struct tm* now;
	now = localtime(&t);
	// http://www.cplusplus.com/reference/ctime/tm/
	ostringstream oss;
	oss << "[" << (now->tm_year + 1900) << '/' <<
		setfill('0') << setw(2) << (now->tm_mon + 1) << '/' <<
		setfill('0') << setw(2) << now->tm_mday << ";" <<
		setfill('0') << setw(2) << now->tm_hour << ":" <<
		setfill('0') << setw(2) << now->tm_min << ":" <<
		setfill('0') << setw(2) << now->tm_sec << "]";
	return oss.str();
}

string Io::print(E_DEBUG_LEVEL debug_level, string prologue, string message, bool doDate)
{
	ostringstream oss;
	if (doDate) oss << get_date_time_string().c_str() << " ";
	oss << debug_level_to_string(debug_level).c_str() << " ";
	if (prologue.size() > 0) oss << prologue.c_str() << ": ";
	oss << message.c_str();
	if (debug_level >= this->debug_level)
	{
		ostream* out = (debug_level <= E_DEBUG_LEVEL::MESSAGE) ? stdout : stderr;
		if (out != 0)
		{
			(*out) << oss.str().c_str();
		}
	}
	return oss.str();
}

string Io::println(E_DEBUG_LEVEL debug_level, string prologue, string message, bool doDate)
{
	ostringstream oss;
	oss << message.c_str() << endl;
	return print(debug_level, prologue, oss.str(), doDate);
}

string Io::build_dfname(string dname, string fname)
{
	if (dname.length()==0) return fname;
	char last = dname.c_str()[dname.length()-1];
	return (last == '/') ? dname + fname : dname + "/" + fname;
}

void Io::get_ifstream(string pfname, ifstream& in, std::ios_base::openmode mode)
{
	in.open(pfname.c_str(), mode);
}

string Io::read_file(istream& in)
{
	ostringstream oss;
	while (in.good())
	{
		char c = in.get();
		oss << c;
	}
	return oss.str().substr(0,oss.str().length()-1);
}

vector<string> Io::read_file_by_lines(istream& in, bool drop_leading_ws, char comment_char, bool drop_empty_lines)
{
	vector <string> res;
	string line;
	while (std::getline(in,line))
	{
		if (drop_leading_ws)
		{
			istringstream iss(line);
			// Drops leading whitespace.
			iss >> std::ws;
			getline(iss,line);
		}
		if ((line.size()==0 && !drop_empty_lines) ||
			(comment_char!=0 && line[0]!=comment_char))
		{
			res.push_back(line);
		}
	}
	return res;
}
//< ------------------------------------------------------------------
}
