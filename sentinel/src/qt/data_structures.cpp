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

#include "data_structures.h"

#include <cmath>
#include <map>
#include <iostream>
#include <sstream>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>

using std::cout;
using std::endl;
using std::map;
using std::endl;
using std::stringstream;
using std::istringstream;
using std::ostringstream;

namespace display
{
//> Known_Sceneries. -------------------------------------------------
bool Known_Sceneries::is_supported(E_SCENERY scenery)
{
	bool found_it = false;
	for (vector<E_SCENERY>::const_iterator CI=sceneries.begin();CI!=sceneries.end();CI++)
		{ if (*CI == scenery) { found_it = true; break; } }
	return found_it;
}

string Known_Sceneries::toString(E_SCENERY scenery)
{
	string key;
	switch (scenery)
	{
		case E_SCENERY::MASTER: key = "MASTER"; break;
		case E_SCENERY::EUROPE: key = "EUROPE"; break;
		case E_SCENERY::SELENE: key = "SELENE"; break;
		case E_SCENERY::MARS:   key = "MARS"; break;
		case E_SCENERY::ASTEROID: key = "ASTEROID"; break;
		default: key = "EUROPE"; break;
	}
	return key;
}
//< ------------------------------------------------------------------

//> Vertex_Data. -----------------------------------------------------
string Vertex_Data::toString()
{
	ostringstream oss;
	oss << "v: (" << vertex.x() << ", " << vertex.y() << ", " <<
		vertex.z() << ", " << vertex.w() << "); " <<
	  "n: (" << normal.x() << ", " << normal.y() << ", " << normal.z() << "; " <<
	  "tc: (" << tex_coord.x() << ", " << tex_coord.y() << "); " <<
	  "color: ("  << color.x() << ", " << color.y() << ", " <<
		color.z() << ", " << color.w() << ");";
	return oss.str();
}
//< ------------------------------------------------------------------

//> Mesh_Data. -------------------------------------------------------
vector<GLushort> Mesh_Data::obj_index_to_vector(const string obj)
{
	istringstream iss(obj);
	char c;
	int val;
	vector<GLushort> res;
	while (iss >> val)
	{
		iss >> c; // Eating away the '/' character.
		res.push_back(val);
	}
	return res;
}

bool Mesh_Data::parse_blender_obj(string src)
{
	string caller = "Mesh_Data::parse_blender_obj(..)";
	E_DEBUG_LEVEL dl = E_DEBUG_LEVEL::WARNING;
	//> Step 1: Parse input lines. -----------------------------------
	istringstream in(src);
	float val;
	bool has_first = false;
	string first_line = "";
	string line;
	vector<QVector4D> obj_vertices;
	vector<QVector2D> obj_tex_coords;
	vector<QVector3D> obj_normals;
	vector<string> obj_faces; // Contains complete face indecies like '1/4 5/8 6/6'
	while (getline(in, line))
	{
		if (!has_first) { first_line = line; has_first = true; }
		// 'v ' starts a vec3 vertex line.
		if (line.substr(0,2).compare("v ")==0)
		{
			// Quite the opposite of ostringstream. Quite practical for parsing! :-)
			istringstream iss(line.substr(2));
			QVector4D v(0,0,0,0);
			iss >> val; v.setX(val);
			iss >> val; v.setZ(val); // Note: With blender Z and Y coordinates are
			iss >> val; v.setY(val); // swapped from an openGL point of view.
			v.setW(1.0);
			obj_vertices.push_back(v);
		}
		// vt introduces a UV texture 2D coordinate pair.
		else if (line.substr(0,3).compare("vt ")==0)
		{
			istringstream iss(line.substr(3));
			QVector2D t(0,0);
			iss >> val; t.setX(val);
			iss >> val; t.setY(val);
			obj_tex_coords.push_back(t);
		}
		// vn introduces normal vectors.
		else if (line.substr(0,3).compare("vn ")==0)
		{
			istringstream iss(line.substr(3));
			QVector3D n(0,0,0);
			iss >> val; n.setX(val);
			iss >> val; n.setZ(val); // Note: With blender Z and Y coordinates are
			iss >> val; n.setY(val); // swapped from an openGL point of view.
			obj_normals.push_back(n);
		}
		// f introduces a face, using vertex indices, starting at 1
		else if (line.substr(0,2).compare("f ")==0)
		{
			obj_faces.push_back(line);
		}
		else {} // Ignore the rest. For now.
	}
	if (obj_vertices.size() == 0 || obj_normals.size() == 0 ||
		obj_tex_coords.size() == 0)
	{
		ostringstream oss;
		oss << "One of the input data vectors from the .obj remains empty. "
			"vertices: "  << obj_vertices.size() << "; normals: "  <<
			obj_normals.size() << "; tex_coords: "  << obj_tex_coords.size();
		if (io) io->println(dl, caller, oss.str());
		return false;
	}
	//< --------------------------------------------------------------
	//> Step 2: Building a mapping of blender (a/at/an) to GL (index).
	// We need a bijective mapping blender_index => { 0,..,obj_faces.size()-1 }
	map<string,GLushort> index_obj_gl;
	GLushort index_gl = 0;
	for (vector<string>::const_iterator CI=obj_faces.begin();CI!=obj_faces.end();CI++)
	{
		istringstream iss((*CI).substr(2));
		string block;
		while (iss >> block)
		{
			// This seems to work even for string object keys. I tested it:
			// string a("alpha"), b("alpha") has m[b] overwriting m[a].
			if (index_obj_gl.find(block)==index_obj_gl.end())
			{
				index_obj_gl[block] = index_gl++;
			}
		}
	}
	//< --------------------------------------------------------------
	//> Step 3: Building the GL vectors based on new GL indexing. ----
	uint block_size = 0;
	uint n = index_obj_gl.size();
	vector<QVector4D> gl_vertices;
	vector<QVector3D> gl_normals;
	vector<QVector2D> gl_tex_coords;
	gl_vertices.resize(n, QVector4D(0,0,0,1));
	gl_normals.resize(n, QVector3D(0,0,0));
	gl_tex_coords.resize(n, QVector2D(0,0));
	for (map<string,GLushort>::const_iterator CI=index_obj_gl.begin();CI!=index_obj_gl.end();CI++)
	{
		vector<GLushort> parts = obj_index_to_vector(CI->first);
		GLushort index = CI->second;
		block_size = parts.size();
		if (block_size>0) gl_vertices[index] = obj_vertices.at(parts[0]-1);
		if (block_size>1) gl_tex_coords[index] = obj_tex_coords.at(parts[1]-1);
		if (block_size>2) gl_normals[index] = obj_normals.at(parts[2]-1);
	}
	//< --------------------------------------------------------------
	//> Step 4: Building the GL elements vector. ---------------------
	// This step relies on having everything triangulated!
	elements.clear();
	for (vector<string>::const_iterator CI=obj_faces.begin();CI!=obj_faces.end();CI++)
	{
		istringstream iss(CI->substr(2));
		string block;
		while (iss >> block)
		{
			elements.push_back(index_obj_gl.at(block));
		}
	}
	//< --------------------------------------------------------------
	//> Step 5: Load the vectors into this->vertices. ----------------
	for (uint j=0;j<n;j++)
	{
		this->vertices.push_back(Vertex_Data
		(
			gl_vertices[j],
			gl_normals[j],
			gl_tex_coords[j]
		));
	}
	if (io)
	{
		ostringstream oss;
		oss << "Generated " << this->vertices.size() << " Vertex_Data blocks "
			"and " << this->elements.size() << " index elements for '" <<
			first_line.c_str() << "'.";
		if (io) io->println(E_DEBUG_LEVEL::VERBOSE, caller, oss.str());
	}
	//< --------------------------------------------------------------
	return true;
}

bool Mesh_Data::transfer_vertices_and_elements_to_GPU()
{
	//> Sanity Checks and preliminaries. -----------------------------
	string caller = "Mesh_Data::transfer_vertices_and_elements_to_GPU()";
	E_DEBUG_LEVEL dl = E_DEBUG_LEVEL::WARNING;
	if (vertices.size() == 0 || elements.size() == 0)
	{
		if (io) io->println(dl, caller, "Unable to transfer. Either vertices "
			"or elements are yet empty.");
		return false;
	}
	//< --------------------------------------------------------------
	//> Actual transfer. ---------------------------------------------
	if (!buf_vertices.create() || !buf_elements.create())
	{
		if (io) io->println(dl, caller,
			"Failure to create either buf_vertices or buf_elements.");
		return false;
	}
	if (buf_vertices.bind())
	{
		buf_vertices.allocate(&(vertices[0]), vertices.size() * sizeof(Vertex_Data));
		buf_vertices.release();
	} else {
		if (io) io->println(dl, caller,
			"Failure to bind buf_vertices to active openGL context.");
		return false;
	}
	if (buf_elements.bind())
	{
		buf_elements.allocate(&(elements[0]), elements.size() * sizeof(GLushort));
		buf_elements.release();
	} else {
		if (io) io->println(dl, caller,
			"Failure to bind buf_elements to active openGL context.");
		return false;
	}
	//< --------------------------------------------------------------
	return true;
}

bool Mesh_Data::bind()
{
	bool allGood = buf_vertices.bind();
	allGood = buf_elements.bind() && allGood;
	if (texture) texture->bind();
	return allGood;
}

void Mesh_Data::release()
{
	buf_vertices.release();
	buf_elements.release();
	if (texture) texture->release();
}

Mesh_Data::Mesh_Data(Io_Qt* io)
	: program_name("undef"), texture(0),
	  buf_vertices(QOpenGLBuffer::VertexBuffer), buf_elements(QOpenGLBuffer::IndexBuffer)
{
	this->io = io;
	// draw_mode = GL_TRIANGLE_STRIP;
	draw_mode = GL_TRIANGLES;
}

Mesh_Data::~Mesh_Data()
{
	if (buf_vertices.isCreated()) buf_vertices.destroy();
	if (buf_elements.isCreated()) buf_elements.destroy();
}
//< ------------------------------------------------------------------

//> Viewer_Data. -----------------------------------------------------
float Viewer_Data::get_phi()
{
	QVector2D vec(dir_view.x(),dir_view.y());
	vec.normalize();
	float phi = acos(vec.x())/deg_to_radians; // in [0,180]
	if (vec.y() < 0) phi = 360.-phi;
	return phi;
}

float Viewer_Data::get_theta()
{
	QVector3D dir = dir_view;
	dir.normalize();
	float theta = acos(dir.z())/deg_to_radians;
	return theta;
}

QVector3D Viewer_Data::get_direction()
{
	return this->dir_view;
}

QVector3D Viewer_Data::get_site()
{
	return this->site;
}

void Viewer_Data::set_opening(float opening)
{
	this->opening = opening;
	viewer_data_changed();	
}

void Viewer_Data::set_direction(float phi, float theta, float alpha)
{
	phi *= deg_to_radians;
	theta *= deg_to_radians;
	// alpha? Nope! We will actually need it in degrees!
	float sp = sin(phi); float cp = cos(phi);
	float st = sin(theta); float ct = cos(theta);
	QVector3D d_view(cp*st, sp*st, ct);
	set_dir_view(d_view);
	if (st==0)
	{
		QVector3D d_up(-1,0,0);
		set_dir_up(d_up);
	} else {
		QVector3D d_up(0,0,1);
		if (alpha != 0)
		{
			QMatrix4x4 A;
			A.setToIdentity();
			A.rotate(alpha,dir_view);
			set_dir_up(A * d_up);
		} else {
			set_dir_up(d_up);
		}
	}
}

void Viewer_Data::set_dir_view(QVector3D dir_view)
{
	this->dir_view = dir_view;
	viewer_data_changed();
}

void Viewer_Data::set_dir_up(QVector3D dir_up)
{
	this->dir_up = dir_up;
	viewer_data_changed();
}

void Viewer_Data::update_direction(float dt, float dphi, float dtheta, float dalpha)
{
	//> Step 1: Reconstruct phi and theta in radians from dir_view. --
	dir_view.normalize();
	float theta = acos(dir_view.z());
	
	float phi=0;
	QVector2D dir_view_plain(dir_view.x(),dir_view.y());
	dir_view_plain.normalize();
	if (dir_view_plain.lengthSquared() > 0)
	{
		phi = acos(dir_view_plain.x());
		if (dir_view_plain.y() < 0) phi = -phi;
	}
	//< --------------------------------------------------------------
	//> Step 2: Apply delta values. ----------------------------------
	phi += dphi*dt*deg_to_radians;
	theta += dtheta*dt*deg_to_radians;
	float sp = sin(phi); float cp = cos(phi);
	float st = sin(theta); float ct = cos(theta);
	QVector3D dir(cp*st, sp*st, ct);
	//< --------------------------------------------------------------
	//> Step 3: Rotate dir_up by alpha. ------------------------------
	// TODO: Special case: dir_view and dir_up are collinear. What then?
	if (dalpha>0)
	{
		QMatrix4x4 A;
		A.setToIdentity();
		A.rotate(dalpha, dir_view);
		set_dir_up(A * dir_up);
	}
	//< --------------------------------------------------------------
	set_dir_view(dir);
}

void Viewer_Data::turn_around_z_axis()
{
	QVector3D dir(-dir_view.x(),-dir_view.y(),dir_view.z());
	set_dir_view(dir);
}

void Viewer_Data::set_site(QVector3D site)
{
	this->site = site;
	viewer_data_changed();
}

void Viewer_Data::update_site(float dt, QVector3D ds)
{
	QVector3D new_site = dt*ds + site;
	set_site(new_site);
}

void Viewer_Data::set_perspective(float near, float far, float opening)
{
	this->near = near;
	this->far = far;
	set_opening(opening);
}

void Viewer_Data::update_perspective(float dt, float dnear, float dfar, float dopening)
{
	this->near += dt * dnear;
	this->far += dt * dfar;
	set_opening(this->opening + dt * dopening);
}

void Viewer_Data::set_viewer_data(QVector3D site, QVector3D dir, float opening)
{
	set_site(site);
	set_dir_view(dir);
	set_opening(opening);
}

void Viewer_Data::update_viewer_data_for_transfer(QPoint old_site, QPoint new_site,
	float old_alt, float new_alt, float opening)
{
	QVector3D site(new_site.x(), new_site.y(), new_alt);
	QVector3D dir(
		old_site.x()-new_site.x(),
		old_site.y()-new_site.y(),
		old_alt - new_alt
	);
	set_viewer_data(site,dir,opening);
}

float Viewer_Data::get_fov_h()
{
	float v_stretch = tan(get_opening()*deg_to_radians / 2.);
	float h_stretch = get_aspect() * v_stretch;
	float half_h_fov = atan(h_stretch)/deg_to_radians;
	return half_h_fov * 2.0f;
}

float Viewer_Data::get_aspect()
{
	if (parent == 0) throw "Unable to calculate apsect ratio without proper parent.";
	return ((float)parent->width())/((float)parent->height());
}

QMatrix4x4 Viewer_Data::get_camera()
{
	float aspect = get_aspect();
	if (aspect*opening > 120) this->opening = 120/aspect;
	float fov = opening;
	QMatrix4x4 A;
	A.setToIdentity();
	A.perspective(fov, aspect, near, far);
	A.lookAt(site,site+dir_view,dir_up);
	return A;
}

string Viewer_Data::toString()
{
	ostringstream oss;
	oss << "site: (" << site.x() << ", " << site.y() << ", " << site.z() << "); " <<
	  "dir_vw: (" << dir_view.x() << ", " << dir_view.y() << ", " << dir_view.z() << "); " <<
	  "dir_up: (" << dir_up.x() << ", " << dir_up.y() << ", " << dir_up.z() << "); " << endl <<
	  "near: " << near << ", far: " << far << ", opening: " << opening;
	return oss.str();
}

Viewer_Data::Viewer_Data(QOpenGLWidget* parent, QVector3D site, float phi,
	float theta, float alpha, float near, float far, float opening)
		: deg_to_radians(0.01745329251), parent(parent)
{
	this->set_site(site);
	this->set_perspective(near, far, opening);
	this->set_direction(phi, theta, alpha);
}

Viewer_Data::Viewer_Data(QOpenGLWidget* parent)
  : Viewer_Data(parent, QVector3D(0,0,0), 0, 90, 0,
	DEFAULT_NEAR_PLANE, DEFAULT_FAR_PLANE, DEFAULT_OPENING_ANGLE) {}
//< ------------------------------------------------------------------

//> Player Data. -----------------------------------------------------
bool Player_Data::take_hit(float val)
{
	if (psi_shield_val > 0) { psi_shield_val-=val; return false; }
	if (confidence_val > 0) { confidence_val-=val; return false; }
	return true;
}

void Player_Data::reset_confidence()
{
	this->confidence_val = confidence_max;
}

void Player_Data::reset_shields()
{
	this->psi_shield_val = this->psi_shield_max;
	this->confidence_val = this->confidence_max;
}

void Player_Data::set_cursor_mode(bool cursor_mode)
{
	if (cursor_mode != this->cursor_mode)
	{
		this->cursor_mode = cursor_mode;
		cursor_mode_switched(cursor_mode);
	}
}

void Player_Data::set_site(QPoint site)
{
	this->site = site;
}

int Player_Data::update_energy_units(int delta_W)
{
	energy_units += delta_W;
	return energy_units;
}

Player_Data::Player_Data(QOpenGLWidget* parent, Setup_game_data* setup,
	Io_Qt* io, QPoint site, float opening_min, float opening_default, float opening_max) :
		site(site.x(),site.y()), former_site(-1,-1)
{
	this->io = io;
	this->cursor_mode = false;
	this->opening_min = opening_min;
	this->opening_default = opening_default;
	this->opening_max = opening_max;
	this->self_spin = (float)setup->spinBox_self_spin;
	this->psi_shield_max = (float)setup->spinBox_psi_shield;
	this->psi_shield_val = this->psi_shield_max;
	this->confidence_max = (float)setup->spinBox_confidence;
	this->confidence_val = this->confidence_max;
	this->energy_units = DEFAULT_ENERGY_UNITS;
	this->viewer_data = new Viewer_Data(parent);
	this->under_light_attack = false;
	this->under_heavy_attack = false;
}

Player_Data::~Player_Data()
{
	delete this->viewer_data;
}
//< ------------------------------------------------------------------
}
