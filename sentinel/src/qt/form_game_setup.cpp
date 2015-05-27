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
#include <iostream>
#include <string>
#include <cmath>
#include <sstream>
#include <QMessageBox>
#include "form_game_setup.h"
#include "io_qt.h"

using std::cout;
using std::endl;
using std::string;
using std::getline;
using std::stringstream;
using std::istringstream;
using std::ostringstream;

using namespace mhk_gl;

namespace display
{
//> Setup_game_data. -------------------------------------------------
Setup_game_data::Setup_game_data(Dialog_setup_game* src)
{
	this->src = src;
	if (src) doSnapshot();
}

Setup_game_data::Setup_game_data(const Setup_game_data& orig)
{
	this->lineEdit_campaign = orig.lineEdit_campaign;
	this->horizontalSlider_challenge = orig.horizontalSlider_challenge;
	this->spinBox_sentries = orig.spinBox_sentries;
	this->combobox_gravity = orig.combobox_gravity;
	this->combobox_age = orig.combobox_age;
	this->combobox_rotation_type = orig.combobox_rotation_type;
	this->spinBox_psi_shield = orig.spinBox_psi_shield;
	this->spinBox_confidence = orig.spinBox_confidence;
	this->spinBox_spin_period = orig.spinBox_spin_period;
	this->checkBox_meanies = orig.checkBox_meanies;
	this->checkBox_random_scenery = orig.checkBox_random_scenery;
	this->spinBox_rows = orig.spinBox_rows;
	this->spinBox_cols = orig.spinBox_cols;
	this->spinBox_self_spin = orig.spinBox_self_spin;
	this->spinBox_energy = orig.spinBox_energy;
	this->spinBox_object_resilience = orig.spinBox_object_resilience;
	this->spinBox_antagonist_fov = orig.spinBox_antagonist_fov;
	this->src = orig.src;
}

void Setup_game_data::doSnapshot()
{
	if (src==0) throw "Snapshot of non-existing setup form attempted.";
	Ui::Dialog_setup_game* ui = src->get_ui();
	lineEdit_campaign = ui->lineEdit_campaign->text();
	horizontalSlider_challenge = ui->horizontalSlider_challenge->value();
	spinBox_sentries = ui->spinBox_sentries->value();
	combobox_gravity = ui->comboBox_gravity->currentIndex();
	combobox_age = ui->comboBox_age->currentIndex();
	combobox_rotation_type = ui->comboBox_rotation_type->currentIndex();
	spinBox_psi_shield = ui->spinBox_psi_shield->value();
	spinBox_confidence = ui->spinBox_confidence->value();
	spinBox_spin_period = ui->spinBox_spin_period->value();
	checkBox_random_scenery = ui->checkBox_random_scenery->isChecked();
	checkBox_meanies = ui->checkBox_meanies->isChecked();
	spinBox_rows = ui->spinBox_rows->value();
	spinBox_cols = ui->spinBox_cols->value();
	spinBox_self_spin = ui->spinBox_self_spin->value();
	spinBox_energy = ui->spinBox_energy->value();
	spinBox_object_resilience = ui->spinBox_object_resilience->value();
	spinBox_antagonist_fov = ui->spinBox_antagonist_fov->value();
}

void Setup_game_data::write_onto_src()
{
	if (src==0) return; // Nothing to do.
	Ui::Dialog_setup_game* ui = src->get_ui();
	ui->lineEdit_campaign->setText(lineEdit_campaign);
	ui->horizontalSlider_challenge->setValue(horizontalSlider_challenge);
	ui->spinBox_sentries->setValue(spinBox_sentries);
	ui->comboBox_gravity->setCurrentIndex(combobox_gravity);
	ui->comboBox_age->setCurrentIndex(combobox_age);
	ui->comboBox_rotation_type->setCurrentIndex(combobox_rotation_type);
	ui->spinBox_psi_shield->setValue(spinBox_psi_shield);
	ui->spinBox_confidence->setValue(spinBox_confidence);
	ui->spinBox_spin_period->setValue(spinBox_spin_period);
	ui->checkBox_random_scenery->setChecked(checkBox_random_scenery);
	ui->checkBox_meanies->setChecked(checkBox_meanies);
	ui->spinBox_rows->setValue(spinBox_rows);
	ui->spinBox_cols->setValue(spinBox_cols);
	ui->spinBox_self_spin->setValue(spinBox_self_spin);
	ui->spinBox_energy->setValue(spinBox_energy);
	ui->spinBox_object_resilience->setValue(spinBox_object_resilience);
	ui->spinBox_antagonist_fov->setValue(spinBox_antagonist_fov);
}
//< ------------------------------------------------------------------

Dialog_setup_game::Dialog_setup_game(vector<string>& planets, QWidget* parent)
	: QDialog(parent)
{
	this->campaign_seed = 0;
	ui_dialog_setup_game = new Ui::Dialog_setup_game();
	ui_dialog_setup_game->setupUi(this);
	populate_combobox_rotation_type();
	populate_combobox_gravity(planets);
	populate_combobox_age();
	default_setup_game_data = new Setup_game_data(this);
	last_close_setup_game_data = new Setup_game_data(this);
	
	connect(ui_dialog_setup_game->pushButton_ok_campaign,SIGNAL(clicked()),this,SLOT(onClick_campaign()));
	connect(ui_dialog_setup_game->pushButton_ok_challenge,SIGNAL(clicked()),this,SLOT(onClick_challenge()));
	connect(ui_dialog_setup_game->pushButton_ok_custom,SIGNAL(clicked()),this,SLOT(onClick_custom()));
	connect(ui_dialog_setup_game->pushButton_cancel,SIGNAL(clicked()),this,SLOT(onClick_cancel()));
	connect(ui_dialog_setup_game->pushButton_close,SIGNAL(clicked()),this,SLOT(onClick_close()));
	connect(ui_dialog_setup_game->pushButton_restore_defaults,SIGNAL(clicked()),this,SLOT(onClick_restore_defaults()));
	stdout = &(std::cout);
	stderr = &(std::cerr);
}

Dialog_setup_game::~Dialog_setup_game()
{
	delete ui_dialog_setup_game;
	delete default_setup_game_data;
}

void Dialog_setup_game::onClick_campaign()
{
	QString text = get_ui()->lineEdit_campaign->text();
	istringstream iss(text.toStdString());
	uint code;
	iss >> code;
	if (setup_campaign_level(code))
	{
		this->accept();
		last_close_setup_game_data->doSnapshot();
		start_campaign();
	} else {
		ostringstream oss;
		oss << QObject::tr("Given campaign code '").toStdString().c_str() <<
			code <<
			QObject::tr("' is invalid.").toStdString().c_str();
		QString text = oss.str().c_str();
		QMessageBox::about(this, QObject::tr("Invalid campaign code."), text);
	}
}

void Dialog_setup_game::onClick_challenge()
{
	time_t timer;
	time(&timer);
	create_challenge(
		(uint)timer,
		ui_dialog_setup_game->horizontalSlider_challenge->value()
	);
	last_close_setup_game_data->doSnapshot();
	this->accept();
	start_challenge();
}

void Dialog_setup_game::onClick_custom()
{
	last_close_setup_game_data->doSnapshot();
	this->accept();
	start_custom();
}

void Dialog_setup_game::onClick_cancel()
{
	last_close_setup_game_data->write_onto_src();
	this->reject();
}

void Dialog_setup_game::onClick_close()
{
	last_close_setup_game_data->doSnapshot();
	this->reject();
}

void Dialog_setup_game::onClick_restore_defaults()
{
	default_setup_game_data->write_onto_src();
}

void Dialog_setup_game::populate_combobox_gravity(vector<string> names)
{
	for (vector<string>::const_iterator CI=names.begin();CI!=names.end();CI++)
	{
		ui_dialog_setup_game->comboBox_gravity->addItem(tr(CI->c_str()));
	}
	if (names.size()>1) ui_dialog_setup_game->comboBox_gravity->setCurrentIndex(1);
}

void Dialog_setup_game::populate_combobox_rotation_type()
{
	QStringList list;
	list.push_back(tr("To the left"));
	list.push_back(tr("To the right"));
	list.push_back(tr("Randomized"));
	ui_dialog_setup_game->comboBox_rotation_type->addItems(list);
	ui_dialog_setup_game->comboBox_rotation_type->setCurrentIndex(2);
}

uint Dialog_setup_game::get_n_gravity_from_form()
{
	return (uint)(ui_dialog_setup_game->comboBox_gravity->currentIndex());
}

uint Dialog_setup_game::get_n_age_from_form()
{
	uint cnt = (uint)(ui_dialog_setup_game->comboBox_age->count());
	if (cnt==0) throw "Combobx age not populated.";
	uint current = (uint)(ui_dialog_setup_game->comboBox_age->currentIndex());
	return cnt - current - ((uint)1);
}

uint Dialog_setup_game::get_n_sentries_from_form()
{
	return (uint)(ui_dialog_setup_game->spinBox_sentries->value());
}

uint Dialog_setup_game::get_n_energy_from_form()
{
	uint max = (uint)(ui_dialog_setup_game->spinBox_energy->maximum());
	//uint min = (uint)(ui_dialog_setup_game->spinBox_energy->minimum());
	uint val = (uint)(ui_dialog_setup_game->spinBox_energy->value());
	return max-val;
}

uint Dialog_setup_game::get_max_n_gravity_from_form()
{
	return ui_dialog_setup_game->comboBox_age->count();
}

uint Dialog_setup_game::get_max_n_age_from_form()
{
	return ui_dialog_setup_game->comboBox_age->count();
}

uint Dialog_setup_game::get_max_n_sentries_from_form()
{
	return ui_dialog_setup_game->spinBox_sentries->maximum() -
		ui_dialog_setup_game->spinBox_sentries->minimum();
}

uint Dialog_setup_game::get_max_n_energy_from_form()
{
	return ui_dialog_setup_game->spinBox_energy->maximum() -
		ui_dialog_setup_game->spinBox_energy->minimum();
}

void Dialog_setup_game::populate_combobox_age()
{
	QStringList list;
	list.push_back(tr("Ancient"));
	list.push_back(tr("Aged"));
	list.push_back(tr("Intermediate"));
	list.push_back(tr("Young"));
	ui_dialog_setup_game->comboBox_age->addItems(list);
	ui_dialog_setup_game->comboBox_age->setCurrentIndex(2);
}

void Dialog_setup_game::put_values_into_form(uint n_gravity, uint n_age,
		uint n_sentries, uint n_energy)
{
	// In reality for energy _low_ values increase the level of difficulty. Hence ...
	n_energy = ui_dialog_setup_game->spinBox_energy->maximum() - n_energy;
	// In reality for age _low_ values increase the level of difficulty. Hence ...
	n_age = ui_dialog_setup_game->comboBox_age->count() - n_age - (uint)1;

	ui_dialog_setup_game->comboBox_gravity->setCurrentIndex(n_gravity);
	ui_dialog_setup_game->comboBox_age->setCurrentIndex(n_age);
	ui_dialog_setup_game->spinBox_sentries->setValue(n_sentries);
	ui_dialog_setup_game->spinBox_energy->setValue(n_energy);
	last_close_setup_game_data->doSnapshot();
}

uint Dialog_setup_game::generate_campaign_code(uint n_gravity, uint n_age,
	uint n_sentries, uint n_energy, uint offset, bool& campaign_won)
{
	uint max_offset = DEFAULT_CAMPAIGN_MAX_OFFSET;
	campaign_won = false;
	//> Handling offset overflow. ------------------------------------
	uint delta = 0;
	if (offset > max_offset)
	{
		delta = offset / max_offset;
		offset = offset % max_offset;
	}
	uint max_n_gravity = get_max_n_gravity_from_form();
	uint max_n_age = get_max_n_age_from_form();
	uint max_n_sentries = get_max_n_sentries_from_form();
	uint max_n_energy = get_max_n_energy_from_form();
	while (delta > 0)
	{
		delta--;
		n_energy++;
		if (n_energy > max_n_energy)
		{
			n_energy = 0;
			n_sentries++;
			if (n_sentries > max_n_sentries)
			{
				n_sentries = 0;
				n_age++;
				if (n_age > max_n_age)
				{
					n_age = 0;
					n_gravity++;
					if (n_gravity > max_n_gravity)
					{
						campaign_won = true;
						n_gravity = max_n_gravity;
					}
				}
			}
		}
	}
	//< --------------------------------------------------------------
	//> Generating the actual code. ----------------------------------
	uint code = n_energy + 16*n_sentries + 256*n_age + 4096*n_gravity;
	if (sizeof(uint) > 2) { code += 65536*offset; } // 2^16==65536.
	//< --------------------------------------------------------------
	return code;
}

bool Dialog_setup_game::parse_campaign_code(uint code, uint& n_gravity,
	uint& n_age, uint& n_sentries, uint& n_energy, uint& offset)
{
	uint max_offset = DEFAULT_CAMPAIGN_MAX_OFFSET;
	//> Step 1: Parsing the code. ------------------------------------
	n_energy   = (code & 0x000f);
	n_sentries = (code & 0x00f0) / 16;
	n_age      = (code & 0x0f00) / 256;
	n_gravity  = (code & 0xf000) / 4096;
	offset     = code / 65536;
	//< --------------------------------------------------------------
	//> Step 2: Checking validity. -----------------------------------
	uint max_gravity = get_max_n_gravity_from_form();
	uint max_age = get_max_n_age_from_form();
	uint max_sentries = get_max_n_sentries_from_form();
	uint max_energy = get_max_n_energy_from_form();
	return (
		offset <= max_offset &&
		n_energy <= max_energy &&
		n_sentries <= max_sentries &&
		n_age <= max_age &&
		n_gravity <= max_gravity
	);
	//< --------------------------------------------------------------
}

void Dialog_setup_game::harmonize_values(uint& n_gravity,
	uint& n_age, uint& n_sentries, uint& n_energy)
{
	// TODO: Optimize these criteria some time.
	if (n_gravity > 3) { n_age = qMin(n_age, (uint)2); }
	if (n_gravity > 3) { n_energy = qMin(n_energy, (uint)10); }
	if (n_sentries > 3) { n_age = qMin(n_age, (uint)2); }
	if (n_sentries > 3) { n_energy = qMin(n_energy, (uint)10); }
}

bool Dialog_setup_game::get_next_campaign_code(uint offset, uint& new_code)
{
	uint n_gravity = get_n_gravity_from_form();
	uint n_age = get_n_age_from_form();
	uint n_sentries = get_n_sentries_from_form();
	uint n_energy = get_n_energy_from_form();
	bool campaign_won = false;
	new_code = generate_campaign_code(n_gravity, n_age, n_sentries, n_energy,
		offset, campaign_won);
	return campaign_won;
}

int Dialog_setup_game::get_level_number(uint code)
{
	uint max_offset = DEFAULT_CAMPAIGN_MAX_OFFSET;
	uint n_gravity;
	uint n_age;
	uint n_sentries;
	uint n_energy;
	uint offset;
	bool valid = parse_campaign_code(code, n_gravity, n_age,
		n_sentries, n_energy, offset);
	if (!valid) return -1;
	uint max_age = get_max_n_age_from_form();
	uint max_sentries = get_max_n_sentries_from_form();
	uint max_energy = get_max_n_energy_from_form();
	return
		(n_gravity * max_age * max_sentries * max_energy +
		n_age * max_sentries * max_energy +
		n_sentries * max_energy +
		n_energy)*max_offset + offset;
}

bool Dialog_setup_game::setup_campaign_level(uint code)
{
	uint n_gravity = 0;
	uint n_age = 0;
	uint n_sentries = 0;
	uint n_energy = 0;
	uint offset = 0;
	bool valid = parse_campaign_code(code, n_gravity, n_age, n_sentries, n_energy, offset);
	if (valid)
	{
		harmonize_values(n_gravity, n_age, n_sentries, n_energy);
		put_values_into_form(n_gravity, n_age, n_sentries, n_energy);
		this->campaign_seed = offset + get_level_number(code);
	}
	return valid;
}

float Dialog_setup_game::sentry_value(uint n)
{
	// Maximum 1800 for 5 sentries.
	return 360.*((float)n);
}

float Dialog_setup_game::gravity_value(uint n)
{
	// Maximum 1500 for 5.
	return 300.*((float)n);
}

float Dialog_setup_game::age_value(uint n)
{
	// Maximum 800 for 4.
	return 200.*((float)n);
}

float Dialog_setup_game::energy_value(uint n)
{
	// Maximum 3600 for 15.
	return 240.*((float)n);
}

float Dialog_setup_game::get_value(uint n_gravity,
	uint n_age, uint n_sentries, uint n_energy)
{
	return
		gravity_value(n_gravity) +
		age_value(n_age) +
		sentry_value(n_sentries) +
		energy_value(n_energy);
}

float Dialog_setup_game::get_maximum_value()
{
	return get_value(
		get_max_n_gravity_from_form(),
		get_max_n_age_from_form(),
		get_max_n_sentries_from_form(),
		get_max_n_energy_from_form());
}

void Dialog_setup_game::create_challenge(uint seed, int raw_level)
{
	int slider_max = ui_dialog_setup_game->horizontalSlider_challenge->maximum();
	raw_level %= (slider_max+1);
	// Pull level in the attainable range.
	float level = get_maximum_value()*((float)raw_level)/((float)slider_max);
	qsrand(seed);
	uint n_sentries = 0;
	uint n_gravity = 0;
	uint n_age = 0;
	uint n_energy = 0;
	bool n_sentries_maxed_out = false;
	bool n_gravity_maxed_out = false;
	bool n_age_maxed_out = false;
	bool n_energy_maxed_out = false;
	while (!(n_sentries_maxed_out && n_gravity_maxed_out && n_age_maxed_out && n_energy_maxed_out))
	{
		int k = qrand()%6;
		switch (k)
		{
			case 0:
				if (!n_sentries_maxed_out)
				{
					if ((++n_sentries) >= get_max_n_sentries_from_form())
						n_sentries_maxed_out = true;
				}
				break;
			case 1: 
				if (!n_gravity_maxed_out)
				{
					if ((++n_gravity) >= get_max_n_gravity_from_form())
						n_gravity_maxed_out = true;
				}
				break;
			case 2:
				if (!n_age_maxed_out)
				{
					if ((++n_age) >= get_max_n_age_from_form())
						n_age_maxed_out = true;
				}
				break;
			default:
				if (!n_energy_maxed_out)
				{
					if ((++n_energy) >= get_max_n_energy_from_form())
						n_energy_maxed_out = true;
				}
				break;
		}
		if (get_value(n_gravity,n_age,n_sentries,n_energy) >= level) break;
	}
	
	harmonize_values(
		n_gravity,
		n_age,
		n_sentries,
		n_energy
	);
	//> Write the values into the form. ------------------------------
	put_values_into_form(
		n_gravity,
		n_age,
		n_sentries,
		n_energy
	);
	//< --------------------------------------------------------------
}
}
