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
#include <sstream>
#include <QMessageBox>
#include <QPainter>
#include <qpaintengine.h>
#include <sstream>

#include "mhk_cmake_config.h"
#include "ui_main.h"
#include "form_main.h"
#include "form_about.h"

using std::time_t;
using std::endl;
using std::ostringstream;
using std::istringstream;

namespace display
{
void Form_main::setup_menu()
{
	connect(uiMainWindow->action_new_game, SIGNAL(triggered()), this, SLOT(new_game()));
	connect(uiMainWindow->action_restart, SIGNAL(triggered()), this, SLOT(restart_game()));
	connect(uiMainWindow->action_similar, SIGNAL(triggered()), this, SLOT(similar_game()));
	connect(uiMainWindow->action_exit, SIGNAL(triggered()), this, SLOT(exit_program()));
	connect(uiMainWindow->actionRules, SIGNAL(triggered()), this, SLOT(about_rules()));
	connect(uiMainWindow->actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
	connect(uiMainWindow->actionAbout_Sentinel, SIGNAL(triggered()), this, SLOT(about_sentinel()));
	connect(uiMainWindow->actionAbout_mhk, SIGNAL(triggered()), this, SLOT(about_mhk()));
	connect(uiMainWindow->actionAbout_remake, SIGNAL(triggered()), this, SLOT(about_remake()));
	connect(uiMainWindow->actionGPL, SIGNAL(triggered()), this, SLOT(about_gpl()));
}

void Form_main::setup_dialog_setup()
{
	// TODO: Move to separate function!
	map<string,string> planets = Io_Qt::parse_by_subkey(planetary_data, "NAME");
	vector<string> planet_vec;
	for (vector<string>::const_iterator CI=planetary_order.begin();CI!=planetary_order.end();CI++)
		{ planet_vec.push_back(planets[*CI]); }
	
	this->dialog_setup_game = new Dialog_setup_game(planet_vec, this);

	connect(dialog_setup_game, SIGNAL(start_campaign()),this,SLOT(ok_setup_campaign()));
	connect(dialog_setup_game, SIGNAL(start_challenge()),this,SLOT(ok_setup_challenge()));
	connect(dialog_setup_game, SIGNAL(start_custom()),this,SLOT(ok_setup_custom()));
}


// http://www.informit.com/articles/article.aspx?p=1405225&seqNum=3
// http://stackoverflow.com/questions/11986280/add-icon-to-status-bar-qt
void Form_main::setup_statusBar()
{
	label_statusBar = new QLabel(tr("Welcome."));
	label_statusBar_energy = new QLabel();
	//label_statusBar_energy->setAlignment(Qt::AlignCenter);
	uiMainWindow->statusbar->addWidget(label_statusBar);
	uiMainWindow->statusbar->addWidget(label_statusBar_energy);

	pixmap_robot_icon = new QPixmap(":/graphics/robot_icon.png");
	pixmap_block_icon = new QPixmap(":/graphics/block_icon.png");
	pixmap_tree_icon = new QPixmap(":/graphics/tree_icon.png");
	pixmap_big_icon = new QPixmap(":/graphics/big_icon.png");
	pixmap_energy = 0;
}

// https://blog.qt.io/blog/2014/09/10/qt-weekly-19-qopenglwidget/
void Form_main::setup_Widget_OpenGl()
{
	uiMainWindow->openGLWidget->set_io(&this->io);

	connect(uiMainWindow->openGLWidget,SIGNAL(update_parent_statusBar_text(QString)),
		this,SLOT(update_statusBar_text(QString)));
	connect(uiMainWindow->openGLWidget,SIGNAL(disable_program(QString)),
		this,SLOT(disable_program(QString)));
	connect(uiMainWindow->openGLWidget,SIGNAL(exit_requested()),
		this,SLOT(exit_program()));
	connect(uiMainWindow->openGLWidget,SIGNAL(fullscreen_key_pressed()),
		this,SLOT(toggle_fullscreen()));
	connect(uiMainWindow->openGLWidget,SIGNAL(request_new_game()),
		this,SLOT(request_new_game()));
	connect(uiMainWindow->openGLWidget,SIGNAL(request_restart_game()),
		this,SLOT(restart_game()));
}

void Form_main::setup_form()
{
	setWindowIcon(QIcon(":graphics/eye.png"));
	uiMainWindow = new Ui::MainWindow();
	uiMainWindow->setupUi(this);
	this->known_sounds = new Known_Sounds;
	
	this->setWindowTitle(QString(get_program_name()));
	Io_Qt::parse_config_file(":/misc/gravity.txt", planetary_data, planetary_order);
	
	this->setup_menu();
	this->setup_dialog_setup();
	this->setup_statusBar();
	this->setup_Widget_OpenGl();
}

Game* Form_main::new_game_object(E_GAME_TYPE type, uint seed, Setup_game_data* game_data)
{
	// Note that get_mesh_data_* requires initializeOpenGL to have run!
	if (!game_data) game_data = dialog_setup_game->get_game_data();
	if (!game_data) throw "No non-0 game data available.";
	if (!game_data->is_valid()) throw "Invalid game data was passed!";
	E_SCENERY scenery = get_scenery_by_selection(
		game_data->checkBox_random_scenery ? -1 : game_data->combobox_gravity);
	uiMainWindow->openGLWidget->set_scenery(scenery);
	uiMainWindow->openGLWidget->set_scenery_light(get_light_color_by_scenery(scenery));
	Game* game = new Game(
		type,
		seed,
		game_data,
		uiMainWindow->openGLWidget,
		&(this->io),
		known_sounds,
		uiMainWindow->openGLWidget->get_framerate(),
		uiMainWindow->openGLWidget->get_mesh_data_connection(),
		uiMainWindow->openGLWidget->get_mesh_data_odd(),
		uiMainWindow->openGLWidget->get_mesh_data_even(),
		uiMainWindow->openGLWidget->get_mesh_data_sentinel(),
		uiMainWindow->openGLWidget->get_mesh_data_sentinel_tower(),
		uiMainWindow->openGLWidget->get_mesh_data_sentry(),
		uiMainWindow->openGLWidget->get_mesh_data_tree(),
		uiMainWindow->openGLWidget->get_mesh_data_robot(),
		uiMainWindow->openGLWidget->get_mesh_data_block(),
		uiMainWindow->openGLWidget->get_mesh_data_meanie()
	);
	// Randomize timer after map generation was completed.
	qsrand(get_timestamp());

	update_statusBar_energy(game->get_player()->get_energy_units());
	connect(game->get_player()->get_viewer_data(),SIGNAL(viewer_data_changed()),uiMainWindow->openGLWidget,SLOT(request_paintGL()));
	connect(game,SIGNAL(update_statusBar_text(QString)),this,SLOT(update_statusBar_text(QString)));
	connect(game,SIGNAL(update_statusBar_energy(int)),this,SLOT(update_statusBar_energy(int)));
	connect(game,SIGNAL(set_light_filtering_factor(QVector4D,int)),
		uiMainWindow->openGLWidget,SLOT(set_light_filtering_factor(QVector4D,int)));
	connect(game,SIGNAL(request_paintGL()),
		uiMainWindow->openGLWidget,SLOT(request_paintGL()));
	connect(game, SIGNAL(update_campaign_code(int)),this,SLOT(update_campaign_code(int)));
	
	uiMainWindow->openGLWidget->set_light_filtering_factor(QVector4D(1,1,1,1),2);
	this->active_game_data = *game_data;
	{
		ostringstream oss;
		uint code;
		dialog_setup_game->get_next_campaign_code(0,code);
		oss << "New game object created. Level code for these settings: " << code <<
			" (level " << dialog_setup_game->get_level_number(code) << ").";
		io.println(E_DEBUG_LEVEL::VERBOSE,"new_game_object(..)",oss.str());
	}
	return game;
}

Form_main::Form_main(QWidget* parent, E_DEBUG_LEVEL debug_level,
	ostream* stdout, ostream* stderr) : QMainWindow(parent),
		io(parent, debug_level, stdout, stderr)
{
	if (debug_level==E_DEBUG_LEVEL::VERBOSE)
	{
		io.println(debug_level,"Form_main(..)", "Using VERBOSE message mode.");
	}
	setup_form();
	dialog_about = new Dialog_about(tr("About ..."), "", this);
#if defined(Q_OS_SYMBIAN)
  this->showMaximized();
#else
  this->show();
#endif
	uint seed = get_timestamp();
	uiMainWindow->openGLWidget->set_game(new_game_object(E_GAME_TYPE::CHALLENGE,seed));
	//uiMainWindow->openGLWidget->set_game(new_game_object(E_GAME_TYPE::CAMPAIGN,(uint)0));
	// Needs to be here because the appropriate signal within Game is not
	// yet connected during Game construction.
	update_statusBar_text(uiMainWindow->openGLWidget->get_game()->get_game_status_string());
}

Form_main::~Form_main()
{
	delete uiMainWindow;
	delete dialog_setup_game;
	delete dialog_about;
	delete label_statusBar;
	delete label_statusBar_energy;
	delete pixmap_robot_icon;
	delete pixmap_block_icon;
	delete pixmap_tree_icon;
	delete pixmap_big_icon;
	delete pixmap_energy;
	delete known_sounds;
}

// http://stackoverflow.com/questions/25454648/qmainwindow-close-signal-not-emitted
void Form_main::closeEvent(QCloseEvent* event)
{
	// event->ignore(); // <-- this actually is an option to hinder the window from closing!
	std::cout << "Thanks for playing " << get_program_name().toStdString().c_str() << " !" << endl;
	event->accept();
}

void Form_main::new_game()
{
	this->dialog_setup_game->show();
}

void Form_main::exit_program()
{
	this->close();
}

void Form_main::toggle_fullscreen()
{
	if (isFullScreen())
	{
		showNormal();
	} else {
		showFullScreen();
	}
}

uint Form_main::get_timestamp()
{
	time_t timer;
	time(&timer);
	return (uint)timer;
}

E_SCENERY Form_main::get_scenery_by_selection(int index)
{
	string sc;
	Known_Sceneries scs;
	E_SCENERY res = E_SCENERY::EUROPE;
	if (index < 0)
	{
		index = qrand() % Known_Sceneries::number_of_known_sceneries();
		res = static_cast<E_SCENERY>(index);
		ostringstream oss;
		oss << "rnd code " << res;
		sc = oss.str();
	} else {
		string key = planetary_order.at(index);
		map<string,string> sceneries = Io_Qt::parse_by_subkey(planetary_data, "SCENERY");
		sc = sceneries.at(key);
		if (sc.compare("MASTER")==0) res = E_SCENERY::MASTER;
		else if (sc.compare("EUROPE")==0) res = E_SCENERY::EUROPE;
		else if (sc.compare("SELENE")==0) res = E_SCENERY::SELENE;
		else if (sc.compare("MARS")==0) res = E_SCENERY::MARS;
		else if (sc.compare("ASTEROID")==0) res = E_SCENERY::ASTEROID;
	}
	if (!scs.is_supported(res))
	{
		io.println(E_DEBUG_LEVEL::WARNING, "Form_main::get_scenery_by_selection(int index)",
			"Given scenery code '"+sc+"' is unsupported so far. Reverting to EUROPE.");
		res = E_SCENERY::EUROPE;
	}
	return res;
}

QVector4D Form_main::get_light_color_by_scenery(E_SCENERY scenery)
{
	string key = Known_Sceneries::toString(scenery);
	map<string,string> light_strs = Io_Qt::parse_by_subkey(planetary_data, "LIGHT");
	istringstream iss(light_strs.at(key));
	float r, g, b, a;
	iss >> r; iss >> g; iss >> b; iss >> a;
	return QVector4D(r,g,b,a);
}

void Form_main::plugin_new_game(Game* game)
{
	// The order of commands is important here.
	Game* old_game = uiMainWindow->openGLWidget->get_game();
	uiMainWindow->openGLWidget->set_game(0); // "Turns off" the old game.
	delete old_game; // Safely deletes it.
	uiMainWindow->openGLWidget->set_game(game); // Sets the new game.
	// Needs to be here because the appropriate signal within Game is not
	// yet connected during Game construction.
	update_statusBar_text(game->get_game_status_string());
}

void Form_main::restart_game(uint seed)
{
	Game* old_game = uiMainWindow->openGLWidget->get_game();
	if (!old_game) return; // No old game to restart.
	seed = seed ? seed : old_game->get_landscape()->get_seed();
	E_GAME_TYPE type = old_game->get_game_type();
	Game* new_game = new_game_object(
		type,
		seed,
		this->active_game_data.is_valid() ? &(this->active_game_data) : 0
	);
	// Note that plugin_new_game() will safely delete old_game.
	plugin_new_game(new_game);
}

void Form_main::similar_game()
{
	Game* old_game = uiMainWindow->openGLWidget->get_game();
	if (old_game)
	{
		// In case of a CAMPAIGN a similar level should be equivalent to restarting.
		uint seed = (old_game->get_game_type() == E_GAME_TYPE::CAMPAIGN) ? 
			old_game->get_landscape()->get_seed() : get_timestamp();
		restart_game(seed);
	}
}

void Form_main::ok_setup_campaign()
{
	plugin_new_game(new_game_object(E_GAME_TYPE::CAMPAIGN, dialog_setup_game->get_campaign_seed()));
}

void Form_main::ok_setup_challenge()
{
	plugin_new_game(new_game_object(E_GAME_TYPE::CHALLENGE, get_timestamp()));
}

void Form_main::ok_setup_custom()
{
	plugin_new_game(new_game_object(E_GAME_TYPE::CUSTOM, get_timestamp()));
}

void Form_main::request_new_game()
{
	Game* game = this->uiMainWindow->openGLWidget->get_game();
	E_GAME_TYPE type = game ? game->get_game_type() : E_GAME_TYPE::CHALLENGE;
	switch(type)
	{
		case E_GAME_TYPE::CAMPAIGN: ok_setup_campaign(); break;
		case E_GAME_TYPE::CHALLENGE: ok_setup_challenge(); break;
		case E_GAME_TYPE::CUSTOM: ok_setup_custom(); break;
		default: throw "Unknown typ encountered.";
	}
}

void Form_main::update_statusBar_text(QString text)
{
	this->label_statusBar->setText(tr(text.toStdString().c_str()));
}

void Form_main::update_statusBar_energy(int energy)
{
	//> Step 1: Piece together what icons are needed. ----------------
	if (energy < 0) energy = 0;
	int trees = 0;
	int blocks = 0;
	int robots = 0;
	int big = 0;
	if (energy % 3 == 1) { energy--; trees = 1; }
	else if (energy % 3 == 2) { energy-=2; blocks = 1; }
	robots = energy / 3;
	big = robots / 5;
	robots = robots % 5;
	//< --------------------------------------------------------------
	//> Step 2: Actually assign the new graphic. ---------------------
	QSize size_icon = pixmap_robot_icon->size();
	
	int space = 6;
	int width_icon = size_icon.width()+space;
	int width = width_icon*(trees+blocks+robots+big)+space;
	QPixmap* pm = new QPixmap(
		width,
		size_icon.height()
	);
	
	QPainter painter(pm);
	painter.fillRect(QRect(0,0,width,size_icon.height()), QColor(127,127,127,255));
	int pos=0;
	for (int j=0;j<big;j++) painter.drawPixmap(space+(pos++)*width_icon,0,*pixmap_big_icon);
	for (int j=0;j<robots;j++) painter.drawPixmap(space+(pos++)*width_icon,0,*pixmap_robot_icon);
	for (int j=0;j<blocks;j++) painter.drawPixmap(space+(pos++)*width_icon,0,*pixmap_block_icon);
	for (int j=0;j<trees;j++) painter.drawPixmap(space+(pos++)*width_icon,0,*pixmap_tree_icon);
	
	label_statusBar_energy->setPixmap(*pm);
	delete pixmap_energy;
	pixmap_energy = pm;
	//< --------------------------------------------------------------
}

void Form_main::update_campaign_code(int energy)
{
	uint code;
	bool won_campaign = dialog_setup_game->get_next_campaign_code(energy,code);
	if (won_campaign)
	{
			update_statusBar_text("Congratulations! You have won the campaign.");
			code = 0;
			dialog_setup_game->get_ui()->lineEdit_campaign->setText("0");
			dialog_setup_game->setup_campaign_level(code);			
	} else {
		uint level = dialog_setup_game->get_level_number(code);
		{
			ostringstream oss;
			oss << code;
			dialog_setup_game->get_ui()->lineEdit_campaign->setText(QString(oss.str().c_str()));
			dialog_setup_game->setup_campaign_level(code);
		}
		{
			ostringstream oss;
			oss << QObject::tr("Advanced ").toStdString() <<
				energy <<
				QObject::tr(" levels to level ").toStdString() <<
				level << ". " <<
				QObject::tr("New code: ").toStdString() <<
				code << ".";
			update_statusBar_text(oss.str().c_str());
		}
	}
}

void Form_main::disable_program(QString msg)
{
	this->uiMainWindow->action_new_game->setEnabled(false);
	// TODO: Having tooltips for menu items is not as simple as that.
	this->uiMainWindow->action_new_game->setToolTip(tr(
		"Unable to start game. See status bar and console error messages for details."));
	this->uiMainWindow->action_opengl_setup->setEnabled(false);
	this->label_statusBar->setText(msg);
	this->label_statusBar->setStyleSheet("QLabel { font-weight: bold; background-color : black; color : red; }");
}

QString Form_main::get_program_name() const
{
	ostringstream oss;
	oss << MHK_PROJECT_NAME " V" << MHK_VERSION_MAJOR << "." << MHK_VERSION_MINOR;
	return QString(oss.str().c_str());
}

void Form_main::about(QString fname, QString title, bool use_message_box)
{
	QFile qf(fname);
	QString text;
	if (!qf.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		text = " is an openGL >= 2.0, Qt 5.4 C++ remake of the "
			"<i>Firebird</i> classic <i>the Sentinel</i> originally released "
			"for the Commodore 64 back in 1986.</p><p>"
			"Markus-Hermann Koch, mhk@markuskoch.eu, "
				"29.04.2015-25.05.2015</p>";
		text.prepend(get_program_name());
		text.prepend("<p>");
	} else {
		QTextStream in(&qf);
		text = in.readAll();
	}
	if (use_message_box)
	{
		QMessageBox::about(this, title, text);
	}else{
		dialog_about->set(title,text);
		dialog_about->show();
	}
}

void Form_main::about_rules()
{
	QString fname(":/about/about_rules.html");
	QString title(tr("How to play"));
	about(fname, title);
}

void Form_main::about_sentinel()
{
	QString fname(":/about/about_sentinel.html");
	QString title(tr("About the original ..."));
	about(fname, title, true);
}

void Form_main::about_mhk()
{
	QString fname(":/about/about_mhk.html");
	QString title(tr("About ..."));
	about(fname, title, true);
}

void Form_main::about_remake()
{
	QString fname(":/about/about_remake.html");
	QString title(tr("About ..."));
	about(fname, title);
}

void Form_main::about_gpl()
{
	QString fname(":/about/about_gpl.html");
	QString title(tr("Gnu Public License"));
	about(fname, title);
}
}
