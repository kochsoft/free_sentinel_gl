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

#include "form_about.h"

namespace display
{
void Dialog_about::set(QString title, QString content)
{
	this->setWindowTitle(title);	
	this->ui_dialog_about->textEdit_content->setText(content);
}

Dialog_about::Dialog_about(QString title, QString content, QWidget* parent)
	: QDialog(parent)
{
	this->ui_dialog_about = new Ui::Dialog_about();
	this->ui_dialog_about->setupUi(this);
	this->set(title,content);
	connect(this->ui_dialog_about->pushButton_close,SIGNAL(clicked(bool)),
		this,SLOT(onClick_close()));
}

Dialog_about::~Dialog_about()
{
	delete this->ui_dialog_about;
}

void Dialog_about::onClick_close()
{
	this->accept();
}
}