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
 * The secret heart of Sentinel GL this class generates landscapes. 
 * It does so from a random seed potentially given as parameter to the
 * process and based on the ideas documented in :/resources/doc/plan.txt
 *
 * Markus-Hermann Koch, mhk@markuskoch.eu, 12.05.2015.
 */

#ifndef MHK_LANDSCAPE_H
#define MHK_LANDSCAPE_H

#include <vector>
#include <map>
#include <stack>
#include <string>
#include <QOpenGLBuffer>
#include "data_structures.h"
#include "io_qt.h"

using std::vector;
using std::map;
using std::multimap;
using std::stack;
using std::string;

using namespace display;

typedef unsigned int uint;

namespace game
{
enum E_SQUARE_TYPE { CONNECTION, ODD, EVEN, UNDEFINED };
enum E_FIGURE_TYPE { TREE, BLOCK, MEANIE, ROBOT, SENTRY, SENTINEL, TOWER };
enum E_ANTAGONIST_ACTION { STILL, MOVING_FORWARD, MOVING_BACKWARD };
enum E_MATTER_STATE { STABLE, MANIFESTING, DISINTEGRATING, TRANSMUTING, GONE };

struct Attack_duration
{
	/** Number of frames the target object is under attack by this Figure.
	 * Inanimate objects will be transmuted every object_confidence*framerate
	 * duration_frames. The player will ignore the attack while the psi-
	 * shield wears down. Then like an inanimate object. Once the antagonist
	 * loses focus -- and be it for 1 frame the Attack_duration object will
	 * be removed. However, the player psi-shield and confidence will only
	 * recharge, when he is completely out of attack by anyone.
	 * 
	 * Note that it is the _board_position_ that is under attack.
	 * An attack continues if the player transfers to another location.
	 * After all the robot he left behind is still there. And even
	 * if the robot is gone. There may be blocks, trees on blocks, or
	 * other blocks that fill the bill!
	 * 
	 * Also note that non-block-based trees will not be considered here.
	 * They are part of the reaction on synthoids in partial cover and
	 * will be handled by GAME completely (instead of losing the first energy
	 * to the attack a meanie will be created)
	 */
	int duration_frames;
	/** Target figure's board position. */
	QPoint board_pos;
	/** Visibility. Important for the determination of the type of threat. */
	E_VISIBILITY visibility;
	
	void inc() { duration_frames++; }
	
	Attack_duration(int duration_frames, QPoint board_pos, E_VISIBILITY visibility)
	{
		this->duration_frames = duration_frames;
		this->board_pos = board_pos;
		this->visibility = visibility;
	}
};

/** A game piece detached from its position on the board. */
class Figure
{
private:
	/** Mesh_Data pointer for the figure. This figure has no rights
	 * to do any changes on the mesh! */
	Mesh_Data* mesh;
	/** Set and needed only during transmutation. This will hold the
	 * old this->mesh pointer and be set to 0 again once transmutation is completed.
	 * use this->set_type(..) to trigger transmutation. */
	Mesh_Data* mesh_transmutation_origin;
	E_FIGURE_TYPE type;
	E_MATTER_STATE state;
	/** Relevant for phasing in and out. Keeps track of the progress
	 * of the phasing process and doubles as uniform fade for the
	 * openGL fragment shaders! ;-) Consequently lives in [0,1]. */
	float fade;
	/** The total time a phasing action needs. */
	float fading_time;
	/** Phi angle describing the orientation of the object. */
	float phi;
	/** Only relevant for the active robot during the moment it has been
	 * transfered-to. After all it initially looks into the direction 
	 * of the point of origin of transfer. */
	float theta;
	/** Positive or negative. Inverse spin speed of this figure
	 * in s/360 degrees. Will be 0 for trees, blocks, the tower and robot shells.
	 * 0 is a special case meaning not infinite but zero angle speed.
	 */
	float spin_period;
	/** Horizontal field of view for this figure in degrees.
	 * Determines the limits of possible line of sights at any given this->phi. */
	float fov;
	/** Pointer to the figure above this one. 0 if none present.
	 * Note that this object will delete the figure above during its own destruction. */
	Figure* figure_above;
	/** Pointer to the figure below this one. 0 if none present. */
	Figure* figure_below;
	
	/** false as a rule. Will be set to true if the robot sets this figures
	 * state to DISINTEGRATING. absorption_triggered_by_robot is used by
	 * this->get_energy_for_robot(). */
	bool absorption_triggered_by_robot;
	
	/** Set of attacks this antagonist is performing right now. 
	 * Oh yes... for all their weaknesses this is a strength: They can
	 * attack multiple targets at the same time! */
	vector<Attack_duration> attacks;

public:
	/**
	 * @param vector<QPoint> targets: Target list passed here
	 *   from the Game object. It will be the Game object, too
	 *   that actually will evaluate the attack.
	 * @return the current this->attacks after updating by
	 * vector<QPoint> targets. Adds +1 to each counter that was
	 * already present, removes counters which's QPoints are missing
	 * within targets, and initializes new Attack_durations to hitherto
	 * unknown points.
	 */
	vector<Attack_duration> mount_attacks(vector<Antagonist_target>& targets);
	
	Mesh_Data* get_mesh_prototype() { return mesh; }
	/** Relevant for transmutating objects. */
	Mesh_Data* get_old_mesh() { return mesh_transmutation_origin; }
	E_FIGURE_TYPE get_type() { return type; }
	/** Triggers transmutation. Most notably sets state=TRANSMUTING. */
	void set_type(E_FIGURE_TYPE new_type, Mesh_Data* new_mesh_prototype);
	E_MATTER_STATE get_state() { return state; }
	/** Sets the new state and the fact whether or not the caller was the robot. */
	void set_state(E_MATTER_STATE new_state, bool by_robot);
	float get_fade() { return fade; }
	float get_fading_time() { return fading_time; }
	float get_phi() { return phi; }
	void set_phi(float phi) { this->phi = phi; }
	float get_theta() { return theta; }
	float get_fov() { return fov; }
	float get_spin_period() { return spin_period; }
	void set_spin_period(float spin_period) { this->spin_period = spin_period; }

	/** Recursive. Will send this figure to the top of the stack.
	 * Will also make sure that fg->figure_below is properly set.
	 * Exception: the given pointer is 0. Then this->figure_above=0 and done.
	 * @param c: is a debugging parameter. Do not set it. */
	void set_figure_above(Figure* fg, int c=0);

	/** Height of the figure's mesh. Needed for exact visibility calculation. */
	static float get_mesh_height(E_FIGURE_TYPE);
	
	/** Can be 1 or 2. Relevant for partial visibility and stacking.
	 * DO NOT CONFUSE WITH get_altitude_above_square().
	 * @return 1 if this is a block and 2 else. */
	static int get_height(E_FIGURE_TYPE type);
	
	/** Convenience function to check whether this figure has left the game.
	 * @return true if and only if this->state == E_MATTER_STATE::GONE.	 */
	bool is_gone();
	/** Convenience function to check whether this figure is stable.
	 * @return true if and only if this->state == E_MATTER_STATE::STABLE.	 */
	bool is_stable();
	/** @return true if and only if this is a stable or disintegrating
	 * Sentinel, sentry, or meanie. */
	bool is_antagonist();

	/** Some figures have eyes (the robot, sentinel, sentry and meanie).
	 * These eyes have positions in space.
	 * @return Said eye position in object coordinates. This is a constant
	 * for each figure type ultimately depending on the blender obj file.
	 * Note that the latter are organized such that the figure is looking
	 * in positive x direction. */
	static QVector3D get_eye_position_relative_to_figure(E_FIGURE_TYPE);
	
	/** Given the altitude and board coords of the square below this
	 * will calculate the eye position in world coordinates. */
	QVector3D get_eye_position_in_world(QPoint pos, int altitude_base);
	
	/** Direction of view depending on phi. */
	QVector3D get_direction();
	
	/** Starting at and including this Figure all figures within this stack
	 * keeping their order. Hence the last figure will be on top. */
	vector<Figure*> get_above_figure_stack();

	/** Starting below and excluding this Figure all figures within this stack
	 * keeping their order. Hence the last figure will be at the bottom. */
	vector<Figure*> get_below_figure_stack();
	
	/** Convenience function.
	 * @return the last element of get_above_figure_stack(). */
	Figure* get_top_figure();

	/** @return the pointer to the figure above this one.
	 * Should be 0 if there is none. */
	Figure* get_above_figure() { return figure_above; }

	/** @return the pointer to the figure below this one.
	 * Should be 0 if there is none. */
	Figure* get_below_figure() { return figure_below; }

	/** How far is it from the base of this object to the actual square? 
	 * e.g.: This distance is always 2 for The Sentinel since he always
	 * stands on a tower of height 2. It is the number of blocks beneath
	 * each other stackable item. */
	int get_altitude_above_square();
	
	/** Progresses this->phi and this->fade with respect to the given dt.
	 * @param float dt: Delta t in seconds since the last evaluation.
	 * @param E_ANTAGONIST_ACTION action: A forward moving figure simply does
	 *   this according to its self_spin value. A backward moving figure
	 *   does the same in the other direction. This will happen, if the
	 *   antagonist sees an object he wants to absorb, but sees it such
	 *   that turning 'forward' would move the target further out of central
	 *   focus. The antagonist then should turn backward until he is fully
	 *   facing the object he is interacting with.
	 *   Lastly, a still figure is fully focused on something and does not
	 *   move at all.
	 * @return true if and only if the progress made would warrant a
	 *   repaint of the openGL scene if the Figure were within the viewport.
	 * Note: This function also exercises the power to update matter state.
	 * I.e.: If fading progresses to 0 the state will be set to GONE.
	 * If fading progresses to 1 the state will be set to STABLE. 
	 */
	bool progress(float dt, E_ANTAGONIST_ACTION action=E_ANTAGONIST_ACTION::STILL);


	/** @return energy value for this type as stated in game rules. */
	static int get_energy_value(E_FIGURE_TYPE type);

	/** @return 0 if !absorption_triggered_by_robot. Else it returns this
	 *   Figure's energy value. This method is used to determine how 
	 *  much energy the robot receives upon a Figure's destruction. 
	 *  calls get_energy_value(..).
	 *  Used by Game::remove_goners_from_board(). */
	int get_energy_for_robot();
	
	/** Looks at the top of this figure's stack and deletes it. 
	 * Exception: This function does _not_ have this figure commit suicide.
	 * If above_figure == 0 nothing will happen.
	 *   @param bool only_if_GONE: Do this deletion action only if the
	 *     top of the stack figure is of status GONE.
	 * @return 0 if !absorption_triggered_by_robot. Else it returns this
	 *   Figure's energy value.
	 */
	int check_for_and_delete_top_figure(bool only_if_GONE);
	
	QString get_figure_name();
	
	/** Initializes this figure. */
	Figure(E_FIGURE_TYPE type, E_MATTER_STATE state, Mesh_Data* mesh,
		float phi, float theta, float spin_period, float fov, float fading_time);
	/** Copy constructor. Makes an independent copy in all but one aspect:
	 * the figure_below will be set to 0 for security reasons. 
	 * The stack above, however, will be copied analogously.
	 */
	Figure(const Figure& original);
	/** The destructor deletes the figure_above. Does nothing to the figure below. */
	~Figure();
};

class Square
{
private:
	Io_Qt* io;
	/** Square type. Light, dark or rock wall. */
	E_SQUARE_TYPE type;
	/** Only relevant if type in { ODD, EVEN}.
	 * The integer game height level of this square. */
	int level;
	/** Note: This is only a pointer. It is intended that the whole game
	 * contains only 3 such Mesh_Data objects that were all created using
	 * Widget_OpenGl::initialize_objects(). This object has no rights what-so-ever
	 * to modify the Mesh_Data object. It is entitled to its own QOpenGLBuffer though. */
	Mesh_Data* mesh_prototype;

public:
	Mesh_Data* get_mesh_prototype() { return mesh_prototype; }

	/** @param bool normalize: If true .normalize() is called on the
	 *   resulting vector.
	 * @return the three dimensional crossproduct u_ x v_, where
	 * u_=(u1,u2,u3) lacking u4, and v_=(v1,v2,v3) lacking v4. */
	static QVector3D qvec4_x_qvec4(QVector4D, QVector4D, bool normalize=false);
	
	/** Corner vertices of this square within the 3D environment. Size 4.
	 * Note that since board squares never move these squares will be
	 * saved directly in target coordinates around the squares (x,y) position
	 * on the game board. The transformation from object coordinates into
	 * world coordinates can be skipped. And that is well since it avoids
	 * the complex vertex distortion in dependence of neighboring squares!
	 */
	vector<Vertex_Data> vertices;
	
	/** Buffer for the corner vertices this->vertices on the GPU. */
	QOpenGLBuffer buf_vertices;
	
	/** Plateau id. Needed during landscape generation. */
	int plateau_id;
	
	/** Trivial getter for this->type. */
	E_SQUARE_TYPE get_type() { return type; }
	void set_type(E_SQUARE_TYPE type, Mesh_Data* mesh_prototype)
	{
		this->type = type;
		if (type==E_SQUARE_TYPE::CONNECTION) level = -1; // Connection tiles have no level.
		this->mesh_prototype = mesh_prototype;
	}
	
	/** Convenience getter for this square's game altitude.
	 * @return -1 if it has no unique game altitude. Else said altitude. */
	int get_altitude() { return level; }
	/** During the landscape generation it may become necessary to alter this value. 
	 * If that happens this->vertices will also be updated.
	 * This function is for ODD and EVEN squares. It requires that the E_SQUARE_TYPE
	 * was set accordingly. */
	void set_altitude(int level, int x, int y);
	/** Analogous function to set_altitude but focused on CONNECTION squares. */
	void set_sloped_altitudes(float alt_pp, float alt_mp, float alt_mm, float alt_pm, int x, int y);
	
	/** Fills this->buf_vertices with the vertices data and transfers it to the GPU.
	 * @return true if and only if all went well. */
	bool transfer_vertices_to_GPU();
	
	/** Trivial constructor setting plateau_id=level=-1, mesh_prototype=0 and type=EVEN. */
	Square(Io_Qt* io=0);
	
	/** Destroys this->buf_vertices. */
	~Square();
};

/** Simple micro class capsuling an array of Squares or Figures. */
template <class T> class Board
{
private:
	/** Should the Square* objects within the Square** board be deleted in the end? */
	bool delete_contents_upon_destruction;
	/** Number of squares on the board in x direction. */
	int width;
	/** Number of squares on the board in y direction. */
	int height;

public:
	/** Board pointer. The array of Square* will be created by the
	 * constructor and freed again by the destructor.
	 * public for convenience. Don't mess with it!
	 * The board is in row major. */
	T** board;
	
	int get_width() { return width; }
	int get_height() { return height; }
	/** Gets a pointer to a Square on the board.
	 * @param int x, int y: Board coordinates of the target square.
	 *   Both indecies are starting at 0.
	 * @return The pointer associated with the target coordinates. 
	 * @throws char* if the coordinates are out of range.
	 */
	T* get(int x, int y);
	T* get(QPoint p);
	/** Setter befitting get(..).
	 * @return true if and only if (x,y) is within the board limits.
	 * If they were not a char* is thrown. */
	bool set(int x, int y, T* item);
	bool set(QPoint p, T*item);
	
	/** Draws a silhoutte of the defined squares.
	 * @param byPlateauId: If true the last digit of the plateau ids will be shown.
	 *   else the altitude will be shown. */
	string toString(bool by_plateau_id=true);
	
	/** Initializes an empty board full of 0 pointers. */
	Board(int width, int height, bool delete_contents_upon_destruction);
	~Board();
};

class Landscape
{
private:
	/** Distribution of trees, sentries, the sentinel tower and The Sentinel.
	 * Copy, but do not modify. Consider the player loses and the game needs
	 * to be restarted! Especially: DO NOT USE THESE FIGURES IN AN ACTUAL 
	 * GAME. They are templates. initial_board_fg will delete them
	 * upon its own destruction. Since it is only needed by Landscape there
	 * should be no getter to this object! */
	Board<Figure> initial_board_fg;
	/** The indivudal squares of the board. */
	Board<Square> board_sq;

	/** For debugging stuff. */
	Io_Qt* io;
	/** Targeted altitude, age, tree number and sentry number for the growing landscape. */
	int gravity;
	int age;
	// Time for a complete antagonist turn-around in seconds.
	float spin_period;
	// Horizontal fov of an antagonist in degrees.
	float fov;
	// Time it takes an antagonist to fade in seconds.
	float fading_time;
	int trees;
	int sentries;
	int width;
	int height;
	int peak_altitude;
	int randomized_spin;
	/** Initialized (-1,-1). Will be filled by distribute_objects. */
	QPoint initial_robot_position;
	/** Random seed applied to qsrand() during construction of this object. */
	uint seed;
	Mesh_Data* mesh_connection;
	Mesh_Data* mesh_odd;
	Mesh_Data* mesh_even;
	Mesh_Data* mesh_sentinel;
	Mesh_Data* mesh_sentinel_tower;
	Mesh_Data* mesh_sentry;
	Mesh_Data* mesh_tree;
	Mesh_Data* mesh_robot;
	Mesh_Data* mesh_block; // For public get_mesh() function
	Mesh_Data* mesh_meanie; // For public get_mesh() function

	//> Landscape generation. ----------------------------------------
	/** @return a random permutation of { 0,..,n-1 }. */
	static vector<int> random_permutation(int n);
	
	/** @return a vector of possible nuclei based on the momentary board_sq
	 * considering a 0 pointer to denote a square that is still free. */
	vector<QPoint> get_possible_nucleus_list();

	/** Random square respecting the rules for a new nucleus. I.e.:
	 * There is no other square adjeacent neither diagonal nor horizontal
	 * nor vertical.
	 * @return the coveted square or (-1,-1) if impossible. */
	QPoint get_random_sq_eligible_for_new_plateau_nucleus();
	
	/** @return a vector with all board coodinate pairs pointing at
	 * a Square holding the given plateau_id. */	
	vector<QPoint> get_plateau_positions(int plateau_id);
	
	/** @return true if and only if
	 *   1. the target square is yet a 0 pointer AND
	 *   2. all neighbours are either 0 pointers or squares sharing
	 *      the given plateau id. */
	bool is_valid_new_plateau_square(QPoint pos, int plateau_id);
	
	/** Determines the size of the given plateau in squares. */
	int get_plateau_size(int plateau_id);
	
	/** Applies a cubic polynomial to the task of getting n values of altitudes 
	 * where there are first some vales, then more intermediate plateaus and
	 * increasingly scarse peak regions.
	 * @param int max_altitude: Maximum altitude. The resulting vector will
	 *   contain both this value and the 0.
	 * @param int n: Number of altitude values to be drawn from the polynomial.
	 * @param float x0: Left border. Should be <0. The closer to zero the
	 *   lower the run of many altitudes in the same region. Suggest -.3 or less.
	 * @param float xn_1: Right border. Should be >0. The closer to zero the
	 *   higher the run of many altitudes in the same region. if -x0==xn_1
	 *   the run will be around (max_altitude/2). Suggest .5 or larger.
	 * @return an ascendingly ordered vector<int> with n elements in
	 *   [0,max_altitude] fitted using a cubic polynomial with f(x0)=0 and
	 *   f(x_{n-1})=max_altitude and the indecies from 0 to n-1 are mapped
	 *   equidistantly to the interval [x0,x_{n-1}]
	 * 
	 * Note: The polynomial in question. Let A=max_altitude, x1=xn_1, k in {0,..,n-1}
	 * 
	 * f(k) = A*\frac{[-x0+\frac{k}{n-1}(x0+x1)]^3+x0^3} {x0^3+x1^3}
	 */
	vector<int> get_altitudes_by_cubic_polynomial(int max_altitude, int n, float x0, float xn_1);
	
	/** It is possible that get_altitudes_by_cubic_polynomial(..) returns 
	 * a vector with gaps like [1,3,3,5]. Such gaps endanger the solvability 
	 * of a level and need to be removed. This function does this.
	 * Note: This function depends on the altitudes being presented in
	 * ascending order. */
	void remove_altitude_gaps(vector<int>& altitudes);

	/** Requires sq!=0. Sets the square's type depending on x,y coordinates
	 * on the board. The resulting type will be ODD or EVEN. */
	void set_square_type_depending_on_xy(int x, int y, Square* sq);
	
	/**
	 * @param QPoint pos: Target board coordinates on the board.
	 * @return am 8-sized map key => val where key is a shortcut out of
	 * { 'p_', 'pp', '_p', 'mp', 'm_', 'mm', '_m', 'pm', '__' } where each
	 * the first character denotes the neighborhood in x direction
	 * (positive 'p', negative 'n' or identical '_') and the second
	 * does the same for the neighborhood in y direction. Thus
	 * 'pm' signifies the square at (x+1,y-1) if pos==(x,y).
	 * val of course is a pointer to said neighbor or 0 if non-existing
	 * or UNDEFINED or CONNECTION.
	 */
	map<string,Square*> get_adjacent_odd_even_squares(QPoint pos);

public:
	/** Tool function for distribute_objects() and Game::hyperspace_jump.
	 * @return vector<QPoint> with the QPoints to flat squares with one of
	 * the required altitude values. */
	vector<QPoint> get_coordinates_by_altitude(int min_altitude=0, int max_altitude=-1);

	/** @return a random free square from the given vector or (-1,-1) if that fails. */
	QPoint pick_initially_free_random_square(vector<QPoint>& squares_by_height);
	
private:	
	/** Picks an empty square of the desired height and places a freshly created
	 * Figure of this type there. Also adds a copy to initial_board_fg.
	 * @param QPoint& site: will be filled with the square that has been chosen
	 *   or (-1,-1) if that failed.
	 * @return true if this was possible. false else. */
	bool place_figure_on_free_random_square(E_FIGURE_TYPE type, Mesh_Data* mesh,
		vector<QPoint>& squares_by_height, QPoint& site);
	
	/** Step 1: Dependent on age a number of plateau nuclei is chosen.
	 * Note: This function depends on minimum dimensions >4. >4 ensures that
	 * there are at least 2 nuclei.
	 * @return the board coordinates of the actually generated nuclei. */
	vector<QPoint> generate_nuclei();
	
	/** Step 2 and 3: Expand nuclei into plateaus.
	 * @param int n: The number of nuclei from generate_nuclei().
	 * @param bool neglect: If false all plateaus will take turn in expanding
	 *   leading to roughly equal-sized plateaus that reliably fill out the board.
	 *   Setting it to false will have some plateaus to drop out of the process
	 *   after some time stopping their growth. However this may lead to invalid
	 *   maps. The gospel: First call this function with neglect==true, then
	 *   call it again with neglect==false. */
	void expand_nuclei(int n, bool neglect);
	
	/** Step 4: Assign height values. 
	 * Also makes certain that the lowest vales are larger than 1 square.
	 * @param vector<QPoint> nuclei: Nuclei sites as returned by generate_nuclei().
	 * @return map plateau_id => assigned altitude value.
	 */
	map<int,int> assign_altitudes(vector<QPoint>& nuclei);
	
	/** Step 5: Bridge equi-altitude gaps.
	 * It may happen that palteaus are adjacent to each other sharing the
	 * same height yet being disconnected by 'flat' rock CONNECTION tiles.
	 * If that happens the rock connection tile is turned into a regular
	 * tile (at this time it is a 0 pointer anyways). */
	void bridge_equi_contour_plateaus();
	
	/** Step 6: Assign the ODD-EVEN information and the appropriate Mesh_Data*
	 * for all flat squares. Assumes that CONNECTION squares are still 0 pointers. */
	void assign_odd_even();
	
	/** Step 7: Generate connection pieces. 
	 * At this point it is ensured that every corner of a slope piece
	 * has contact to at least one flat square with a well-defined height.
	 * Everything else would constitute a bug. */
	void assign_slopes();
	
	/** Step 8: Distribute objects. Sentinel tower on one of the highest plateaus.
	 * Sentries in the upper third reaches. Trees dominant in the lower regions. */
	void distribute_objects();
	
	/** Step 9: Send Squares to GPU. */
	void send_board_sq_to_GPU();
	 
	/** Intended to be called only once by the constructor. Generates the landscape.
	 * For details see ':/resources/doc/plan.txt'. */
	void generate_landscape();
	//< --------------------------------------------------------------
	
public:
	//> For the convenience of Game::mainfest_figure(..). ------------
	/** Simply returns a random number in {0,..,359}. */
	float get_random_angle();

	/** Convenience getters for Game::mainfest_figure(). */
	Mesh_Data* get_mesh(E_FIGURE_TYPE type);
	
	float get_antagonist_spin_period() { return spin_period; }
	float get_antagonist_fov() { return fov; }
	float get_antagonist_fading_time() { return fading_time; }
	//< --------------------------------------------------------------

	int get_width() { return width; }
	int get_height() { return height; }
	
	int get_peak_altitude() { return this->peak_altitude; }
	Board<Square>* get_board_sq() { return &board_sq; }
	/** Useful when determining the radius for the thunder dome. */
	float get_board_diagonal_length();
	
	/** For Game constructor. Generates a whole new Figure board with
	 * independent copies of the initial_board_fg figures.
	 * The caller of this function is responsible for deletion of the
	 * board. The board itself will be created with
	 * delete_contents_upon_destruction == false. */
	Board<Figure>* get_new_initialized_board_fg();
	
	/** After generation of the landscape this function returns the player's
	 * initial position on the board. 
	 * @throws char* if the starting position is still (-1,-1) (i.e. uninitialized). */
	QPoint get_player_starting_position();
	
	/** Returns this object's random seed. Note that this is meant for level
	 * reconstruction purposes. The seed is only used once while the
	 * constructor of this Landscape runs where it is plugged into qsrand(seed). */
	uint get_seed() { return this->seed; }
	
	/** @return Landscape height at the give square. Returns -1 if the square
	 *   is a CONNECTION not having a fixed altitude or -2 if x,y point
	 *   beyond the board or the Square* in question is 0. */
	int get_altitude(int x, int y);
	
	/**
	 * Initializes this object and calls this->generate_landscape().
	 * @param uint seed: Random seed to be passed on to qsrand(..) setting up qrand().
	 *   qrand() is cool because it is thread-safe. And this class already
	 *   has qt dependencies. May be 0. In that case qsrand() is not called.
	 * @param int width: Width of the board > 3. Such width is needed for the
	 *   sentinel tower, surrounding rock and at least one square of low-land.
	 * @param int height Height of the board > 3.
	 * @param int gravity: Governs the maximum height of the landscape.
	 *   The higher the gravity the flatter the landscape. 0 is strong gravity,
	 *   the higher the value the weaker the field. 5 should stand for weightlessness.
	 * @param int age: Age of the landscape. The younger the more plateau seeds
	 *   will be sewn. 0 ist youngest.
	 * @param float spin_period: Time it takes for an antagonist to turn 360 degrees.
	 * @param float fov: Horizontal field of view for the antagonists.
	 * @param float fading_time: Fading time for the antagonist. For dramatic
	 *   effect this time will be automatically doubled for The Sentinel.
	 * @param int trees: The program will attempt to distribute this many trees among
	 *   each 100 squares of the vales of the growing landscape.
	 * @param int sentries: If possible this number of sentries will be distributed
	 *   among the peaks of the growing landscape.
	 * @param int randomized_spin: If 0 the spin direction for each
	 *   antagonist will be mathematically positive (if viewed from above).
	 *   If 1 it will be negative, if 2 it will be randomized for each individual
	 *   antagonist.
	 * @param IoQt* io: Such a complicated object should have access to an Io_Qt*
	 *   pointer for debugging message output.
	 * @param Mesh_Data* mesh_connection, mesh_odd, mesh_even, ...: Pointers to
	 *   the mesh data associated with CONNECTION squares, ODD squars and EVEN
	 *   squares. Needed in order to initialize the GL aspects of the squares
	 *   generated for this landscape.
	 */
	Landscape(uint seed, int width, int height, int gravity, int age,
		float spin_period, float fov, float fading_time,
		int trees, int sentries, int randomized_spin,
		Io_Qt* io, Mesh_Data* mesh_connection,
		Mesh_Data* mesh_odd, Mesh_Data* mesh_even, Mesh_Data* mesh_sentinel,
		Mesh_Data* mesh_sentinel_tower,	Mesh_Data* mesh_sentry,
		Mesh_Data* mesh_tree, Mesh_Data* mesh_robot, Mesh_Data* mesh_block,
		Mesh_Data* mesh_meanie);
};
}

#endif
