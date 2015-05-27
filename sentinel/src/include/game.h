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
 * This game, too, needs a Game class initializing the setup
 * and taking track of the progress of the game.
 * 
 * Markus-Hermann Koch, mhk@markuskoch.eu, 13.05.2015
 * 
 * Note about multithreading implied by QTimer: Apparently there is none.
 * According to
 * 
 *   http://stackoverflow.com/questions/23555631/can-i-use-qtimer-to-replace-qthread
 * 
 * QTimers run within the main GUI thread context and cause no race conditions.
 * Suits me fine. Sentinel runs fast enough as it does while the only chance
 * for multithreading would be view analysis for the diverse agents -- which
 * still is fast enough when queued.
 */

#ifndef MHK_GAME_H
#define MHK_GAME_H

#include <map>
#include <QTimer>
#include <QSound>

#include "form_game_setup.h"
#include "landscape.h"
#include "io_qt.h"
#include "scanner.h"

using std::map;
using std::pair;

using namespace display;

namespace game
{
enum E_GAME_TYPE { CAMPAIGN, CHALLENGE, CUSTOM };
enum E_GAME_STATUS { SURVEY, PRELIMINARY, RUNNING, SENTINEL_ABSORBED, TOWER_TAKEN, WON, LOST };
enum E_UPDATE_GAME_STATUS_BY { SURVEYOR, MANIFESTOR, DISINTEGRATOR, GONER_REMOVER,
	HYPERSPACE, TRANSFER, ANTAGONIST };
// ABSMANI means both absorption and manifestation are possible. Exchange means mind transfer.
enum E_POSSIBLE_PLAYER_ACTION { NO, ABSORPTION, MANIFESTATION, ABSMANI, EXCHANGE };

class Known_Sounds
{
private:
	bool sound_on;
	map<string, QSound*> sounds;

public:
	bool get_sound() { return sound_on; }
	bool toggle_sound() { sound_on = !sound_on; return sound_on; }
	
	void play(string key)
	{
		if (sound_on) sounds.at(key)->play();
	}
	
	/** Default constructor setting sensible defaults. */
	Known_Sounds()
	{
		sounds.insert(pair<string,QSound*>("absorption_sentinel",new QSound(":/sound/absorption_sentinel.wav")));
		sounds.insert(pair<string,QSound*>("absorption",new QSound(":/sound/absorption.wav")));
		sounds.insert(pair<string,QSound*>("delayed_plop",new QSound(":/sound/delayed_plop.wav")));
		sounds.insert(pair<string,QSound*>("tick",new QSound(":/sound/tick.wav")));
		sounds.insert(pair<string,QSound*>("frog",new QSound(":/sound/frog.wav")));
		sounds.insert(pair<string,QSound*>("frog_reverse",new QSound(":/sound/frog_reverse.wav")));
		sounds.insert(pair<string,QSound*>("plop",new QSound(":/sound/hyperspace_plop.wav")));
		sounds.insert(pair<string,QSound*>("victory",new QSound(":/sound/victory.wav")));
		sounds.insert(pair<string,QSound*>("defeat",new QSound(":/sound/defeat.wav")));
		sound_on = true;
	}
	
	~Known_Sounds();
};

class Game : public QObject
{
Q_OBJECT

signals:
	void update_statusBar_text(QString);
	void update_statusBar_energy(int);
	void set_light_filtering_factor(QVector4D,int);
	void request_paintGL();
	/** After a game was won in CAMPAIGN mode. 
	 * @param energy_loot: Amount of energy the synthoid has taken with him. */
	void update_campaign_code(int energy_loot);

private:
	/** Light effect for the hyperspace loadup phase. */
	static const QVector4D hyperspace_light_factor;
	static const QVector4D light_attack_light_factor;
	static const QVector4D heavy_attack_light_factor;
	static const QVector4D absorbed_light_factor;
	
	Known_Sounds* known_sounds;

	/** For debugging messages. */
	Io_Qt* io;

	/** Game type that was chosen. */
	E_GAME_TYPE game_type;

	/** Game status. Managed by update_game_status(). */
	E_GAME_STATUS status;

	/** Landscape for generating and retaining the board. */
	Landscape* landscape;
	
	/** Scanner object for evaluating line-of-sight situations. */
	Scanner* scanner;

	/** Pointer to the figures on the board. Only base figures are therein.
	 * i.e. those that actually stand upon the squares. Use Figure methods
	 * in oreder to access stacked items. */
	Board<Figure>* board_fg;
	
	/** Player Data object. */
	Player_Data* player;
	
	/** Once the sentinel starts disintegrating hyperjumps and mind
	 * transfers are forbidden. */
	bool sentinel_disintegrating;
	
	/** From the checkbox concerning meanies within setup_data. */
	bool do_meanies;
	
	/** A meanie */
	QTimer* meanie_timer;

	/** Should be 0 as a rule unless an active hyperspace process is present. */
	QTimer* hyperspace_timer;

	/** QTimer pause related helpers. */
	bool meanie_timer_paused;
	int meanie_timer_remaining;
	bool hyperspace_timer_paused;
	int hyperspace_timer_remaining;
	/** Reset the above for helper variables to 0s and false. */
	void reset_timer_helpers();
	
	/** For damage control concerning antagonist attacks. */
	float framerate;
	
	/** Object 'confidence' */
	float object_resilience;
	
	/** Checks whether the game status needs to be changed and does so if necessary.
	* This function should be called at the end of end_survey(), disintegrate_figure(),
	* manifest_figure(), remove_goners_from_board(), transfer(), and hyperspace().
	* 1.) The game snaps from SURVEY to PRELIMINARY as soon as end_survey() is called.
	* 2.) The game snaps from PRELIMINARY to RUNNING as soon as manifesting or
	*   disintegrating figures are found on the board.
	* 3.) The game snaps from RUNNING to SENTINEL_ABSORBED as soon as
	*   remove_goners_from_board() has called and the board was found
	*   to be without The Sentinel.
	* 4.) The game snaps to TOWER_TAKEN after transfer() was called moving
	*   the player's consciousness onto The Sentinel's tower.
	* 5.) The game snaps to WON or LOST after being called from hyperspace(). 
	* @param E_UPDATE_GAME_STATUS_BY caller: Thus update_game_status() knows
	*   who was calling it. This should faciliate implementation.
	*/
	void update_game_status(E_UPDATE_GAME_STATUS_BY caller);

	/** If the player's consciousness leaves a robot shell that shell should
	* face in the direction the player looked into last. This function
	* assumes there is a robot on the player square and updates its
	* orientation according to the player's View_Data. 
	* Call shortly before comitting the actual transfer. */
	void match_current_robot_to_viewer_Data();

	/** Works on top of a figure stack and triggers its transmutation into the new type.
	 * This requires that said top figure is STABLE. */
	void transmute_figure(QPoint, E_FIGURE_TYPE new_type);
	
	/** Checks all stack-tops for Figures with matter state GONE,
	* pops and deletes them. If afterwards the stack is empty the pointer
	* on the figure board is reset to 0.
	* This function calls update_game_status(GONER_REMOVER) if and only
	* if it has just removed The Sentinel from play.
	*/
	void remove_goners_from_board();

	/** Tool fct for get_possible_interactions. */
	bool is_stack_of_stable_blocks(Figure* figure);
	
	/** Analyses the game situation and player position in order
	 * to determine how the player may interact with a given result
	 * from this->get_mosue_target(). */
	E_POSSIBLE_PLAYER_ACTION get_possible_interactions(QPoint,Figure*);

	/** If the player is detected the antagonist wants to center his
	 * attention on the player. If need be he turns against his usual
	 * turning direction in order to get the player central to his view. 
	 * This fucntion determines whether this turning back is necessary.
	 * It is a tool for antagonist_action(..)
	 * @param QPoint pos_robot: Square of the player robot. It is assumed,
	 *   that the robot was detected by this antagonist.
	 * @param QPoint pos_antagonist: Position of the antagonist on the
	 *   board.
	 * @param float phi: Phi angle of the antagonist.
	 * @param bool turns_positive: Natural direction of the antagonist.
	 * @return true if and only if the antagonist would turn
	 * in a mathematically positive way.
	 */
	E_ANTAGONIST_ACTION antagonist_turn_direction(QPoint pos_robot,
		QPoint pos_antagonist, float phi, bool turns_positive);
	
	/** @return the player robot position within the targets or (-1,-1) if unavailable. */
	QPoint find_player_in_targets(vector<Antagonist_target>&);
	
	/** Takes a vector as was returned by Scanner::get_all_board_positions_* 
	 * and removes such squares that are of CONNECTION type.
	 * This is useful for e.g. antagonist_tree_manifestation since else
	 * antagonists would happily manifest trees on connection squares.
	 */
	vector<QPoint> restrict_to_free_non_CONNECTION(vector<QPoint> data);
	
	/** Tool function for SLOT revert_meanie_to_tree(). Scans the board
	 * for a stable meanie. 
	 * @return QPoint_Figure pair with either the info for the stable meanie
	 * or [(-1,-1),0] if none was found. */
	QPoint_Figure find_stable_meanie();
	
	/** Seeks a stable meanie and turns it into a tree. */
	void revert_meanie_to_tree();

	/** Turns an arbitrary tree into a meanie. */
	void antagonist_summon_meanie(QPoint antagonist_pos, Figure* antagonist);
	
	/** Employs methods from Scanner and Landscape to randomly manifest
	 * a tree within the field of view of the antagonist. */
	void antagonist_tree_manifestation(QPoint antagonist_pos, Figure* antagonist);
	
	/** Called by antagonist_action() which in turn is called by doProgress (and
	 * doubles determining the momentary turning direction of the antagonist).
	 * antagonist_attack() calls figure_mount_attack and evaluates the results
	 * of all attacks on the player and inanimate objects that are attacked
	 * by antagonists. It also calls update_game_status(..) in case of game loss
	 * and organizes background color effects.
	 * @param QPoint player_pos: If not (-1,*) it will be assumed that this is
	 *   the board position where the player was detected by this antagonist.
	 * @param Figure* antagonist: Figure pointer of the antagonist.
	 *   antagonist->mount_attack() will be called.
	 * @param vector<Antagonist_target> targets: Antagonist targets ultimately
	 *   returned from the Scanner object earlier this turn.
	 * @return true if and only if the antagonist did attack the player this round.
	 * Important for regeneration of confidence and shields if there was no attack
	 * at all during one round.
	 */
	bool antagonist_attack(QPoint player_pos, QPoint antagonist_pos,
		Figure* antagonist, vector<Antagonist_target>& targets);
	
	/** Tool fct for progress. Gets the action (turning direction) a given figure
	 * will take based on its view.
	 * This fct also initiates antagonist battle action calling antagonist_attack()
	 * @param QPoint pos: Position of a figure on the board.
	 *   Action will be evaluated based on this figures field of view.
	 * @param Figure* figure: Pointer to the potential antagonist.
	 * @param bool& hitPlayer: As stated above an attack will be mounted by
	 *   the figure if it is an antagonist. Should this attack hit the player
	 *   hitPlayer will be set to true. false else.
	 * @return This figure's action. Innert objects like trees always
	 *   return STILL. Antagonists may move either forward or backward
	 *   or may also stand still depending on what they see. */
	E_ANTAGONIST_ACTION antagonist_action(QPoint pos, Figure* figure, bool& hitPlayer);
	
	/** 
	 * Tool function for antagonist_action.
	 * Employs this scanner in order to get a vector of all targets
	 * the given antagonist sees right now. 
	 * @param QPoint board_pos: Board position of the antagonist in question.
	 * @param bool seek_trees: Should be set to false for any antagonist that
	 *   is not seeking a tree in the hopes of making a meanie.
	 * @return vector<Antagonist_target> of all possible targets for this antagonist. */
	vector<Antagonist_target> get_antagonist_targets(QPoint board_pos, bool seek_trees=false);
	
	/** Checks each board square for stacks of figures (including those of height 1).
	 * @return the figure on the top of each stack. Obviously a !=0 pointer. */
	 vector<QPoint_Figure> get_all_top_figures();
	 
	 /** Counts the energy left in the landscape. */
	 int count_landscape_energy();
	 
public:
	E_GAME_TYPE get_game_type() { return this->game_type; }
	Player_Data* get_player() { return this->player; }
	E_GAME_STATUS get_status() { return status; }
	Landscape* get_landscape() { return this->landscape; }
	Board<Figure>* get_board_fg() { return this->board_fg; }
	bool toogle_sound() { return this->known_sounds->toggle_sound(); }

	/** Updates the states of all non-player figures by dt for each calling
	 * progress in turn. It also calls remove_goners_from_board().
	 * @param float dt:
	 * @return true if any figure has a visible progress.
	 * Note: Will ALWAYS return false and do nothing else unless in
	 *   RUNNING or SENTINEL_ABSORBED state.
	 * TODO: May yet be optimized by respecting the FOV of the player. */
	bool do_progress(float dt);
	
	/** Employs this->scanner in order to identify which object or square
	 * if any is under the mouse pointer.
	 * @param float mouse_gl_x, mouse_gl_y: Mouse coordinates in [-1,1]^2.
	 * @param QPoint& board_pos: Will be filled eihter with (-1,-1) if nothing
	 *   is under the mouse, with the coordinates of the square the mouse points
	 *   to, if it does not point at an object, or with the board coordinates
	 *   of the object the mouse is pointing at.
	 * @param Figure*& figure: Will be filled with 0 if the mouse is not
	 *   pointing at an object or with the pointer to that object.
	 * @return A vector of possible interactions for the player with this result.
	 */
	E_POSSIBLE_PLAYER_ACTION get_mouse_target(float mouse_gl_x, float mouse_gl_y,
		QPoint& board_pos, Figure*& figure);
	
	/** @return a human readable string describing the game status. */
	QString get_game_status_string();
	
	
	/** Updates View_Data for survey mode with this angle. */
	void set_survey_view_data(float dphi, float dtheta);
	
	/** Performs a U-turn if not in SURVEY, WON or LOST state. */
	void do_u_turn();
	
	/** Calls update_game_status() in order to have the game progress
	 * from SURVEY to PRELIMINARY. Also sets the player view into the robot. */
	void end_survey();
	
	/**
	 * Assumes that the action is legal.
	 * Evaluates the figure stack on the given position and adds the
	* new Figure to its top setting up the figure's altitude correctly.
	* @return true if and only if manifestation was successful.
	* Note that manifestation may only fail if attempted by the robot
	* due to lack of energy. This function also takes care of deducting
	* the cost of a manifestation triggered by the robot.
	*/
	bool manifest_figure(QPoint pos, E_FIGURE_TYPE type, bool by_robot);

	/** Works on top of a figure stack.
	* Sets the target figure's E_MATTER_STATE to DISINTEGRATING
	* and tells the target Figure if it was the robot that did this.
	* @param pos: Board coordinates of the target figure. It will always be at
	*   the top of the stack.
	* @param bool by_robot: Has this disintegration been triggered by the
	*   robot? Important for energy distribution later on.
	* Note: If the Figure's state was not STABLE to begin with nothing happens.
	*/
	void disintegrate_figure(QPoint pos, bool by_robot);

	/**
	* Does nothing if hyperdrive_timer != 0.
	* Starts the warp drive. I.e.:
	* Generates a timer object this->hyperdrive_timer which, after 3 seconds,
	* will implement the jump by calling hyperspace_jump().
	* @return true if and only if the request was successfully issued.
	*/
	bool hyperspace_request();

	/** Assuming there is a robot shell on the destination square.
	* First calls match_robot_to_Viewer_Data().
	* Then transfers the player's consciousness to said target robot shell
	* and have his viewing angle focus on the robot shell he just left. */
	void transfer(QPoint destination);

	/** Stops and restarts the hayperdrive and meanie QTtimers 
	 * hyperdrive_timer and meanie_timer, if they exist, that is. */
	void pause_timers();
	void unpause_timers();
	
	/** Sets up the game bringing it to status SURVEY.
	 * 	 @param Mesh_Data* mesh_connection, mesh_odd, mesh_even: Pointers to
	 *   the mesh data associated with CONNECTION squares, ODD squars and EVEN
	 *   squares passed on to Landscape constructor.
	 */
	Game(E_GAME_TYPE game_type, uint seed, Setup_game_data* setup,
		QOpenGLWidget* parent, Io_Qt* io, Known_Sounds* known_sounds,
		float framerate, Mesh_Data* mesh_connection,
		Mesh_Data* mesh_odd, Mesh_Data* mesh_even, Mesh_Data* mesh_sentinel,
		Mesh_Data* mesh_sentinel_tower,	Mesh_Data* mesh_sentry,
		Mesh_Data* mesh_tree, Mesh_Data* mesh_robot,
		Mesh_Data* mesh_block, Mesh_Data* mesh_meanie);
	~Game();
	
private slots:
	/* Triggered by the timeout of the hyperdrive_timer. Resets
	 * the timer to 0, the hyperspace_request to false and
	 * actually implements the jump complete with 3 units of energy consumption.
	 * Three cases:
	 * 1.) The warp drive was activated from The Sentinel's tower.
	 *   Simply call update_game_status().
	 * 2.) Player's energy drops below zero.
	 *   Simply call update_game_status().
	 * 3.) Neither happens. No need to call update_game_status() unless
	 *   we are still PRELIMINARY.
	 *   In any case first call match_robot_to_Viewer_Data().
	 *   Next choose a suitable hyperspace destination square,
	 *   instant-manifest a robot there, and place the player's consciousness within.
	 *   As a finishing touch randomize his Viewer_Data phi angle! */
	void hyperspace_jump();
	
	/** Triggered by the timeout of this->meanie_timer. 
	 * Turns a stable meanie (there should be one at most. However, zero may happen
	 * if the meanie was successful or if the player absorbed it) back into a tree
	 * by means of transmutation. */
	void meanie_timeout_slot();
};
}

#endif
