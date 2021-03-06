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
 * Wrapper class for easy handling of Ui::Dialog_setup_game.
 *
 * Markus-Hermann Koch, mhk@markuskoch.eu, 03.05.2015
 */

#ifndef MHK_FORM_SETUP_GAME_H
#define MHK_FORM_SETUP_GAME_H

#include <QDialog>
#include <QString>
#include <string>
#include <ostream>
#include <vector>
#include <map>
#include "config.h"
#include "ui_dialog_setup_game.h"

using std::string;
using std::ostream;
using std::vector;
using std::map;

namespace display
{
/** Forward declaration for the benefit of Setup_game_data. */
class Dialog_setup_game;

struct Setup_game_data
{
	// Textbox for a fixed level code.
	QString lineEdit_campaign;
	// Difficulty setting for a random challenge.
	int horizontalSlider_challenge;
	// What is the maximum value for the sentry spinbox?
	int spinBox_sentries_max;
	// How many sentries shall be generated?
	int spinBox_sentries;
	// Parameter for the height of the mountains. The stronger the
	// gravitational field the plainer the landscape (and the simpler the game).
	int combobox_gravity;
	// Age of the landscape. The younger the more plateaus. 0=young, 2=old.
	int combobox_age;
	// Will the enemies spin to the left, the right or individually randomized?
	int combobox_rotation_type;
	// After being seen by a sentry or the sentinel.
	// This is the time in seconds nothing will happen.
	int spinBox_psi_shield;
	// After the Psi Shield has been drained the first energy will be absorbed.
	// After that it will take 'confidence' seconds for the next energy to be lost.
	// This is also the time it will take for an opponent to reduce an empty
	// robot to a boulder or a boulder to a tree.
	int spinBox_confidence;
	// How long will a sentry or the sentinel take to spin 360 degrees?
	int spinBox_spin_period;
	// If and only if true the sentries and the sentinel will have the
	// ability to manifest meanies out of trees.
	bool checkBox_meanies;
	// Should the environment shown in the thunderdome be dependent on the planet?
	bool checkBox_random_scenery;
	// Height of the board in squares.
	int spinBox_rows;
	// Width of the board in squares.
	int spinBox_cols;
	// Full rotation period of the player at top speed in seconds.
	int spinBox_self_spin;
	// Energy of the system per height level in terms of trees. Excluding
	// sentries, the sentinel, the player robot, and his initial excess energy.
	int spinBox_energy;
	// Resistence of a solid object if under attack by an antagonist.
	// After this time the object starts fading/transmuting.
	int spinBox_object_resilience;
	// Horizontal field of view for the antagonists.
	int spinBox_antagonist_fov;

	// Pointer to the form this Setup_game_data is using. May in fact be 0.
	Dialog_setup_game* src;

	/** Simply checks whether or not there is a src pointer != 0.
	 * @return true if and only if src!=0. */
	bool is_valid() { return (src!=0); }
	
	/** Constructs struct and, for good measure, takes a first snapshot
	 * if a src!=0 was given. */
	Setup_game_data(Dialog_setup_game* src=0);
	/** Copy constructor resulting in a whole new object. */
	Setup_game_data(const Setup_game_data&);
	/** Evaluates what this->src points to, filling this struct's data. */
	void doSnapshot();
	/** Checks if src != 0. If not sets its values to the values
	 * within this structs data. */
	void write_onto_src();
	/** Like write_onto_src(). But restricts itself to the campaign code
	 * and custom settings. */
	void write_campaign_data_onto_src();
};

class Dialog_setup_game : public QDialog
{
Q_OBJECT
signals:
	void start_campaign();
	void start_challenge();
	void start_custom();

private slots:
	void onClick_campaign();
	void onClick_challenge();
	void onClick_custom();
	// Close the window without starting a new game. Revert to defaults.
	void onClick_cancel();
	// Close the window without starting a new game.
	void onClick_close();
	// Revert to defaults.
	void onClick_restore_defaults();

private:
	Ui::Dialog_setup_game* ui_dialog_setup_game;
	/** Is initialized with the initial values of this form and will
	 * be used by the onClick_restore_defaults() method. */
	Setup_game_data* default_setup_game_data;
	/** Will be overwritten with current form contents each time
	 * 1.: A new game is started, or 2.: The close button is clicked.
	 * Is initialized with the initial values of this form. */
	Setup_game_data* last_close_setup_game_data;
	ostream* stdout;
	ostream* stderr;

	/** Written to by setup_campaign_level this variable holds the
	 * offset value parsed from the campaign code which is to be used
	 * as random seed. */
	uint campaign_seed;
	
	void populate_combobox_gravity(vector<string> names);
	void populate_combobox_rotation_type();
	void populate_combobox_age();
	
	//> Challenge and campaign code generation and handling. ---------
	/** Tool functions for getting the 'the larger the harder' n-values from
	 * the form sheet for campaign code generation. */
	uint get_n_gravity_from_form();
	uint get_n_age_from_form();
	uint get_n_sentries_from_form();
	uint get_n_energy_from_form();
	
	uint get_max_n_gravity_from_form();
	uint get_max_n_age_from_form();
	uint get_max_n_sentries_from_form();
	uint get_max_n_energy_from_form();
	
	/** Note that the last 16 bit of the code are technically unimportant.
	 * However, we want not any old number to be a valid code. Hence this
	 * function will
	 * @return true if and only if the given code returns
	 * (code&0xffff) % 257 == 0.
	 */
	bool is_valid_code(uint code);

	/** Parses the campaign code from the textbox within the setup form. */
	uint get_code_from_form();

	/** Commits the given values to the ui form. */
	void put_values_into_form(uint n_gravity, uint n_age,
		uint n_sentries, uint n_energy);
	
	/** Makes some decision to make a setting set more sane. I.e.:
	 * Corrects it if gravity is minimal and age at the same time is
	 * ancient leaing to worlds with great mountains but no plateaus
	 * to scale them. */
	void harmonize_values(uint& n_gravity,
		uint& n_age, uint& n_sentries, uint& n_energy);

	/** Tool functions for create_challenge(..) Each is rigged thus that
	 * the maximum value for the parameter yields the maximum challenge. */
	static float sentry_value(uint n);
	static float gravity_value(uint n);
	static float age_value(uint n);
	static float energy_value(uint n);
	static float get_value(uint n_gravity, uint n_age, uint n_sentries, uint n_energy);
	/** @return the value of the most severe challenge according to the
	 * *_value-fcts and according to the maximum setting given by the qt ui. */
	float get_maximum_value();
	//< --------------------------------------------------------------

public:
	/** Getter for the seed generated by setup_campaign_level(). */
	uint get_campaign_seed() { return this->campaign_seed; }

	/** @return the level number associated with the given code. */
	static int get_level_number(uint code);
	
	/** 
	 * @param int energy_loot: The energy level the player has taken from his last
	 *   landscape victory. I.e.: The number of levels to advance.
	 * @param uint& new_code: Will be filled with the new code.
	 * @return false if and only if the campaign was won. */
	bool get_next_campaign_code(int energy_loot, uint& new_code);

	Setup_game_data* get_game_data() { return this->last_close_setup_game_data; }

	/** Takes the given code. Makes a right shift of 16 bit.
	 * Uses the remainder as level number.
	 * Uses setup_challege(..) on that difficulty level.
	 * @param uint code: Campaign code taken from the form.
	 * @return true if and only if the setup was successful and rule conform. */
	bool setup_campaign_level(uint code);
	
	/** Randomly (using the given seed) fills this form with a
	 * challenge. The higher the level, the more severe.
	 * @param uint seed: Use this random seed. Useful for campaign mode, too.
	 * @param int level: In { 0, .., slider_max }
	 *   If outside this range level %= (slider_max+1) will be silently applied.
	 * @return nothing. Instead the rolled values are entered into the form
	 *   prior to game creation.
	 */
	void setup_challenge(uint seed, int level);

	/* @param vector<string>& planets: Will be used to fill the combobox_gravity. */
	Dialog_setup_game(vector<string>& planets, QWidget* parent=0);
	~Dialog_setup_game();

	Ui::Dialog_setup_game* get_ui() { return this->ui_dialog_setup_game; }
};
}

#endif

