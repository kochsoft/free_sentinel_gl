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
 * The growing number of data structures within widget_openGl.* makes
 * this program more and more unwieldy. Besides some of the structures
 * will be shared by other classes (landscape will have need of access to
 * vertex data for instance). Hence today I decided to do a clean-up
 * session and move some of the more content-minded structures here.
 * 
 * Markus-Hermann Koch, mhk@markuskoch.eu, 15/05/12/.
 */

#ifndef MHK_DATA_STRUCTURES_H
#define MHK_DATA_STRUCTURES_H

#include <vector>
#include <string>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QVector4D>
#include <QVector3D>
#include <QVector2D>
#include "config.h"
#include "io_qt.h"
#include "form_game_setup.h"

using std::vector;
using std::string;

using namespace mhk_gl;

namespace display
{
/** If you ever change this enum pls also update
 * All_Sceneries::number_of_known_sceneries().
 * TODO: Is there really no better way to determine the size of an enum? */
enum E_SCENERY { MASTER, EUROPE, SELENE, MARS, ASTEROID };
enum E_VISIBILITY { HIDDEN, PARTIAL, FULL };

/** Helper to iterate over all possible sceneries. */
class Known_Sceneries
{
private:
	vector<E_SCENERY> sceneries;
	
public:
	/** Getter for a pointer at this->sceneries. */
	vector<E_SCENERY>* get_sceneries() { return &(this->sceneries); }
	/** @return true if and only if the given scenery may be found in this->sceneries. */
	bool is_supported(E_SCENERY scenery);
	/** @return the size E_SCENERY in elements. */
	static int number_of_known_sceneries() { return 5; }
	/** Standard constructor adding all supported sceneries to this->sceneries. */
	Known_Sceneries()
	{
//		sceneries.push_back(E_SCENERY::MASTER); // TODO: Decomment these lines when ready.
		sceneries.push_back(E_SCENERY::EUROPE);
//		sceneries.push_back(E_SCENERY::SELENE);
//		sceneries.push_back(E_SCENERY::MARS);
//		sceneries.push_back(E_SCENERY::ASTEROID);
	}
};

/** An antagonist always acts on the top of any stack. Visibility is relative to that. */
struct Antagonist_target
{
	E_VISIBILITY visibility;
	QPoint board_pos;
	
	Antagonist_target(E_VISIBILITY visibility, QPoint board_pos)
		{ this->visibility = visibility; this->board_pos = board_pos; }
};

/** In openGL a vertex sports
 * + its 4D coordinates,
 * + for some reason a 3D normal vector which in this project will be
 *   associated with the single face the vectex takes part in.
 * + its 2D texture coordinates. */
struct Vertex_Data
{
	QVector4D vertex;
	QVector3D normal;
	QVector2D tex_coord;
	QVector4D color;
	Vertex_Data(
		QVector4D vertex=QVector4D(0,0,0,1),
		QVector3D normal=QVector3D(0,0,1),
		QVector2D tex_coord=QVector2D(0,0),
		QVector4D color=QVector4D(1,1,1,1)
	) : vertex(vertex), normal(normal), tex_coord(tex_coord), color(color) {}
	string toString();
};

// Remember that &(vector[0]) gets you the array pointer and have a look
// at http://doc.qt.io/qt-5/qtopengl-cube-geometryengine-cpp.html
class Mesh_Data
{
private:
	Io_Qt* io;

	/** Dismantles a blender obj index string into its components as a vector.
	 * e.g.: "15/3/6" -> [15,3,6] */
	vector<GLushort> obj_index_to_vector(const string obj);

public:
	vector<Vertex_Data> vertices;
	// Fill elements according to the choice made in this->draw_mode.
	vector<GLushort> elements;
	// Needed for glDrawElements(..) later on.
	GLenum draw_mode;
	
	string program_name;
	
	QOpenGLTexture* texture;
	
	// Large buffer inspired by
	// http://doc.qt.io/qt-5/qtopengl-cube-geometryengine-cpp.html
	// One array to hold vertices, normals and tex_coords at once.
	QOpenGLBuffer buf_vertices;
	// More traditional buffer holding elements.
	QOpenGLBuffer buf_elements;

	/** Parses a blender created wavefront .obj file content into this Mesh_Data.
	 * Afterwards this->vertices and this->elements will be defined.
	 * @param string src: Plain src from the obj file. Needs to be triangulated,
	 *   hold normals and texture coordinates.
	 * @return true if and only if all went fine. */
	bool parse_blender_obj(string src);

	/** Call this after this->vertices and this->elements are defined.
	 * This command will then define buf_vertices and buf_elements and
	 * shove them to the GPU. */
	bool transfer_vertices_and_elements_to_GPU();
	
	/** Binds buf_vertices, buf_elements, and texture (if available).
	 * @return true if and only if all non-texture bindings succeed. */
	bool bind();
	
	/** Releases all the resources bound by bind(). */
	void release();

	/** Simple constructor setting defaults. */
	Mesh_Data(Io_Qt* io_=0);
	~Mesh_Data();
};

class Viewer_Data : public QObject
{
Q_OBJECT

signals:
	void viewer_data_changed();

private:
	// 2*pi/360.
	const float deg_to_radians;
	
	// Position of the viewer in World space where (0,0,0) is
	// the center of the base of the thunderdome and (0,0,1) is up.
	QVector3D site;
	// Direction of view.
	QVector3D dir_view;
	// Much like the lookAt(..) equivalent. Vector in the span
	// of the "up" direction and the "view direction". Should Not
	// be collinear with dir_view.
	QVector3D dir_up;
	// Perspective "near" distance.
	float near;
	// Perspective "far" distance.
	float far;
	// Perspective vertical angle of opening. Qt-compatibly in degrees.
	float opening;

	// Needed for the apsect ratio.
	QOpenGLWidget* parent;
	
public:
	float get_phi();
	float get_theta();
	QVector3D get_direction();
	QVector3D get_site();
	float get_opening() { return opening; };
	/** Main setter for this attribute. Signals viewer_data_changed(). */
	void set_opening(float opening);
	
	/** Sets this object's direction vectors for dir_view and dir_up.
	 * All angles are given in degrees.
	 * @param float phi: Lives in the flat plane. phi=0 means the direction (1,0,*)
	 * @param float theta: Spherical theta angle. theta=0 is straight up,
	 *   theta=180 is straight down.
	 * @param float alpha: Rotation of the axis of view. For theta in (0,180)
	 *   alpha=0 means that (0,0,1) is in the span of "up-direction" and
	 *   "view-direction". This vector is turned around the view-direction
	 *   by the angle alpha. Should theta hit 0 or 180 this step will be skipped
	 *   and (-1,0,0) will be set.
	 */
	void set_direction(float phi, float theta, float alpha=0);
	/** Main setter for this attribute. Signals viewer_data_changed(). */
	void set_dir_view(QVector3D dir_view);
	/** Main setter for this attribute. Signals viewer_data_changed(). */
	void set_dir_up(QVector3D dir_up);
	
	/** Updates this objects dir_view and dir_up in such a way that it reflects
	 * the changes in phi+=dt*dphi, theta+=dt*dtheta and alpha+=dt*dalpha*/
	void update_direction(float dt, float dphi, float dtheta, float dalpha=0);
	
	/** 180 degrees around the z-axis. */
	void turn_around_z_axis();
	
	/** Main setter for this attribute. Signals viewer_data_changed(). */
	void set_site(QVector3D site);
	
	/** Updates this objects site vector by site += dt*ds */
	void update_site(float dt, QVector3D ds);
	
	/** Sets this object's perspective data. */	
	void set_perspective(float near, float far, float opening);
	
	/** Updates this object's perspective data. 
	 * @param float dt: size of the time step in seconds.
	 * @param float dnear: size of the change in 'near' if dt were 1s.
	 * @param float dfar: size of the change in 'far' if dt were 1s.
	 * @param float dopening: size of the change in 'opening' if dt were 1s. */
	void update_perspective(float dt, float dnear, float dfar, float dopening);

	/** Sets viewer data. */
	void set_viewer_data(QVector3D site, QVector3D direction, float opening);
	/** Updates viewer_data after a transfer. */
	void update_viewer_data_for_transfer(QPoint old_site, QPoint new_site,
		float old_altitude, float new_altitude, float opening);
	
	/** Horizontal field of view in degrees. Nice source:
	 * http://gamedev.stackexchange.com/questions/43922/opengl-fovx-question */
	float get_fov_h();
	
	/** Returns the apsect ratio width/height. The value taken from the parent
	  * QOpenGLWidget dimensions if available. 640/480 else. */
	float get_aspect();
	
	/** @return the matrix A := perspective*lookAt based on this object's data. */
	QMatrix4x4 get_camera();

	string toString();
	
	/** Constructor for a new View_Data Object. */
	Viewer_Data(QOpenGLWidget* parent, QVector3D site, float phi, float theta,
		float alpha, float near, float far, float opening);
	Viewer_Data(QOpenGLWidget* parent);
};

class Player_Data : public QObject
{
Q_OBJECT

signals:
	void cursor_mode_switched(bool new_cursor_mode);

private:
	/** Pointer to io object for debugging messages. */
	Io_Qt* io;
	/** Viewer_Data object describing the position of the eye of the player.
	 * The eye should always be	situated 1.5 squares above the square
	 * the player is standing on. */
	Viewer_Data* viewer_data;

	/** Is the player in roaming or in cursor mode? */
	bool cursor_mode;
	/** Position of the player on the board. */
	QPoint site;
	/** Position of the robot the player has last left in board
	 * coordinates or (-1,-1) if he is in the initial robot. */
	QPoint former_site;
	/** Maximum FOV (field of view) in degrees. */
	float opening_max;
	/** Default player GL FOV in degrees. */
	float opening_default;
	/** Minimum FOV in degrees. */
	float opening_min;
	/** Maximum inverse rotation speed in s/360 degrees
	 * (i.e. the time needed for one full player rotation). */
	float self_spin;
	/** Psi shield max in seconds. */
	float psi_shield_max;
	/** Remaining psi shield energy in seconds */
	float psi_shield_val;
	/** Confidence max in seconds. */
	float confidence_max;
	/** Remaining confidence in seconds */
	float confidence_val;
	/** Energy units in the player's inventory. */
	int energy_units;
	
	/** To be set to true under attack und to false after peace was restored. 
	 * Being seen partially is considered a light attack. Being seen fully
	 * is considered a heavy attack. */
	bool under_heavy_attack;
	bool under_light_attack;
	
public:
	Viewer_Data* const get_viewer_data() { return viewer_data; }
	/** Board position of the player. */
	QPoint get_site() { return site; }
	QPoint get_former_site() { return former_site; }
	float get_opening_max() { return opening_max; }
	float get_opening_default() { return opening_default; }
	float get_opening_min() { return opening_min; }
	float get_self_spin() { return self_spin; }
	float get_psi_shield_max() { return psi_shield_max; }
	float get_psi_shield_val() { return psi_shield_val; }
	float get_confidence_max() { return confidence_max; }
	float get_confidence_val() { return confidence_val; }
	
	/** Reduces defenses by given value. First psi shield then confidence.
	 * @param float val: Value >0 for reduction. Should be 1/framerate.
	 * @return If confidence shrinks below zero true will be returned.
	 * false else.
	 * Note: While it is possible to take several hits in one turn
	 * at the end of the turn reset_confidence should be called. */
	bool take_hit(float val);
	
	/** Resets confidence. Should be called after damage has been taken. */
	void reset_confidence();
	
	/** Reset psi shield and confidence level to maximum.
	 * Should be called after a turn without being under attack. */
	void reset_shields();

	bool get_cursor_mode() { return cursor_mode; }
	/** Sets the new cursor mode if different from the old one.
	 * @signals cursor_mode_switched(..) if the state of cursor_mode has altered. */
	void set_cursor_mode(bool cursor_mode);
	/** Convenience function for switching the cursor mode. */
	void switch_cursor_mode() { set_cursor_mode(!cursor_mode); }
	
	/** Sets a new site. Does not touch viewer_data, however! */
	void set_site(QPoint site);
	
	int get_energy_units() { return energy_units; }
	/** Adds the given value to this->energy_units.
	 * @return the  new value of energy_units. */
	int update_energy_units(int delta_W);
	
	bool is_under_light_attack() { return under_light_attack; }
	void set_under_light_attack(bool val) { under_light_attack = val; }
	bool is_under_heavy_attack() { return under_heavy_attack; }
	void set_under_heavy_attack(bool val) { under_heavy_attack = val; }
	
	/** Sets up a new player data object based on game setup data.
	 * @param QOpenGLWidget* parent: Passed on to viewer_data.
	 */
	Player_Data(QOpenGLWidget* parent, Setup_game_data* setup, Io_Qt* io,
		QPoint site,
		float opening_min,
		float opening_default,
		float opening_max);
	~Player_Data();
};

}

#endif
