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

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <QString>
#include <QDataStream>
#include "io_qt.h"

using std::cout;
using std::cerr;
using std::endl;
using std::stringstream;
using std::istringstream;
using std::ostringstream;

namespace mhk_gl
{
Io_Qt::Io_Qt(QWidget* parent, E_DEBUG_LEVEL debug_level, ostream* stdout,
	ostream* stderr) : Io(debug_level, stdout, stderr)
{
	this->parent = parent;
}

bool Io_Qt::get_stringstream_from_QFile(QFile& qfile, stringstream& ss)
{
	bool opened_it_myself = false;
	if (qfile.exists() && !qfile.isOpen())
	{
		qfile.open(QIODevice::ReadOnly);
		opened_it_myself = true;
	}
	if (!qfile.exists())
	{
		cerr << "Warning: Io_Qt::get_istringstream_from_QFile(..): Target file '" <<
		  qfile.fileName().toStdString().c_str() <<
		  "' does not seem to exist. istringstream will be empty." << endl;
	}
	// http://interest.qt-project.narkive.com/b1PQl7bF/binary-file-embedded-in-resources-to-std-istream
	QDataStream in(&qfile);
	uint len = qfile.size();
	char* c = (char*)malloc(len*sizeof(char));
	in.readRawData(c,len);
	ss.write(c,len);
    free (c);
	if (opened_it_myself) qfile.close();
	return qfile.isOpen();
}

bool Io_Qt::get_stringstream_from_QFile(string pfname, stringstream& ss)
{
	QFile qfile(pfname.c_str());
	get_stringstream_from_QFile(qfile,ss);
	return qfile.exists();
}

QStringList Io_Qt::vector_string_to_QStringList(vector<string>& input)
{
	QStringList res;
	for (vector<string>::const_iterator CI=input.begin();CI!=input.end();CI++)
	{
		res.append(QObject::tr(CI->c_str()));
	}
	return res;
}

bool Io_Qt::parse_config_file(string pfname, map<string,string>& input, vector<string>& ordered_key_vec)
{
	stringstream ss;
	Io_Qt::get_stringstream_from_QFile(pfname,ss);
	vector<string> vs = Io::read_file_by_lines(ss,true,'#',true);
	string key = "NONE";
	for (vector<string>::const_iterator CI=vs.begin();CI!=vs.end();CI++)
	{
		string line(*CI);
		ostringstream oss;
		if (line.substr(0,4).compare("KEY ")==0)
		{
			key = line.substr(4);
			ordered_key_vec.push_back(key);
		} else {
			ostringstream oss;
			map<string,string>::iterator it = input.find(key);
			if(it != input.end()) oss << (it->second).c_str() << std::endl;
			oss << line.c_str();
			input[key] = oss.str();
		}
	}
	return (vs.size() != 0);
}

bool Io_Qt::parse_config_file(string pfname, map<string,string>& input)
{
	vector<string> dummy;
	return Io_Qt::parse_config_file(pfname, input, dummy);
}

map<string,string> Io_Qt::parse_by_subkey(const map<string,string>& input, const string subkey)
{
	map<string,string> res;
	for (map<string,string>::const_iterator CI=input.begin();CI!=input.end();CI++)
	{
		string key(CI->first);
		string val(CI->second);
		istringstream iss(val);
		string line;
		while (getline(iss,line))
		{
			istringstream iss_internal(line);
			string first_word;
			iss_internal >> std::ws;
			iss_internal >> first_word;
			iss_internal >> std::ws; // Drops leading whitespace.
			string line_internal;
			getline(iss_internal,line_internal);
			if (first_word.compare(subkey)==0)
			{
				ostringstream oss;
				map<string,string>::iterator IT = res.find(key);
				if (IT!=res.end()) oss << (IT->second).c_str();
				oss << line_internal;
				res[key] = line_internal;
			}
		}
	}
	return res;
}
}
