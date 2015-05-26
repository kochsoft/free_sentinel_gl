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
 * Scanner class for solving
 * 1.) the problem of which square, boulder, or robot is closest to the
 *   mouse pointer if a scan is initiated.
 * 2.) the problem of which antagonist sees which square, boulder, or robot
 *   depending on landscape and his momentary view settings.
 * 
 * This class is absolutely crucial to the game. I hope, I get it running!
 * 
 * Markus-Hermann Koch, mhk@markuskoch.eu, 18.05.2015
 */

#ifndef MHK_SCANNER_H
#define MHK_SCANNER_H

#include <vector>
#include <map>
#include <QVector3D>
#include <QMatrix4x4>
#include "data_structures.h"
#include "landscape.h"
#include "io_qt.h"

using std::vector;
using std::multimap;

using namespace display;

namespace game
{
struct QPoint_Figure
{
	QPoint pos;
	Figure* fig;
	QPoint_Figure(QPoint pos=QPoint(-1,-1), Figure* fig=0)
		{ this->pos = pos; this->fig = fig; }
};

class Scanner
{
private:
	Io_Qt* io;
	
	/** The determinant of a 2x2 matrix; (m11*m22 - m12*m21) */
	static float det_2x2(float m11, float m12, float m21, float m22);
	
	/** Consider the angle between a and b.
	 * @return true if and only if that angle is larger than min_angle in degrees. */
	static bool angle_larger_than(QVector2D a, QVector2D b, float min_angle=1);
	
	/** First checks whether the angle between (in11,in21) and (in12,in22)
	 * exceeds min_angle. If not returns false. Else returns true
	 * and fills the out values with the inverse matrix to the in values.
	 * Note that
	 * [a b]^-1    [  d -b]
	 * [c d]    =  [ -c  a]*1/det(Input matrix).
	 */
	static bool inv_2x2(float in11, float in12, float in21, float in22,
		float& out11, float& out12, float& out21, float& out22,
		float min_angle=1.);
	
	/** Returns the normalized direction vector the mouse is pointing at. */
	QVector3D get_mouse_direction(float mouse_gl_x, float mouse_gl_y, Viewer_Data* vd);
	
	/** @return the board position under the point of the given QVector3D. */
	static QPoint get_board_pos_from_QVector3D(QVector3D&);
	
	/** Given three vectors in screen coordinates a, b and c which
	 * are fresh from (camera*vertex) this function determines whether
	 * or not the given mouse coordinates are within the triangle
	 * spanned by these three vectors.
	 * @param float mouse_gl_x, mouse_gl_y: Mouse coordinates in [-1,1]^2.
	 * @param QVector4D a,b,c: Let v0,v1,v2 be the respective vectors
	 *   in world coordinates. Then
	 *     a = camera*v0; a /= a.w();
	 *     b = camera*v1; b /= b.w();
	 *     c = camera*v2; c /= c.w();
	 * @return true if the mouse was found in the triangle, false else.
	 */
	static bool is_in_triangle(float mouse_gl_x, float mouse_gl_y,
		QVector4D a, QVector4D b, QVector4D c);

	/**
	 * @param Figure* figure: Pointer to the target figure. Contains
	 *   the phi angle required to build the complete transformation.
	 * @param QPoint board_pos: (x,y) board coordinates of said figure.
	 * @param int altitude_square: Altitude of the board square the figure
	 *   stands on.
	 * @param QMatrix4x4& camera: Camera transformation from Viewer_Data.
	 * @return the complete transformation
	 *   A := perspective * lookAt * translation * rotation
	 * for the given figure. I.e.: The mapping of the prototype vertecies
	 * into the final to-display-[-1,+1]^3 openGL cube.
	 * Note that no similar function for Squares is needed since these
	 * are stored in world coordinates already! Hence for them camera suffices.
	 */
	QMatrix4x4 get_complete_transformation_for_figure(Figure* figure,
		QPoint board_pos, int altitude_square, const QMatrix4x4& camera);

	/** Checks whether or not the given figure may be found under the
	 * given mouse coordinates ignoring line of sight obstructions.
	 * @param float mouse_gl_x, mouse_gl_y: Mouse coordinates in [-1,1]^2.
	 * @param Figure* figure: Target figure to be analyzed for mouse contact.
	 * @param QMatrix4x4& complete_transformation: As returned by
	 *   this->get_complete_transformation_for_figure(); i.e. the comprehensive
	 *   matrix that transforms the prototype vertices of the given figures into
	 *   the final [-1,1]^3 openGL cube.
	 * @return true if and only if the target figure may be found under the mouse. */
	bool is_figure_under_xray_mouse(float mouse_gl_x, float mouse_gl_y,
		Figure* figure, const QMatrix4x4& complete_transformation);

	/**
	 * @param float mouse_gl_x, mouse_gl_y: Mouse coordinates in [-1,1]^2.
	 * @param Square* square: Square object pointer.
	 * @param QMatrix4x4& camera: Camera transformation mapping the world-
	 *   coordinate vertices of the square into the [-1,1]^3 openGL cube.
	 * @return true if and only if the mouse is over the given square.
	 *   Note: This function does _not_ consider line of sight obstruction.
	 *   It also does _not_ consider whether or not the square is viewed
	 *   from above or below. */
	bool is_square_under_xray_mouse(float mouse_gl_x, float mouse_gl_y,
		Square* square, const QMatrix4x4& camera);

	/** Lower part of get_all_board_positions_in_line() requiring only
	 * eye position and viewer direction in world coordinates. */
	vector<QPoint> get_all_board_positions_in_line(
		QVector3D eye, QVector3D dir, int width, int height);
	
	/** Completely in world coordinates this function builds the candidates
	 * list required for some of the other functions.
	 * @param float mouse_gl_x, mouse_gl_y: Mouse coordinates in [-1,1]^2.
	 * @param Viewer_Data* viewer_data: Needed for the direction of view.
	 * @param int width, height: Board dimensions.
	 * @return a vector, ordered by increasing distance from the eye, of
	 *   those squares on the board, the described line of sight would traverse. */
	vector<QPoint> get_all_board_positions_in_line(
		float mouse_gl_x, float mouse_gl_y, Viewer_Data* viewer_data,
		int width, int height);

public: // Public for the benefit of Game::antagonist_attack()
	/** Turns the given multimap into an ordered vector. */
	vector<QPoint> convert_ordered_multimap_to_vector(multimap<float,QPoint>&);

	/** The big brother of get_all_board_positions_in_line(..) for
	 * an open horizontal field of view. For instance needed for the
	 * antagonist's field of view.
	 *   @param QVector3D eye: The eye of the beholder.
	 *   @param QVector3D direction: View direction in world coordinates.
	 *     Players may retrieve it using this->get_mouse_direction(..).
	 *   @param float h_fov: The horizontal field of view in degrees >=0.
	 * @return a multimap<float,QPoint> where the keys are squared distances from
	 *   the eye position and the values are board coordinates of the
	 *   pertinent squares. The square of the eye itself will _not_ be included.
	 *   There is no game situation when any agent acts on his on square. */
	multimap<float,QPoint> get_all_board_positions_in_h_fov(
		QVector3D eye, QVector3D direction, float h_fov, int width, int height);

private:
	/** Convenience shortcut for player h_fov. */
	multimap<float,QPoint> get_all_board_positions_in_h_fov(
		float mouse_gl_x, float mouse_gl_y, Viewer_Data* viewer_data,
		float h_fov, int width, int height);
	
	/**
	 * @param float mouse_gl_x, mouse_gl_y: Mouse coordinates in [-1,1]^2.
	 * @param vector<QPoint> candidates: Board positions to be considered.
	 *   The return value will be a new vector containing a subset of these
	 *   candidates. Namely the subset that is touched by the mouse.
	 * @param Landscape* landscape: Landscape object holding world coordinate
	 *   vertex data for the candidate board squares.
	 * @param QMatrix4x4& camera: Camera transformation transforming the world
	 *   coordinate square vertices into the [-1,1]^3 openGL cube.
	 * @param bool stop_after_first: If true the returned array will have
	 *   only one element for the process will be halted after the first hit.
	 * @return a shorter version of candidates that only holds those board
	 *   positions as are returned as true by is_square_under_xray_mouse(..).
	 */
	vector<QPoint> restrict_to_squares_under_xray_mouse(float mouse_gl_x,
		float mouse_gl_y, vector<QPoint> candidates, Landscape* landscape,
		const QMatrix4x4& camera, bool stop_after_first=true);
	
	/**
	 * 1.) calls get_all_board_positions_in_line(..) in order to get the
	 *   viable candidate squares.
	 * 2.) calls restrict_to_squares_under_xray_mouse(..) in order to
	 *   find which squares are actually under the mouse.
	 * 3.) Returns the first of the board positions from step (2) or (-1,-1)
	 *   if that vector is empty. It is important for get_figure_under_mouse(..)
	 *   that this function DOES NOT drop CONNECTION squares for CONNECTION
	 *   squares may obstruct the visibility of figures.
	 * @param all but viewer_data: Passed on to restrict_to_squares_under_xray_mouse(..)
	 * @param Viewer_data* viewer_data: Holds eye, direction and fov for
	 *   get_all_board_positions_in_line(..)
	 * @return board position of the square under the mouse which is closest
	 *   to the eye.
	 */
	QPoint get_square_under_mouse(float mouse_gl_x, float mouse_gl_y,
		vector<QPoint>& candidates, Landscape* landscape, Viewer_Data* viewer_data);
	
	/**
	 * @param vector<QPoint>& squares_in_line: As returned by
	 *   get_all_board_positions_in_line(..)
	 * @param QPoint board_pos_under_mouse: square_under_mouse: As returned
	 *   by get_square_under_mouse(..). If there is a square under the mouse
	 *   that is closer to the eye than any candidate figure (real closer --
	 *    it is permissible to be the square the figure is standing on)
	 *   then that square obstructs the view of that candidate figure
	 *   and we do not need to consider said figure any longer.
	 *  @param vector<QPoint>& board_positions_in_line: As returned by
	 *    get_all_board_positions_in_line(..).
	 * @param Board<Figure>* board_fg: Pointer to Figure centererd Board.
	 * @return This function simply iterates through all
	 *   board_positions_in_line and adds all STABLE Figures on that line to the
	 *   returned vector with key==Board postition and value=Figure pointer.
	 *   Note that due to stacking one board position may in fact hold multiple
	 *   figures. 
	 * Note again: FOR PRACTICAL USE-CASE REASONS THIS FUNCTION ONLY RETURNS
	 * STABLE Figure*s. For only these can be interacted with.
	 */
	vector<QPoint_Figure> get_all_stable_figures_in_line(
		QPoint board_pos_under_mouse,
		vector<QPoint>& board_positions_in_line, Board<Figure>* board_fg);
	
	/**
	 * @param float mouse_gl_x, mouse_gl_y: Mouse coordinates in [-1,1]^2.
	 * @param Landscape* landscape: For getting the square altitude.
	 * @param multimap<QPoint,Figure*> candidates returned by
	 *   get_all_stable figures_in_line(..)
	 * @param QMatrix4x4& camera: Enhanced into complete transformation and
	 *   then passed on to is_figure_under_xray_mouse(..)
	 * @return
	 * 1. Let fg=0.
	 * 2. If there are Figures under the mouse consider those figures that are
	 *    on the Square closest to the eye (evaluate keys of candidates).
	 *    Of those stacked Figures pick the bottom one. fg = that figure.
	 * return fg.
	 */
	QPoint_Figure get_figure_under_mouse(float mouse_gl_x, float mouse_gl_y,
		Landscape* landscape, vector<QPoint_Figure>& candidates,
		QMatrix4x4& camera);

	/** @return the number of boulders on this figure, including the figure itself. */
	int get_block_stack_height(Figure* figure);

	/** For the antagonists. Is the line of sight free to the base of the
	 * given target square at the given altitude? It is if for all flat squares
	 * on the way from eye to target the line of sight is above the square.
	 * @param bool can_see_from_below: If false the target is hidden from view
	 *   if it is situated above the eye.
	 */
	bool can_see_square(Landscape* landscape,
		QVector3D eye, QPoint target_sq, float alt_target_sq, bool can_see_from_below);

public:
	/**
	 * Determines where the mouse is
	 * pointing at. Writes its findings into board_pos and figure.
	 * @param float mouse_gl_x, mouse_gl_y: Mouse coordinates in [-1,1]^2.
	 * @param Viewer_Data* viewer_data: Holds stuff like eye position, viewer
	 *   direction, fov, and the camera projection.
	 * @param Landscape* landscape: Landscape object holding the terrain data.
	 * @param Board<Figure>* board_fg: Figure centered board holding all the
	 *   figures on the board.
	 * @param QPoint player_board_pos: Board coordinates of the square the
	 *   player is standing on. All figures on that position will be ignored.
	 *   The mouse is never pointing at the player or any block he may be
	 *   standing on.
	 * @param QPoint& board_pos: Will be filled with the square the mouse
	 *   is pointing at (ODD, EVEN, or CONNECTION) if at any. This respects
	 *   line of sight obstructions by figures! Will be filled with (-1,-1) if
	 *   no square is under the mouse. Exception: A figure!=0 is returned.
	 *   In that case board_pos will be filled with the board_pos of the square
	 *   the target figure is standing on. However, this square need not be under
	 *   the mouse. Consider the note below!
	 * @param Figure*&; Will be filled with a pointer to the closest figure the
	 *   mouse is pointing at or 0 if the figure is obstructed by terrain or if
	 *   the mouse is not pointing at a figure at all. If, however, a figure was
	 *   identified as the mouse target then board_pos will be filled with
	 *   the board_pos of the square the target figure is standing on.
	 * @return Writes its findings into board_pos and figure.
	 * Note: If there is a Figure under the mouse that figure obstructs all
	 * squares that are not the square the Figure is standing on in the first place!
	 * 
	 * Note: There will NOT be a Figure*!=0 if a square is found.
	 * If a figure!=0 is present the returned square will simply be the square
	 * the figure is standing on. This does NOT mean the square is under mouse.
	 * Quite the contrary. If a square is under mouse the figure pointer will
	 * always be 0.
	 */
	void get_mouse_target(float mouse_gl_x, float mouse_gl_y,
		Viewer_Data* viewer_data, Landscape* landscape,
		Board<Figure>* board_fg, QPoint player_board_pos,
		QPoint& board_pos, Figure*& figure);

	/**
	 * Wrapper function for Scanner::get_antagonist_targets(..).
	 * @param QVector3D eye_in_world: Eye position of the antagonist in world coordinates.
	 * @param QVector2D direction_in_world: View direction of the antagonist in
	 *   world coordinates. Note that these specific scan-masters need no z-coord.
	 *   Their vertical fov is assumed to be 180 degrees from pole to pole.
	 * @param float fov_horizontal: Horizontal field of view of this antagonist.
	 * @param Landscape* landscape: Landscape pointer.
	 * @param Board<Figure>* board_fg: Pointer to the figure centered board.
	 * @param bool seek_trees: If true nothing but non-stacked trees will
	 *   be returned. This is useful for meanie summoning.
	 * @return vector of all Antagonist_targets seen by this antagonist right now.
	 */
	vector<Antagonist_target> get_antagonist_targets(
		QVector3D eye_in_world, QVector2D direction_in_world, float fov_horizontal,
		Landscape* landscape, Board<Figure>* board_fg, bool seek_trees=false);
	
	Scanner(Io_Qt* io=0);
};
}

#endif
