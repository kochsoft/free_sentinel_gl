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
 * Short user interface class for the sole purpose of displaying help texts.
 * 
 * Markus-Hermann Koch, 25.05.2015, mhk@markuskoch.eu
 */

#ifndef MHK_FORM_ABOUT_H
#define MHK_FORM_ABOUT_H

#include <QDialog>
#include <QString>
#include "ui_dialog_about.h"

namespace display
{

class Dialog_about : public QDialog
{
Q_OBJECT

private slots:
	void onClick_close();

private:
	Ui::Dialog_about* ui_dialog_about;

public:
	void set(QString title, QString content);
	
	Dialog_about(QString title, QString content, QWidget* parent=0);
	~Dialog_about();
};

}

#endif
