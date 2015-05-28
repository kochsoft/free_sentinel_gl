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
 * Class inheriting from auto-generated Ui::MainWindow 
 * Here all the MHK-programmed code concerning this form
 * will be implemented.
 * 
 * Markus-Hermann Koch, mhk@markuskoch.eu, 30.04.2015
 */

#ifndef MHK_FORM_MAIN_H
#define MHK_FORM_MAIN_H

#include <QCloseEvent>
#include <QMainWindow>
#include <QPixmap>
#include <iostream>
#include <string>
#include <map>
#include "ui_main.h"
#include "form_game_setup.h"
#include "form_about.h"
#include "io_qt.h"
#include "game.h"

typedef unsigned short ushort;

using std::cout;
using std::cerr;
using std::ostream;
using std::string;
using std::map;

using namespace mhk_gl;
using namespace game;

namespace display
{
class Form_main : public QMainWindow
{
private:
	Q_OBJECT

	Io_Qt io;
	Known_Sounds* known_sounds;
	Ui::MainWindow* uiMainWindow;

	/** Copy of dialog_setup_game->get_game_data which is saved here
	 * by new_game_object() for the benefit of restart_game(). */
	Setup_game_data active_game_data;
	
	QLabel* label_statusBar;
	QLabel* label_statusBar_energy;
	int energy_value;
	QPixmap* pixmap_robot_icon;
	QPixmap* pixmap_block_icon;
	QPixmap* pixmap_tree_icon;
	QPixmap* pixmap_big_icon;
	QPixmap* pixmap_energy;
	
	Dialog_setup_game* dialog_setup_game;
	Dialog_about* dialog_about;

	/** Prepares the menu. */
	void setup_menu();
	
	/** Prepares the setup game window. */
	void setup_dialog_setup();

	/** Prepares the status bar. */
	void setup_statusBar();
	
	/** Prepares the openGL environment within QOpenGLWidget. */
	void setup_Widget_OpenGl();

	/** Called by the constructor. Sets up event connections and other
	 * preliminary stuff the qt Designer is overtasked with. */
	void setup_form();
	
	/** For opening for an 'about' message box. Accepts html as Qt does.
	 * @param bool use_message_box: If true a QMessageBox will be used
	 * instead of this->dialog_about. */
	void about(QString fname_resource, QString title, bool use_message_box=false);

	/** Parsed contents of :/misc/gravity.txt */
	map<string,string> planetary_data;
	vector<string> planetary_order; // Ordered copy of the keys to planetary_data.
	
	/** @return the linux timestamp. Useful as a random seed for qsrand(). */
	static uint get_timestamp();

	/** Parses misc/gravity.txt in order to get the appropriate E_SCENERY for the
	 * given SCENERY.
	 * @param int selection: Selection index from the gravity combobox.
	 *   May be <0 for a random selection.
	 * @return the scenery setting appropriate for the selection in the gravity
	 * cobobox.*/
	E_SCENERY get_scenery_by_selection(int);
	
	/** Return the light color as stated within misc/gravity.txt. */
	QVector4D get_light_color_by_scenery(E_SCENERY);

	/** Properly starts a new game using the given game pointer.
	 * Tool function for ok_setup_*() and the constructor. Also
	 * sets up some signal-slot connections concerning the new object. */
	void plugin_new_game(Game* game);

	/** Generates a new Game object based on this->dialog_setup_game.
	 * @param E_GAME_TYPE type: CAMPAIGN, CHALLENGE or CUSTOM.
	 * @param uint seed: Random seed that will be passed on to Landscape.
	 * @param Setup_game_data* game_data: Pointer to game data to be used.
	 *   May be 0. In that case dialog_setup_game->get_game_data()
	 *   will be used.
	 * @return a new game pointer. Make sure to plug it into the Widget_OpenGl
	 *   in an appropriate fashion. I.e.:
	 *     1.) set the old pointer to 0 in order to stop the game.
	 *     2.) delete the old object.
	 *     3.) plug in the new object.
	 * Or simply use this->plugin_new_game(Game*)!
	 */
	Game* new_game_object(E_GAME_TYPE type, uint seed, Setup_game_data* game_data=0);
	
public:
	/** Simple constructor. */
	Form_main(QWidget* parent=0, E_DEBUG_LEVEL debug_level=E_DEBUG_LEVEL::MESSAGE,
		ostream* stdout=&cout, ostream* stderr=&cerr);
	~Form_main();

	/** Needs to be overwritten in order to have exit_program() called
	 * in the case of the user closing the window by clicking on the
	 * (X) in the upper right. */
	virtual void closeEvent(QCloseEvent* event);
	//virtual void resizeEvent(QResizeEvent* event);
	
	/** Returns the name of this program. */
	QString get_program_name() const;
	
private slots:
	void new_game();
	void restart_game(uint seed=0);
	void similar_game();
	void exit_program();
	void toggle_fullscreen();
	void ok_setup_campaign();
	void ok_setup_challenge();
	void ok_setup_custom();
	// Depending on E_GAME_TYPE calls ok_setup_campaign, challenge or custom().
	void request_new_game();
	void update_statusBar_text(QString text);
	/** Sets the energy meter to the new value. */
	void update_statusBar_energy(int energy);
	/** Called by game after a successful campaign level. 
	 * Writes something appropriate into the status bar and updates
	 * the campaign text line within the setup form. */
	void update_campaign_code(int energy);
	void disable_program(QString);
	void about_rules();
	void about_sentinel();
	void about_mhk();
	void about_remake();
	void about_gpl();
};
}

#endif
