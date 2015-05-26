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

/**
 * Qt Extension of Io.
 * 
 * Markus-Hermann Koch, mhk@markuskoch.eu, 06.05.2015
 */

#ifndef MHK_IO_QT_H
#define MHK_IO_QT_H

#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <QObject>
#include <QFile>
#include <QStringList>
#include "io.h"

using std::stringstream;
using std::vector;
using std::map;
using std::cout;
using std::cerr;

namespace mhk_gl
{
// http://stackoverflow.com/questions/11807510/non-qt-base-classes
// Io_Qt must first inherit QObject or there will be trouble inheriting
// from the non-Qt class Io.
class Io_Qt : public QObject, public Io
{
Q_OBJECT

private:
	QWidget* parent;

public:
Io_Qt(QWidget* parent=0, E_DEBUG_LEVEL debug_level=E_DEBUG_LEVEL::MESSAGE,
	  ostream* stdout=&cout, ostream* stderr=&cerr);

/** Creates an istringstream from a QFile content.
 * QFile& qfile: Ref. to some QFile maybe containing qrc content cosntructed
 *   after Q_INIT_RESOURCE by something like 'QFile qfile(":/misc/gravity.txt")'
 *   You may open it using .open("QIODevice::ReadOnly"). If you don't this
 *   function will and will close it again after it is done.
 * stringstream& ss: Reference to the target stringstream. IMPORTANT:
 *   Needs to be fit for both reading and writing.
 *   Consider constructing it using something like
 *   stringstream ss(std::stringstream::in | std::stringstream::out |
 *     std::stringstream::binary);
 *   Using the standard constructor works, too though: stringstream ss;
 * Such a stringstream is a genuine istream and may be passed to several
 * of the input functions within Io.
 * @return qfile.isOpen().
 * Note: ss will contain "" if reading fails.
 * Note: This method will automatically open the QFile for reading if necessary.
 *   Only in that case it will close it agein after reading.
 */
static bool get_stringstream_from_QFile(QFile& qfile, stringstream& ss);
/** Convenience Shortcut allowing to insert resource strings like 
 * ':/misc/gravity.txt' directly.
 * @return qfile.exists();
 */
static bool get_stringstream_from_QFile(string pfname, stringstream& ss);

/** Tool function also using tr(..). E.g. helpful while filling comboboxes. */
static QStringList vector_string_to_QStringList(vector<string>&);

/** For parsing a game config file like ":/misc/gravity.txt".
 * Required format:
 *   1.) Leading whitespace in front of a line will be ignored.
 *   2.) If the rest of the line starts with '#' it will be ignored
 *       as well and be deemed to be a commentary line.
 *   3.) If the rest starts with "KEY " a new key for data will be
 *     added. E.g.: "KEY NAME" will add a key NAME.
 *   4.) After the KEY is activated any string content will be
 *     added as value using the KEY. If the KEY already existed
 *     the new string will be concatenated to it. Linebreaks
 *     will be respected.
 *   5.) At the beginning of the process the active key will be "NONE".
 * @param string pfname: Resource file name.
 * @param map<string,string>& input: Target data map. It will be
 *   added to by the data from the new config file.
 * @param vector<string>& ordered_key_vec: The order of parsing is lost
 *   during the transistion into the unordered map data.
 *   However, it will be kept within ordered_key_vec which mirrors the
 *   keys withing data.
 * @return true if and only if the read in data string read using
 *   Io::read_file_by_lines(ss,true,'#',true); has length!=0.
 *   In other words: false means no relevant text was found within the resource file.
 */
static bool parse_config_file(string pfname, map<string,string>& input, vector<string>& ordered_key_vec);
static bool parse_config_file(string pfname, map<string,string>& input);

/**
 * Returns a map which's keys are identical to the ones in input.
 * However the contents will be parsed thus that only those lines are
 * retained that begin with the subkey (case sensitive and a trailing whitespace).
 * This subkey will be dropped. Should there be several such lines for a
 * given main key they will be concatenated using std::endl.
 *   @param const map<string,string> input: A map like filled using parse_config_file(..)
 *   @param const string subkey: A subkey embedded within the value strings of input.
 * E.g.: an input map
 * "SOL" =>
 *   DESC A medium sized star.
 *   GRAVITY 27
 * "TERRA" =>
 *   DESC Earth is
 *   DESC mostly harmless.
 *   GRAVITY 1
 * Would have parse_by_subkey(input, "DESC") have return a new map
 * "SOL" =>
 *   A medium sized star.
 * "TERRA" =>
 *   Earth is
 *   mostly harmless.
 */
static map<string,string> parse_by_subkey(const map<string,string>& input, const string subkey);
};
}

#endif
