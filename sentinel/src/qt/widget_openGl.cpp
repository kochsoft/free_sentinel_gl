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

#include <cmath>
#include <QSurfaceFormat>
#include <QOpenGLShader>
#include <QFileInfo>
#include "widget_openGl.h"
#include "config.h"

using std::stringstream;
using std::istringstream;
using std::endl;

namespace display
{
Display_Data Widget_OpenGl::default_display = Display_Data();

namespace {	bool pause_light_active = false; }
void Widget_OpenGl::display_pause_game()
{
	if (!game) return;
	QVector4D light_pause(.9,.8,1.3,1);
	ostringstream oss;
	if (is_user_paused || is_auto_paused)
	{
		oss << "Chronometer halted by the " <<
			((is_user_paused) ? "synthoid" : "Sentinel") << ".";
		update_parent_statusBar_text(tr(oss.str().c_str()));
		if (!pause_light_active)
		{
			set_light_filtering_factor(light_pause,0);
			pause_light_active = true;
		}
	} else {
		update_parent_statusBar_text(game->get_game_status_string());
		if (pause_light_active)
		{
			// This 'if' admittedly is a hack. At the beginning of the game when
			// the setup window closes the color filter is reset to (1,1,1,1).
			// Then the game unpauses and we would get false colors
			// if substracting the light_pause filter.
			if (light_filtering_factor.x() != 1. || 
				light_filtering_factor.y() != 1. ||
				light_filtering_factor.z() != 1. ||
				light_filtering_factor.w() != 1.)
			{
				set_light_filtering_factor(light_pause,1);
			}
			pause_light_active = false;
		}		
	}
	do_repaint = true; update();
}

bool Widget_OpenGl::is_paused()
{
	return (is_user_paused || is_auto_paused);
}

Mesh_Data* Widget_OpenGl::get_mesh_data_connection()
	{ return objects.at(get_scenery_resource_string("sq_connection",scenery)); }

Mesh_Data* Widget_OpenGl::get_mesh_data_odd()
	{ return objects.at(get_scenery_resource_string("sq_odd",scenery)); }

Mesh_Data* Widget_OpenGl::get_mesh_data_even()
	{ return objects.at(get_scenery_resource_string("sq_even",scenery)); }

Mesh_Data* Widget_OpenGl::get_mesh_data_tree()
	{ return objects.at(get_scenery_resource_string("tree",scenery)); }

Mesh_Data* Widget_OpenGl::get_mesh_data_sentinel_tower()
	{ return objects.at(get_scenery_resource_string("tower",scenery)); }
	
Mesh_Data* Widget_OpenGl::get_mesh_data_sentinel()
	{ return objects.at("sentinel"); }

Mesh_Data* Widget_OpenGl::get_mesh_data_sentry()
	{ return objects.at("sentry"); }

Mesh_Data* Widget_OpenGl::get_mesh_data_robot()
	{ return objects.at("robot"); }
	
Mesh_Data* Widget_OpenGl::get_mesh_data_block()
	{ return objects.at("block"); }
	
Mesh_Data* Widget_OpenGl::get_mesh_data_meanie()
	{ return objects.at("meanie"); }

//> Constructor and Destructor. --------------------------------------
Widget_OpenGl::Widget_OpenGl(QWidget* parent, Qt::WindowFlags flags)
	: QOpenGLWidget(parent, flags)
{
	this->io = 0;
	this->game = 0;
	this->is_auto_paused = false;
	this->is_user_paused = false;
	this->do_repaint = true;
	this->timer_framerate = 0;
	this->framerate = DEFAULT_FRAMERATE;
	this->scenery = E_SCENERY::EUROPE;
	// https://www.opengl.org/archives/resources/faq/technical/depthbuffer.htm
	// First step for depth testing. Also need glEnable(GL_DEPTH_TEST),
	// zNear and zFar clipping planes, and GL_DEPTH_BUFFER_BIT sent to glClear(..).
//	QSurfaceFormat surface_format;
//	surface_format.setDepthBufferSize(DEFAULT_DEPTH_BUFFER_SIZE);
//	this->setFormat(surface_format);
	this->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
	timer_framerate = new QTimer(this);
    connect(timer_framerate, SIGNAL(timeout()), this, SLOT(update_after_dt()));
	setMouseTracking(true);
}

Widget_OpenGl::~Widget_OpenGl()
{
	if (io) io->println(E_DEBUG_LEVEL::VERBOSE, "~Widget_OpenGl()",	"Deleting glsl programs.");
	for (map<string,QOpenGLShaderProgram*>::iterator IT = programs.begin();
		IT != programs.end(); ++IT)
	{
		delete (IT->second);
	}
	if (io) io->println(E_DEBUG_LEVEL::VERBOSE, "~Widget_OpenGl()",	"Deleting glsl shaders.");
	for (vector<QOpenGLShader*>::iterator IT=shaders.begin();IT!=shaders.end();++IT)
	{
		delete (*IT);
	}
	//// Texture deletion is apparently not important:
    //// http://stackoverflow.com/questions/12403688/not-deleting-texture-memory-in-opengl
	//if (io) io->println(E_DEBUG_LEVEL::VERBOSE, "~Widget_OpenGl()",	"Deleting textures.");
	//for (map<string, QOpenGLTexture*>::iterator IT=textures.begin(); IT!=textures.end(); ++IT)
	//  { delete (IT->second); }
	if (io) io->println(E_DEBUG_LEVEL::VERBOSE, "~Widget_OpenGl()",	"Deleting 3D objects.");
	for (map<string, Mesh_Data*>::iterator IT=objects.begin(); IT!=objects.end(); ++IT)
	  { delete (IT->second); }

	if (io) io->println(E_DEBUG_LEVEL::VERBOSE, "~Widget_OpenGl()",	"Deleting framerate timer.");
	if (timer_framerate)
	{
		timer_framerate->stop();
		delete timer_framerate;
	}
	if (io) io->println(E_DEBUG_LEVEL::VERBOSE, "~Widget_OpenGl()",	"Done deconstructing.");
	// Note that this->io was not created by this object. Hence no deletion.
}
//< ------------------------------------------------------------------

//> GLSL program compiling code. -------------------------------------
map<string,string> Widget_OpenGl::get_glsl_src_by_shader_type(const map<string,string> raw_config, string type)
{
	map<string,string> res = Io_Qt::parse_by_subkey(raw_config, type);
	for (map<string,string>::iterator IT=res.begin();IT!=res.end();++IT)
	{
		string key = IT->first;
		if (key.compare("NONE") == 0) continue;
		string pfname = IT->second;
		stringstream ss;
		if (!Io_Qt::get_stringstream_from_QFile(pfname,ss))
		{
			ostringstream oss;
			oss << "No shader src of type '" << type << "' found for program '" << key << "'";
			if (io) io->println(E_DEBUG_LEVEL::WARNING, "Widget_OpenGl::get_glsl_src_by_shader_type(..)", oss.str());
		}
		(IT->second) = Io::read_file(ss);
	}
	return res;
}

bool Widget_OpenGl::compile_single_shader(QOpenGLShader* shader, string src, string name, ostringstream& oss)
{
	bool allGood = true;
	if (src.size()==0) oss << "Source string for a shader in program '" <<
		name << "' appears to be empty." << endl;
	if (!(shader->compileSourceCode(src.c_str())))
	{
		allGood = false;
		oss << "Compilation of a shader for program '" << name <<
			"' failed." << endl << "=== src ===" << endl <<
			src.c_str() << endl << "=== log ===" << endl <<
			shader->log().toStdString().c_str() << endl << "===========" << endl;
	}
	return allGood;
}

bool Widget_OpenGl::compile_single_program(QOpenGLShaderProgram* program,
	string name, string src_vertex, string src_fragment)
{
	bool allGood = true;
	string caller = "Widget_OpenGl::compile_single_program(..)";
	E_DEBUG_LEVEL dl = E_DEBUG_LEVEL::WARNING;
	ostringstream oss;
	QOpenGLShader* shader_vertex = new QOpenGLShader(QOpenGLShader::ShaderTypeBit::Vertex, this);
	shaders.push_back(shader_vertex);
	allGood = allGood && compile_single_shader(shader_vertex, src_vertex, name, oss);
	QOpenGLShader* shader_fragment = new QOpenGLShader(QOpenGLShader::ShaderTypeBit::Fragment, this);
	shaders.push_back(shader_fragment);
	allGood = allGood && compile_single_shader(shader_fragment, src_fragment, name, oss);
	if (allGood)
	{
		allGood = program->addShader(shader_vertex);
		if (!allGood) oss << "Vertex shader adding: " << program->log().toStdString().c_str() << endl;
		allGood = allGood && program->addShader(shader_fragment);
		if (!allGood) oss << "Fragment shader adding: " << program->log().toStdString().c_str() << endl;
	}
	if (allGood)
	{
		allGood = program->link();
		if (!allGood) oss << program->log().toStdString().c_str() << endl;
	}
	//if (allGood) Linking is fine. Binding probably is glUseProgram().
	//{
	//	allGood = program->bind();
	//	if (!allGood) oss << program->log().toStdString().c_str() << endl;
	//}
	if (!allGood) if (io) io->println(dl,caller,oss.str());
	return allGood;
}

bool Widget_OpenGl::compile_programs()
{
	E_DEBUG_LEVEL dl = E_DEBUG_LEVEL::WARNING;
	string caller = "Widget_OpenGl::compile_programs()";
	//>> Loading the source. -----------------------------------------
	map<string,string> raw_config;
	if (!Io_Qt::parse_config_file(default_display.kernel_list_file, raw_config))
	{
		if (io) io->println(dl, caller, "Raw config could not be compiled.");
		return false;
	}
	map<string,string> src_vertex = get_glsl_src_by_shader_type(raw_config, "VERTEX");
	map<string,string> src_fragment = get_glsl_src_by_shader_type(raw_config, "FRAGMENT");
	//<< -------------------------------------------------------------
	//>> Compiling the programs. -------------------------------------
	// Note: It is assumed that every program has a vertex and a fragment shader.
	for (map<string,string>::iterator IT=src_vertex.begin();IT!=src_vertex.end();++IT)
	{
		string key = IT->first;
		string src_vertex = IT->second;
		map<string,string>::iterator IT_F = src_fragment.find(key);
		if(IT_F == src_fragment.end())
		{
			ostringstream oss;
			oss << "No fragment shader src was found for program '" <<
				key.c_str() << "'." << std::endl;
			if (io) io->println(dl, caller, oss.str());
			return false;
		}
		string src_fragment = IT_F->second;
		programs[key] = new QOpenGLShaderProgram(this);
		if (compile_single_program(programs[key], key, src_vertex, src_fragment))
		{
			ostringstream oss;
			oss << "Successfully buildt program '" << key << "'.";
			if (io) io->println(E_DEBUG_LEVEL::VERBOSE, caller, oss.str());
		} else {
			return false;
		}
	}
	//<< -------------------------------------------------------------
	return true;
}
//< ------------------------------------------------------------------

// http://doc.qt.io/qt-5/qopengltexture.html#details
// Apply textures like this: texture->bind(); glDrawArrays(...);
bool Widget_OpenGl::load_textures()
{
	for (map<string,string>::const_iterator CI=known_texture_resources.textures.begin();
		CI != known_texture_resources.textures.end(); ++CI)
	{
		string key = CI->first;
		string pfname = CI->second;
		QFileInfo checkFile(QString(pfname.c_str()));
		if (!(checkFile.exists() && checkFile.isFile())) throw "Texture file not found.";
		QImage image(pfname.c_str());
		if (key.substr(0,2).compare("v_")==0) image = image.mirrored(false,true);
		if (key.substr(0,2).compare("h_")==0) image = image.mirrored(true,false);
		QOpenGLTexture* texture = new QOpenGLTexture(image); //.mirrored());
		texture->setMinificationFilter(QOpenGLTexture::Nearest);
		texture->setMagnificationFilter(QOpenGLTexture::Linear);
		texture->setWrapMode(QOpenGLTexture::Repeat);
		this->textures[key] = texture;
	}
	ostringstream oss;
	oss << "Done loading " << textures.size() << " textures.";
	if (io) io->println(E_DEBUG_LEVEL::VERBOSE,"load_textures()",oss.str());
	return true;
}

// Helper function for initialize_objects().
Mesh_Data* Widget_OpenGl::build_standard_object(string pfname_obj, string program_name,
		QOpenGLTexture* texture, bool do_transfer_to_GPU)
{
	QFileInfo checkFile(QString(pfname_obj.c_str()));
	if (!(checkFile.exists() && checkFile.isFile())) throw ".obj file not found.";
	stringstream ss;
	io->get_stringstream_from_QFile(pfname_obj,ss);
	Mesh_Data* object = new Mesh_Data(io);
	if (!object->parse_blender_obj(io->read_file(ss)))
	{
		ostringstream oss;
		oss << "Failure to parse '" << pfname_obj.c_str() << ".";
		if (io) io->println(E_DEBUG_LEVEL::ERROR, "build_standard_object(..)",
			oss.str().c_str());
		throw oss.str().c_str();
		return 0;
	}
	object->draw_mode = GL_TRIANGLES;
	object->program_name = program_name;
	object->texture = texture;
	if (do_transfer_to_GPU) object->transfer_vertices_and_elements_to_GPU();
	return object;
}

Known_Texture_Resources::Known_Texture_Resources()
{
	// Note: Start the name with "v_" for vertical mirroring
	// and with "h_" for horizontal mirroring.
	string scrs;
	Known_Sceneries scs;
	for (vector<E_SCENERY>::const_iterator CI=scs.get_sceneries()->begin();
		CI!=scs.get_sceneries()->end();CI++)
	{
	E_SCENERY sc = *CI;
	scrs = Widget_OpenGl::get_scenery_resource_string("sky",sc);
	textures[scrs] = ":/graphics/" + scrs + ".png";

	scrs = Widget_OpenGl::get_scenery_resource_string("foundation",sc);
	textures[scrs] = ":/graphics/" + scrs + ".png";

	scrs = Widget_OpenGl::get_scenery_resource_string("sq_connection",sc);
	textures[scrs] = ":/graphics/" + scrs + ".png";

	scrs = Widget_OpenGl::get_scenery_resource_string("sq_odd",sc);
	textures[scrs] = ":/graphics/" + scrs + ".png";

	scrs = Widget_OpenGl::get_scenery_resource_string("sq_even",sc);
	textures[scrs] = ":/graphics/" + scrs + ".png";

	scrs = Widget_OpenGl::get_scenery_resource_string("tree",sc);
	textures[scrs] = ":/graphics/" + scrs + ".png";

	scrs = Widget_OpenGl::get_scenery_resource_string("tower",sc);
	textures[scrs] = ":/graphics/" + scrs + ".png";
	}
	textures["sentinel"] = ":/graphics/sentinel.png";
	textures["sentry"] = ":/graphics/sentry.png";
	textures["robot"] = ":/graphics/robot.png";
	textures["meanie"] = ":/graphics/meanie.png";
	textures["block"] = ":/graphics/block.png";
}

bool Widget_OpenGl::initialize_objects()
{
	Known_Sceneries scs;
	for (vector<E_SCENERY>::const_iterator CI=scs.get_sceneries()->begin();
		CI!=scs.get_sceneries()->end();CI++)
	{
	string scrs;
	E_SCENERY sc = *CI;
	//>> The Sky Dome. -----------------------------------------------
	scrs = get_scenery_resource_string("sky",sc);
	objects[scrs] = build_standard_object(
		":/blender/sky_dome.obj", "sky", textures.at(scrs), true);
	//<< -------------------------------------------------------------
	//>> The foundation plane of the thunder dome. -------------------
	scrs = get_scenery_resource_string("foundation",sc);
	objects[scrs] = build_standard_object(
		":/blender/plane.obj", "terrain", textures.at(scrs), true);
	//<< -------------------------------------------------------------
	//>> A granite rock wall. ----------------------------------------
	scrs = get_scenery_resource_string("sq_connection",sc);
	objects[scrs] = build_standard_object(
		":/blender/plane.obj", "terrain", textures.at(scrs), true);
	//<< -------------------------------------------------------------
	//>> A light square. ---------------------------------------------
	scrs = get_scenery_resource_string("sq_odd",sc);
	objects[scrs] = build_standard_object(
		":/blender/plane.obj", "terrain", textures.at(scrs), true);
	//<< -------------------------------------------------------------
	//>> A dark square. ----------------------------------------------
	scrs = get_scenery_resource_string("sq_even",sc);
	objects[scrs] = build_standard_object(
		":/blender/plane.obj", "terrain", textures.at(scrs), true);
	//<< -------------------------------------------------------------
	//>> Tree. -------------------------------------------------------
	scrs = get_scenery_resource_string("tree",sc);
	objects[scrs] = build_standard_object(
		":/blender/"+scrs+".obj", "terrain", textures.at(scrs), true);
	//<< -------------------------------------------------------------
	//>> Sentinel Tower. ---------------------------------------------
	scrs = get_scenery_resource_string("tower",sc);
	objects[scrs] = build_standard_object(":/blender/tower.obj",
		"terrain", textures.at(scrs), true);
	//<< -------------------------------------------------------------
	}
	//>> Block. ------------------------------------------------------
	objects["block"] = build_standard_object(":/blender/block.obj",
		"terrain", textures.at("block"), true);
	//<< -------------------------------------------------------------
	//>> Robot. ------------------------------------------------------
	objects["robot"] = build_standard_object(":/blender/robot.obj",
		"terrain", textures.at("robot"), true);
	//<< -------------------------------------------------------------
	//>> Meanie. -----------------------------------------------------
	objects["meanie"] = build_standard_object(":/blender/meanie.obj",
		"terrain", textures.at("meanie"), true);
	//<< -------------------------------------------------------------
	//>> Sentry. -----------------------------------------------------
	objects["sentry"] = build_standard_object(":/blender/sentry.obj",
		"terrain", textures.at("sentry"), true);
	//<< -------------------------------------------------------------
	//>> The Sentinel. -----------------------------------------------
	objects["sentinel"] = build_standard_object(":/blender/sentinel.obj",
		"terrain", textures.at("sentinel"), true);
	//<< -------------------------------------------------------------
	return true;
}

string Widget_OpenGl::get_scenery_resource_string(string prefix, E_SCENERY scenery, string suffix)
{
	string planet;
	switch (scenery)
	{
		case MASTER: planet = "master"; break;
		case EUROPE: planet = "europe"; break;
		case SELENE: planet = "selene"; break;
		case MARS: planet = "mars"; break;
		case ASTEROID: planet = "asteroid"; break;
		default: planet = "europe"; break;
	}
	if (prefix.size() > 0) { prefix += "_"; }
	return prefix + planet + suffix;
}

void Widget_OpenGl::set_context_to_default_state()
{
	for (vector<GLenum>::const_iterator CI=default_display.flags_glEnable.begin();
		CI != default_display.flags_glEnable.end(); ++CI)
	{
		glEnable(*CI);
		switch (*CI)
		{
			case GL_BLEND: // Transparency via alpha channel.
				glBlendFunc(default_display.sfactor_glBlendFunc,
					default_display.dfactor_glBlendFunc);
				break;
 			case GL_CULL_FACE:
				glCullFace(GL_FRONT);
				break;
		}
	}
}

float Widget_OpenGl::plop_fct(float x)
{
	float y = 0;
	if (x < 0)
	{
		y = 0;
	} else if (x < .5)
	{
		y = x*x; // parable up to y=.25
	} else if (x< .65)
	{
		// parable matching first section and running up to 1.3.
		y = 43.333*x*x - 42.833*x + 10.833;
	} else if (x<1.0)
	{
		y = 2.4490*x*x - 4.8980*x + 3.4490;
	} else {
		y = 1.0;
	}
	return y;
}

void Widget_OpenGl::clear_screen(QVector4D color)
{
	glClearColor(color.x(), color.y(), color.z(), color.w());
	glClear(default_display.mask_glClear);
}

Player_Data* Widget_OpenGl::get_player_data()
{
	if (!game) throw "No Game* object found.";
	if (!game->get_player()) throw "Game* object held no player data.";
	return game->get_player();
}

Viewer_Data* Widget_OpenGl::get_viewer_data()
{
	Player_Data* pd = get_player_data();
	Viewer_Data* vd = pd->get_viewer_data();
	if (!vd) throw "Game* objects player pointer held no viewer data.";
	return vd;
}

void Widget_OpenGl::draw_terrain_object(Mesh_Data* object, QMatrix4x4& camera,
	QMatrix4x4& trans_rot_object, float fade, QOpenGLBuffer* buf_vertices)
{
	object->buf_elements.bind();
	if (object->texture) object->texture->bind();
	if (buf_vertices) { buf_vertices->bind(); } else { object->buf_vertices.bind(); }
	QOpenGLShaderProgram* program = programs.at(object->program_name);
	program->bind();
	program->setUniformValue("texture", 0);
	
    int handle_v_vertices = program->attributeLocation("v_vertices");
	int handle_v_normals = program->attributeLocation("v_normals");
    int handle_v_tex_coords = program->attributeLocation("v_tex_coords");
	int handle_v_vertex_colors = program->attributeLocation("v_vertex_colors");
	int handle_pos_light = program->uniformLocation("pos_light");
	int handle_color_light = program->uniformLocation("color_light");
	int handle_ambience = program->uniformLocation("ambience");
	int handle_fade = program->uniformLocation("fade");
	int handle_A = program->uniformLocation("A");
	int handle_B = program->uniformLocation("B");
	
	// Vertices.
	quintptr offset = 0;
	program->enableAttributeArray(handle_v_vertices);
	program->setAttributeBuffer(
		handle_v_vertices, GL_FLOAT, offset, 4, sizeof(Vertex_Data));
	
	// Normal vectors.
	offset += sizeof(QVector4D);
	program->enableAttributeArray(handle_v_normals);
	program->setAttributeBuffer(
		handle_v_normals, GL_FLOAT, offset, 3, sizeof(Vertex_Data));
	
	// Tex coords.
	offset += sizeof(QVector3D);
    program->enableAttributeArray(handle_v_tex_coords);
    program->setAttributeBuffer(
		handle_v_tex_coords, GL_FLOAT, offset, 2, sizeof(Vertex_Data));
	
	// Vertex colors.
	offset += sizeof(QVector2D);
	program->enableAttributeArray(handle_v_vertex_colors);
	program->setAttributeBuffer(
		handle_v_vertex_colors, GL_FLOAT, offset, 4, sizeof(Vertex_Data));
	
	QMatrix4x4 A = camera * trans_rot_object;
	QMatrix3x3 B = trans_rot_object.normalMatrix();

	program->setUniformValue(handle_pos_light, light_position);
	program->setUniformValue(handle_color_light, light_color*light_brightness*light_filtering_factor);
	program->setUniformValue(handle_ambience, light_ambience);
	program->setUniformValue(handle_fade, fade);
	program->setUniformValue(handle_A, A);
	program->setUniformValue(handle_B, B);

    // Draw cube geometry using indices from VBO 1
    glDrawElements(
		object->draw_mode, // Element ordering. Here probably GL_TRIANGLES.
		object->elements.size(), // #elements withinin the bound index buffer.
		GL_UNSIGNED_SHORT, 0);

	program->disableAttributeArray(handle_v_vertex_colors);
	program->disableAttributeArray(handle_v_tex_coords);
	program->disableAttributeArray(handle_v_normals);
	program->disableAttributeArray(handle_v_vertices);
	program->release();

	if (buf_vertices) { buf_vertices->release(); } else { object->buf_vertices.release(); }
	if (object->texture) object->texture->release();
	object->buf_elements.release();
}

void Widget_OpenGl::draw_square(Square* sq, QMatrix4x4& camera, float fade)
{
	if (sq == 0) throw "0 pointer square encountered!";
	Mesh_Data* mesh = sq->get_mesh_prototype();
	QMatrix4x4 trans_rot; trans_rot.setToIdentity(); // The mesh is in world coordinates.
	draw_terrain_object(mesh, camera, trans_rot, fade, &(sq->buf_vertices));
}

void Widget_OpenGl::draw_dome(QMatrix4x4& camera, float fade)
{
	//> Draw the sky dome itself. ------------------------------------
	Mesh_Data* sky = objects.at(get_scenery_resource_string("sky", scenery));
	sky->bind();
    // Offset for position of each data type within Vertex_Data.
	QOpenGLShaderProgram* program = programs.at(sky->program_name);
	program->bind();
    quintptr offset = 0;
	program->setUniformValue("texture", 0);
    // Tell OpenGL programmable pipeline how to locate vertex position data
    int handle_v_vertices = program->attributeLocation("v_vertices");
    int handle_v_tex_coords = program->attributeLocation("v_tex_coords");
	int handle_v_colors = program->attributeLocation("v_colors");
	int handle_A = program->uniformLocation("A");
	int handle_color_light = program->uniformLocation("color_light");
	int handle_fade = program->uniformLocation("fade");
	
	// Assigning the vertices to the vertex shader.
    program->enableAttributeArray(handle_v_vertices);
    program->setAttributeBuffer(
		handle_v_vertices, // handle of the target attribute within the vertex shader.
		GL_FLOAT, // type of data.
		offset, // Offset within the Vertex_Data block seen from the start.
		4, // Size of a single vertex.
		sizeof(Vertex_Data) // Size of a single Vertex_Data block.
	);

	// Assigning the texture coordinates to the vertex shader.
    // Offset for texture coordinates -- they come behind the vec4 vertices
	// and behind the vec3 normal vectors.
    offset += sizeof(QVector4D) + sizeof(QVector3D);
    program->enableAttributeArray(handle_v_tex_coords);
    program->setAttributeBuffer(
		handle_v_tex_coords,
		GL_FLOAT,
		offset,
		2,
		sizeof(Vertex_Data)
	);
	// Assigning the vertex colors to the vertex shader.
	offset += sizeof(QVector2D);
	program->enableAttributeArray(handle_v_colors);
	program->setAttributeBuffer(
		handle_v_colors,
		GL_FLOAT,
		offset,
		4,
		sizeof(Vertex_Data)
	);
	
	QMatrix4x4 A = camera;
	A.translate(0,0,-.01);

	float radius = game ? (2.*game->get_landscape()->get_board_diagonal_length()) : 0.;
	A.scale(radius);
	program->setUniformValue(handle_A, A);

	// Assigning the light color.
	program->setUniformValue(handle_color_light, light_color*light_brightness*light_filtering_factor);

	program->setUniformValue(handle_fade, fade);
	
    // Draw cube geometry using indices from VBO 1
    glDrawElements(
		sky->draw_mode, // Element ordering. Here probably GL_TRIANGLES.
		sky->elements.size(), // #elements withinin the bound index buffer.
		GL_UNSIGNED_SHORT,
		0
	);
	
	program->disableAttributeArray(handle_v_tex_coords);
	program->disableAttributeArray(handle_v_vertices);
	program->release();
	sky->release();
	//< --------------------------------------------------------------
	//> Draw the base of the thunderdome. ----------------------------
	A.setToIdentity(); A.translate(0,0,-.01); A.scale(radius*2);
	draw_terrain_object(objects.at(
		get_scenery_resource_string("foundation", scenery)),
		camera,
		A,
		fade
	);
	//< --------------------------------------------------------------
}
					
float Widget_OpenGl::get_appropriate_scale(Figure* figure)
{
	/** Never shrink the Sentinel! */
	if (figure->get_type()==E_FIGURE_TYPE::SENTINEL) return 1.0;
	E_MATTER_STATE state = figure->get_state();
	float scale = 1.0;
	float fade = figure->get_fade();
	switch (state)
	{
		case E_MATTER_STATE::STABLE: scale = 1.0; break;
		case E_MATTER_STATE::MANIFESTING: // Fall-through
		case E_MATTER_STATE::TRANSMUTING: scale = plop_fct(fade); break;
		case E_MATTER_STATE::DISINTEGRATING: scale = fade; break;
		case E_MATTER_STATE::GONE: scale = 0.0; break;
		default: break;
	}
	return scale;
}

void Widget_OpenGl::draw_landscape(float fade)
{
	if (!game) return;
	Landscape* ls = game->get_landscape();
	if (!ls) throw "Landscape is still a 0 pointer";
	int width = ls->get_board_sq()->get_width();
	int height = ls->get_board_sq()->get_height();
	QPoint player_site = game->get_player()->get_site();
	QMatrix4x4 camera = game->get_player()->get_viewer_data()->get_camera();

	draw_dome(camera,fade); 

	for (int x=0;x<width;x++)
	{
		for (int y=0;y<height;y++)
		{
			Square* sq = ls->get_board_sq()->get(x,y);
			draw_square(sq,camera,fade);
			Figure* fg = game->get_board_fg()->get(x,y);
			if (fg)
			{
				vector<Figure*> fgs = fg->get_above_figure_stack();
				QMatrix4x4 A; A.setToIdentity();
				A.translate(x,y,sq->get_altitude());
				for (vector<Figure*>::const_iterator CI=fgs.begin();CI!=fgs.end();CI++)
				{
					Figure* f = *CI;
					if (
						game->get_status() != E_GAME_STATUS::SURVEY &&
						player_site.x() == x && player_site.y() == y &&
						f->get_type()==E_FIGURE_TYPE::ROBOT
					) continue; // Don't draw self unless in SURVEY mode.
					// B: tranlate*rotate*scale for the object.
					QMatrix4x4 B = A;
					B.rotate(f->get_phi(),QVector3D(0,0,1));
					float scale = get_appropriate_scale(f);
					B.scale(scale);
					draw_terrain_object(f->get_mesh_prototype(),camera,B,f->get_fade()*fade);
					if (f->get_state()==E_MATTER_STATE::TRANSMUTING && scale > 0)
					{
						float old_mesh_fade = 1-f->get_fade();
						B.scale(old_mesh_fade/scale);
						Mesh_Data* old_mesh = f->get_old_mesh();
						if (old_mesh)
							draw_terrain_object(old_mesh,camera,B,old_mesh_fade*fade);
					}
					// The next figure in the stack will be on the new figure.
					A.translate(QVector3D(0,0,Figure::get_height(f->get_type())));
				}
			}
		}
	}
	//< --------------------------------------------------------------
}

void Widget_OpenGl::setup_light_source(QVector4D light_color,
	float light_ambience, QVector3D light_position)
{
	this->light_filtering_factor = QVector4D(1,1,1,1);
	this->light_brightness = QVector4D(1,1,1,1);
	this->light_position = light_position;
	this->light_color = light_color;
	this->light_ambience = light_ambience;
}

void Widget_OpenGl::initializeGL()
{
	//> Initializing the openGL context. -----------------------------
	initializeOpenGLFunctions();
	set_context_to_default_state();
	//< --------------------------------------------------------------
	//> Checking for openGL 2.0. -------------------------------------
	GLint maj, min, dbbits;
	this->glGetIntegerv(GL_MAJOR_VERSION, &maj);
    this->glGetIntegerv(GL_MINOR_VERSION, &min);
	this->glGetIntegerv(GL_DEPTH_BITS, &dbbits);
	if (maj < 2)
	{
		ostringstream oss;
		oss << "Critical: Open GL version " << maj << "." << min << " detected. "
			"However, open GL >= 2.0 is required.";
		disable_program(QString(oss.str().c_str()));
		initializeGL_ok = false;
		return;
	} else {
		ostringstream oss;
		oss << "Initialized openGL " << maj << "." << min << " context with a " <<
			dbbits << " bit depth buffer.";
		this->update_parent_statusBar_text(oss.str().c_str());
		initializeGL_ok = true;
	}
	//< --------------------------------------------------------------
	//> Initializing game content. -----------------------------------
	setup_light_source();
	initializeGL_ok = compile_programs()   && initializeGL_ok;
	initializeGL_ok = load_textures()      && initializeGL_ok;
	initializeGL_ok = initialize_objects() && initializeGL_ok;
	if (!initializeGL_ok)
	{
		disable_program("Critical: Unable to initialize game resources. See console error messages for details.");
	}
	//< --------------------------------------------------------------
	//> Setting up the update timer. ---------------------------------
    timer_framerate->start(1000./framerate);
	//< --------------------------------------------------------------
}

void Widget_OpenGl::resizeGL(int width, int height)
{
	do_repaint = true;
}

// Note: makeCurrent() and doneCurrent() are automatically called when Qt calls paintGL.
void Widget_OpenGl::paintGL()
{
	if (!(do_repaint && initializeGL_ok && game)) { return; }
	clear_screen();
	draw_landscape(this->light_color.w());
	do_repaint = false;
}

float Widget_OpenGl::get_Gl_mouse_x(bool clamp)
{
	float val = (2*(float)(mapFromGlobal(QCursor::pos()).x())/(float)(this->width()))-1;
	if (clamp && abs(val) > 1) val = val < 0 ? -1 : 1;
	return val;
}

float Widget_OpenGl::get_Gl_mouse_y(bool clamp)
{
	float val = -((2*(float)(mapFromGlobal(QCursor::pos()).y())/(float)(this->height()))-1);
	if (clamp && abs(val) > 1) val = val < 0 ? -1 : 1;
	return val;
}


void Widget_OpenGl::mouseReleaseEvent(QMouseEvent* e)
{
	if (!game) return;
	Qt::MouseButton button = e->button();
	Viewer_Data* vd = game->get_player()->get_viewer_data();
	Player_Data* pd = game->get_player();
	E_GAME_STATUS status = game->get_status();
	bool running = !is_paused();
	switch (button)
	{
		case Qt::MouseButton::LeftButton:
			if (status==E_GAME_STATUS::SURVEY)
			{
				if (running)
				{
					game->end_survey();
					request_paintGL();
				}
			} else if (status==E_GAME_STATUS::WON) {
				request_new_game();
				request_paintGL();
			} else if (status==E_GAME_STATUS::LOST) {
				request_restart_game();
				request_paintGL();
			} else {
				if (running) pd->switch_cursor_mode();
			}
			break;
		case Qt::MouseButton::RightButton:
			if (running)
			{
				game->do_u_turn();
			}
			break;
		case Qt::MouseButton::MiddleButton:
			if (running)
			{
				vd->set_opening(get_player_data()->get_opening_default());
			}
			break;
		default: break;
	}
	e->accept();
}

void Widget_OpenGl::update_zoom(float dFOV)
{
	if (!game) return; // Nothing to see, nothing to do.
	Player_Data* pd = game->get_player();
	Viewer_Data* vd = pd->get_viewer_data();
	float opening = vd->get_opening() + dFOV;
	if (opening > pd->get_opening_max()) { opening = pd->get_opening_max(); }
	if (opening < pd->get_opening_min()) { opening = pd->get_opening_min(); }
	vd->set_opening(opening);
}

void Widget_OpenGl::wheelEvent(QWheelEvent* e)
{
	if ((!is_paused()) && game && !e->angleDelta().isNull())
	{
		// +-120, -120: turned towards user.
		float angle = -(float)(e->angleDelta().y())/60.;
		update_zoom(angle);
	}
	e->accept();
}

void Widget_OpenGl::player_dynamic_rotation(float framerate, float center)
{
	if (get_player_data()->get_cursor_mode()) return;
	float dt = 1./framerate;
	float x = get_Gl_mouse_x();
	float y = get_Gl_mouse_y();
	if ((x < -center || x > center || y < -center || y > center)
		&& underMouse())
	{
		Player_Data* pd = get_player_data();
		Viewer_Data* vd = get_viewer_data();
		int self_spin = pd->get_self_spin();
		float dphi, dtheta;
		switch (game->get_status())
		{
			case E_GAME_STATUS::PRELIMINARY:
			case E_GAME_STATUS::RUNNING:
			case E_GAME_STATUS::SENTINEL_ABSORBED:
			case E_GAME_STATUS::TOWER_TAKEN:
				dphi = -x * 360./((float)self_spin);
				dtheta = -y * 360./((float)self_spin);
				vd->update_direction(dt,dphi,dtheta);
				break;
			case E_GAME_STATUS::SURVEY:
				dphi = dt * x * 360./((float)self_spin);
				dtheta = (y < -center || y > center) ?
					dt * y * 360./((float)self_spin) : 0;
				game->set_survey_view_data(dphi,dtheta);
				break;
			default: break;
		}
	}
}

void Widget_OpenGl::update_after_dt()
{
	if (is_paused() || !game) return;
	player_dynamic_rotation(framerate);
	bool had_progress = game->do_progress(1./((float)framerate));
	do_repaint = do_repaint || had_progress;
	update();
}

void Widget_OpenGl::request_paintGL()
{
	do_repaint = true;
}

void Widget_OpenGl::set_light_filtering_factor(QVector4D color, int action)
{
	switch (action)
	{
		case 0: // Additive.
			light_filtering_factor *= color;
			break;
		case 1: // Substractive.
			color.setX(1./color.x());
			color.setY(1./color.y());
			color.setZ(1./color.z());
			color.setW(1./color.w());
			light_filtering_factor *= color;
			break;
		case 2: // Simple replacement.
			light_filtering_factor = color;
			break;
		default: throw "Unknown action.";
	}
}

void Widget_OpenGl::enterEvent(QEvent* e)
{
	is_auto_paused = false;
	if (game) game->unpause_timers();
	display_pause_game();
}

void Widget_OpenGl::leaveEvent(QEvent* e)
{
	is_auto_paused = true;
	if (game) game->pause_timers();
	display_pause_game();
}

// void Widget_OpenGl::focusInEvent(QFocusEvent* e) { enterEvent(e); }
// void Widget_OpenGl::focusOutEvent(QFocusEvent* e) { leaveEvent(e); }

void Widget_OpenGl::modify_brightness(float factor)
{
	if (factor == 1.0)
	{
		light_brightness = QVector4D(1,1,1,1);
	} else {
		light_brightness *= factor;
	}
	request_paintGL();
}

void Widget_OpenGl::keyPressEvent(QKeyEvent* e)
{
	if (!game) { e->ignore(); return; }
	QPoint board_pos(-1,-1);
	Figure* figure=0;
	E_POSSIBLE_PLAYER_ACTION action = game->get_mouse_target(
		get_Gl_mouse_x(true), get_Gl_mouse_y(true), board_pos, figure);
	bool running = !is_paused();
	switch(e->key())
	{
		case Qt::Key_Escape: exit_requested(); break;
		case Qt::Key_A: // Absorb object.
			if (running && (action == E_POSSIBLE_PLAYER_ACTION::ABSMANI ||
				action == E_POSSIBLE_PLAYER_ACTION::ABSORPTION))
			{
				game->disintegrate_figure(board_pos,true);
			}
			break;
		case Qt::Key_T: // Manifest tree.
			if (running && (action == E_POSSIBLE_PLAYER_ACTION::ABSMANI ||
				action == E_POSSIBLE_PLAYER_ACTION::MANIFESTATION))
			{
				game->manifest_figure(board_pos,E_FIGURE_TYPE::TREE,true);
			}
			break;
		case Qt::Key_B: // Manifest block.
			if (running && (action == E_POSSIBLE_PLAYER_ACTION::ABSMANI ||
				action == E_POSSIBLE_PLAYER_ACTION::MANIFESTATION))
			{
				game->manifest_figure(board_pos,E_FIGURE_TYPE::BLOCK,true);
			}
			break;
		case Qt::Key_R: // Manifest robot.
			if (running && (action == E_POSSIBLE_PLAYER_ACTION::ABSMANI ||
				action == E_POSSIBLE_PLAYER_ACTION::MANIFESTATION))
			{
				game->manifest_figure(board_pos,E_FIGURE_TYPE::ROBOT,true);
			}
			break;
		case Qt::Key_Q: // Transfer consciousness.
			if (running && (action == E_POSSIBLE_PLAYER_ACTION::EXCHANGE))
			{
				game->transfer(board_pos);
			}
			break;
		case Qt::Key_U: // Attempt u-turn.
			if (running)
			{
				game->do_u_turn();
			}
			break;
		case Qt::Key_H: // Hyperjump.
			if (running)
			{
				if (game->hyperspace_request())
				{
					request_paintGL();
				}
			}
			break;
		case Qt::Key_Space: // Toggle cursor mode.
			if (running)
			{
				game->get_player()->switch_cursor_mode();
			}
			break;
		case Qt::Key_O: // Toggle sound.
			{
				update_parent_statusBar_text(
					game->toogle_sound() ?
						QObject::tr("Sound on.") :
						QObject::tr("Sound off.")
				);
				break;
			}
		case Qt::Key_P: // Toggle user driven pause mode.
			if (this->underMouse())
			{
				is_user_paused = !is_user_paused;
				is_auto_paused = false;
				if (is_user_paused) { game->pause_timers(); }
				else { game->unpause_timers(); }
				display_pause_game();
			}
			break;
		case Qt::Key_F: // Toggle fullscreen mode.
			{
				fullscreen_key_pressed();
				request_paintGL();
			}
			break;
		case Qt::Key_Plus: // Zoom in.
			if (running)
			{
				update_zoom(-DEFAULT_DELTA_ZOOM);
			}
			break;
		case Qt::Key_Minus: // Zoom out.
			if (running)
			{
				update_zoom(DEFAULT_DELTA_ZOOM);
			}
			break;
		case Qt::Key_Period: // Normalize zoom.
			if (running)
			{
				game->get_player()->get_viewer_data()->
					set_opening(get_player_data()->get_opening_default());
				request_paintGL();
			}
			break;
		case Qt::Key_1: // Darken image.
			{
				modify_brightness(0.95);
			}
			break;
		case Qt::Key_2: // Reset image brightness.
			{
				modify_brightness();
			}
			break;
		case Qt::Key_3: // Brighten image.
			{
				modify_brightness(1./.95);
			}
			break;
		case Qt::Key_S: // Analyze scanned object.
			{
				game->handle_scan_key(action, board_pos, figure);
			}
			break;
		case Qt::Key_W: // Where am I? Yes, debug code. But I like it nonetheless :-)
			{
				game->where_am_i();
			}
			break;
		default: e->ignore(); // Propagate this keypress to parent QObject.
	}
}
}
