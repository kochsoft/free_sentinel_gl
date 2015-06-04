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
 * Having decided to install Qt 5.4 locally on my home folder 
 * I can use the modern QOpenGLWidget after all. This class in
 * its structure is inspired by
 * https://blog.qt.io/blog/2014/09/10/qt-weekly-19-qopenglwidget/
 * 
 * The idea is still to have this class in a rather abstract way
 * and generate the actual game content in a more C style like
 * separate file. Well, still gotta learn this openGl stuff from
 * a Qt point of view. So let's see ...
 * 
 * Markus-Hermann Koch, mhk@markuskoch.eu, 07.05.2015
 * 
 * 08.05.2015: Decided to allow hard-coded configuration code within this header file.
 * This appears to be simpler than outsourcing everything to config files.
 * 
 * Sources:
 * https://blog.qt.io/blog/2014/09/10/qt-weekly-19-qopenglwidget/
 * Starting shot for the build-up of this class.
 * 
 * http://doc.qt.io/qt-5/qml-qtquick-shadereffect.html
 * Mappings of Qt-Types to GLSL shader equivalents. E.g.: QVector3D -> vec3
 * 
 * http://doc.qt.io/qt-5/qtopengl-cube-example.html
 * Somewhat obsolete but helpful textured cube example.
 * 
 * http://stackoverflow.com/questions/16387331/gluperspective-equivalent-for-qt-and-opengl
 * Somebody tried to get perspective matrices to run under Qt.
 */

#ifndef MHK_WIDGET_OPENGL_H
#define MHK_WIDGET_OPENGL_H

#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QTimer>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QFocusEvent>
#include <QKeyEvent>
#include "io_qt.h"
#include "data_structures.h"
#include "game.h"

using std::ostream;
using std::ostringstream;
using std::string;
using std::map;
using std::pair;
using std::cout;
using std::cerr;

using namespace mhk_gl;
using namespace game;

namespace display
{
/** Struct giving general fixed displaying information for the openGl scene. */
struct Display_Data
{
	// Resource for the glsl program list.
	string kernel_list_file;
	// rgba color for the background.
	QVector4D backgroundcolor;
	// Passed to glClear(..)
	GLbitfield mask_glClear;
	// glEnable commands.
	vector<GLenum> flags_glEnable;
	// glBlendFunc command 'sfactor'
	GLenum sfactor_glBlendFunc;
	// glBlendFunc command 'dfactor'
	GLenum dfactor_glBlendFunc;
	
	/** Default constructor setting sensible defaults. */
	Display_Data() :
		kernel_list_file(":/misc/kernels.txt"),
		backgroundcolor(0.0, 0.0, 0.2, 1.0),
		mask_glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT),
		sfactor_glBlendFunc(GL_SRC_ALPHA),
		dfactor_glBlendFunc(GL_ONE_MINUS_SRC_ALPHA)
	{
		flags_glEnable.push_back(GL_BLEND); // Allow for transparency.
		flags_glEnable.push_back(GL_DEPTH_TEST); // Closer pixels obstruct farther pixels.
		flags_glEnable.push_back(GL_MULTISAMPLE); // Smooth the edges.
		flags_glEnable.push_back(GL_CULL_FACE); // Only see the top of faces.
	}
};

/** Used by Widget_OpenGl::load_textures() in order to fill Widget_OpenGl::textures */
struct Known_Texture_Resources
{
	map<string,string> textures;

	/** Default constructor setting sensible defaults. */
	Known_Texture_Resources();
};

class Widget_OpenGl : public QOpenGLWidget, public QOpenGLFunctions
{
Q_OBJECT

signals:
	/** Replace the text segment within the status bar. */
	void update_parent_statusBar_text(QString text);
	/** To be connected to Form_main::toggle_fullscreen(). */
	void fullscreen_key_pressed();
	/** Request to disable the "New Game" option in the main form.
	 * It is sent by a failure in initializeGL and effectively
	 * admits defeat: Sentinel GL will not run in this environment. */
	void disable_program(QString msg);
	/** If the user presses Esc signal this. */
	void exit_requested();
	/** Request to start a new game depending on the setup form settings.
	 * Note that the Game object will see to it, that the setup form is updated
	 * if a campaign level was won. */
	void request_new_game();
	void request_restart_game();

private:
	// Will be set by initializeGL().
	bool initializeGL_ok;
	/** Should the openGL scene be repainted? Set this to true whenever
	 * you feel paintGL() should run.  paintGL() is the only function
	 * that will set do_repaint=false after it has run. */
	bool do_repaint;
	// If the game is paused this->timer_framerate will not act.
	// More precisely: update_after_dt() will skip its action.
	bool is_auto_paused;
	// A strong pause mode requested by the user. Needs to be deactivated by the user.
	bool is_user_paused;
	// Active scenery setting.
	E_SCENERY scenery;
	
	Io_Qt* io;
	static Display_Data default_display;
	Known_Texture_Resources known_texture_resources;

	/** Filled by compile_programs using the resource ":/misc/kernels.txt" */
	map<string,QOpenGLShaderProgram*> programs;
	/** Shader pointers created by compile_programs(). Needed for deletion later on. */
	vector<QOpenGLShader*> shaders;
	/** Texure objects by resource their key in this->known_texture_resources. */
	map<string, QOpenGLTexture*> textures;
	/** All kinds of 3D objects required for the game. */
	map<string,Mesh_Data*> objects;
	/** How many frames per seconds should be endeavoured? */
	float framerate;
	/** Where in space is the light source? */
	QVector3D light_position;
	/** Initialized with (1,1,1,1) events like pause or attack may alter this
	 * vector. It will be multiplied to light_color prior to GPU upload. */
	QVector4D light_filtering_factor;
	/** What color has the light on this planet without any modifiers? */
	QVector4D light_color;
	/** Brightness factor. Defaults to (1,1,1,1). */
	QVector4D light_brightness;
	/** How much minimum luminescence will have surfaces not hit by light source? */
	float light_ambience;
	/** Pointer to the game object. */
	Game* game;
	
	// Is triggered every 1/framerate seconds. Calls this->update_after_dt() causing repaintGL()
	QTimer* timer_framerate;
	
	/** Updates the statusBar with a pause message. */	
	void display_pause_game();
	
	/** Is the game paused? true if either pause indicator is true. */
	bool is_paused();
	
	/** Employs this->game in order to update the actual zoom factor by
	 * given dFOV. Makes sure that player max and min opening are maintained. */
	void update_zoom(float dFOV);

public:
	void set_io(Io_Qt* io) { this->io = io; }
	void set_game(Game* game) { this->game = game; }
	void set_scenery(E_SCENERY scenery) { this->scenery = scenery; }

	/** Setting up light source, color and rest light ambience.
	 * @param QVector4D light_color: Dominating light color. Every rendering
	 *   within this game will be multiplied by this color.
	 * @param QVector3D light_position: Light position for diffuse lighting.
	 * @param float light_ambience: Minimum light intensity for surfaces in
	 *   complete shadow. In [0,1]. */
	void setup_light_source(QVector4D light_color=QVector4D(1,1,1,1),
		float light_ambience=DEFAULT_LIGHT_AMBIENCE,
		QVector3D light_position=QVector3D(-1,-1,1));
	
	Game* get_game() { return this->game; }
	float get_framerate() { return this->framerate; }
	Mesh_Data* get_mesh_data_connection();
	Mesh_Data* get_mesh_data_odd();
	Mesh_Data* get_mesh_data_even();
	Mesh_Data* get_mesh_data_sentinel();
	Mesh_Data* get_mesh_data_sentinel_tower();
	Mesh_Data* get_mesh_data_sentry();
	Mesh_Data* get_mesh_data_tree();
	Mesh_Data* get_mesh_data_robot();
	Mesh_Data* get_mesh_data_block();
	Mesh_Data* get_mesh_data_meanie();

	/** Hash keys for Mesh_Data, textures, lighting colors as well as some
	 * file names bear a code string denoting to which scenery they belong.
	 * This function offers a centralized facility to get these keys right.
	 * @param string prefix: Will be prefixed to the result. If not empty
	 *   a "_" will be used for the connection.
	 * @param E_SCENERY scenery: Scenery setting the suffix will depend upon.
	 * @param string suffix: Will be suffixed to the result.
	 * @return The complete scenery string. E.g.: "sky_europe.png"
	 *   for prefix=="sky", scenery==EUROPE, suffix=".png" */
	static string get_scenery_resource_string(
		string prefix, E_SCENERY scenery, string suffix="");
	
	//> Constructor, destructor. -------------------------------------
    Widget_OpenGl(QWidget* parent=0, Qt::WindowFlags flags=0);
	virtual ~Widget_OpenGl();

private:
	//> initializeGL() helper functions. -----------------------------
	/** Helpers for compile_programs(..) */
	map<string,string> get_glsl_src_by_shader_type(const map<string,string> raw_config, string type);
	bool compile_single_shader(QOpenGLShader* shader, string src, string name, ostringstream& log);
	bool compile_single_program(QOpenGLShaderProgram* program, string program_name, string src_vertex, string src_fragment);

	/** Fills this->programs using the resource ":/misc/kernels.txt"
	 * After a successful call the glsl programs are compiled and ready to use.
	 * @return true if and only if all went well. */
	bool compile_programs();

	/** Fills this->textures with content.
	 * Draws upon this->known_texture_resources.
	 * Note: A key starting with "v_" or with "h_" has load_textures
	 * flipping the texture vertically or horizontally.
	 * @return true.
	 */
	bool load_textures();

	/** Helper function for initialize_objects() */
	Mesh_Data* build_standard_object(string pfname_obj, string program_name,
		QOpenGLTexture* texture, bool do_transfer_to_GPU);
	
	/** Call after compile_programs and load_textures have run.
	 * Prepares the 3D objects for the game and fills this->objects.
	 * @return true.
	 */
	bool initialize_objects();
	
	/** Applies the content of this->default_display to the context. */
	void set_context_to_default_state();
	//< --------------------------------------------------------------
	//> Helper functions for paintGL(). ------------------------------
	/** Maps fade in [0,1] to a goofy fct in [0,1.3].
	 * @returns float that can be used in scale while drawing objects
	 *   for a nice optical plop effect. Apply it on MANIFESTING figures. */
	float plop_fct(float fade);
	
	/** Clears the screen using default settings*/
	void clear_screen(QVector4D color=default_display.backgroundcolor);
	
	/** Retrieve Non-null Player_Data pointer from this->game->player. 
	 * @throws char* if pd cannot be retrieved or is 0. */
	Player_Data* get_player_data();

	/** Retrieve Non-null Viewer_Data pointer from this->game->player. 
	 * @throws char* if vd cannot be retrieved or is 0. */
	Viewer_Data* get_viewer_data();
		
	/** Draws an object using the terrain shaders.
	 * @param Mesh_Data* object: Pointer to the mesh data for the object to be drawn.
	 * @param QMatrix4x4& camera: Final transformation to be applied to
	 *   the vertices prior to drawing. Consists of (perspective*lookAt).
	 * @param QMatrix4x4& trans_rot: May be the identity matrix if the target
	 *   mesh is in world coordinates already. Else (translation*rotation)
	 *   transformation to be applied prior to the camera transformation.
	 *   This matrix is also needed for the diffuse lighting algorithm.
	 * @param float fade: Alpha channel fading factor in [0,1].
	 * @param QOpenGLBuffer* buf_vertices: If given use these vertices instead
	 *   of the ones already present in Mesh_Data. This is useful for
	 *   this->draw_square() with its world coordinate vertex set.
	 *   May be 0 in order to use the buffer within Mesh_Data.
	 */
	void draw_terrain_object(Mesh_Data* object, QMatrix4x4& camera,
		QMatrix4x4& trans_rot_object, float fade, QOpenGLBuffer* buf_vertices=0);

	/** Basically just calls draw_terrain_object using the world coordinate
	 * vertex buffer within sq. */
	void draw_square(Square* sq, QMatrix4x4& camera, float fade);

	/** Draws the thunderdome. I.e. the sky and the flat plain beneath it. 
	 * fade is the value of the alpha channel. */
	void draw_dome(QMatrix4x4& camera, float fade);
	
	/** Tool function for draw_landscape(). Determines the scale for the given figure. */
	float get_appropriate_scale(Figure*);
	
	/** Draws the landscape based on this->game->get_landscape(). */
	void draw_landscape(float fade);
	//< --------------------------------------------------------------
	/** @return mouse coordinates on the glScreen in [-1,1].
	 * Will be limited to that interval even if the mouse is elsewhere if clamp==true. */
	float get_Gl_mouse_x(bool clamp=true);
	float get_Gl_mouse_y(bool clamp=true);
	
	/** If the player is not in cursor mode this will handle his dynamic rotation. 
	 * @param float framerate: dt=1./framerate. Required for the update of viewer_data.
	 * @param float center: If Gl_mouse_x and Gl_mouse_y are in [-center,center]
	 *   it is assumed that the mouse is so very close to the center of the screen
	 *   that the player wishes not to rotate at all. */
	void player_dynamic_rotation(float framerate, float center=.15);
	
	/** Will mutliply this->light_brightness by the given factor.
	 * Special case: factor==1.0. In that case light_brightness will be
	 * reset to (1,1,1,1). */
	void modify_brightness(float factor=1.0);
	
protected:
	//> Qt event handling functions I have overwritten. --------------
	/** Called once automatically after the GL context was created. */
	virtual void initializeGL();

	virtual void resizeGL(int width, int height);
    virtual void paintGL();
	
	virtual void mouseReleaseEvent(QMouseEvent* e);
	virtual void wheelEvent(QWheelEvent*);
	virtual void keyPressEvent(QKeyEvent*);
	
	virtual void enterEvent(QEvent*);
	virtual void leaveEvent(QEvent*);
	//virtual void focusInEvent(QFocusEvent*);
	//virtual void focusOutEvent(QFocusEvent*);
	//< --------------------------------------------------------------

public slots:
	/** Automatically called by this->timer_framerate. Updates data_view and
	 * causes this->update() to be called ultimately causing this->paintGL(). */
	void update_after_dt();
	/** Simply sets this->do_repaint = true. */
	void request_paintGL();
	/** Multiply new color vector to light_filtering_factor.
	 * @param QVector4D color: The new color to be added, substracted
	 *   or replacing the old one. Depending on action.
	 * @param int action:
	 *   0: Additive calculation.
	 *   1: Substractive calculation.
	 *   2: Replacement of old this->light_filtering_factor.
	 */
	void set_light_filtering_factor(QVector4D color=QVector4D(1,1,1,1),
		int action=0);
};

}
#endif
