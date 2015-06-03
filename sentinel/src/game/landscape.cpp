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

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <QtGlobal>
#include "landscape.h"

using std::ostringstream;
using std::pair;
using std::cout;
using std::endl;

namespace game
{
//> Figure. ----------------------------------------------------------
vector<Attack_duration> Figure::mount_attacks(vector<Antagonist_target>& targets)
{
	// This function is a bit inefficient. However, as it stands it has
	// little work to do.
	vector<Attack_duration> new_attack;
	// Only stable and disintegrating antagonists fight.
	if (state != E_MATTER_STATE::STABLE && state != E_MATTER_STATE::DISINTEGRATING)
		{ return new_attack; }
	//> Step 1: Which old Attack_durations are to be kept? -----------
	for (vector<Attack_duration>::const_iterator CI=attacks.begin();CI!=attacks.end();CI++)
	{
		Attack_duration next = *CI;
		bool found_it = false;
		for (vector<Antagonist_target>::const_iterator CI2=targets.begin();CI2!=targets.end();CI2++)
		{
			if (next.board_pos == CI2->board_pos)
			{
				found_it = true;
				break;
			}
		}
		// Preserve old Attack_duration.
		if (found_it) new_attack.push_back(next);
	}
	//< --------------------------------------------------------------
	//> Step 2: Which new QPoints are to be added? -------------------
	for (vector<Antagonist_target>::const_iterator CI=targets.begin();CI!=targets.end();CI++)
	{
		Antagonist_target next = *CI;
		bool found_it = false;
		for (vector<Attack_duration>::const_iterator CI2=attacks.begin();CI2!=attacks.end();CI2++)
		{
			if (next.board_pos == CI2->board_pos)
			{
				found_it = true;
				break;
			}
		}
		// Add new Attack duration.
		if (!found_it) new_attack.push_back(Attack_duration(0,next.board_pos,next.visibility));
	}
	//< --------------------------------------------------------------
	//> Step 3: Increase all counters by 1. --------------------------
	for (vector<Attack_duration>::iterator IT=new_attack.begin();IT!=new_attack.end();IT++)
	{
		IT->inc();
	}
	//< --------------------------------------------------------------
	this->attacks = new_attack;
	return new_attack;
}

void Figure::set_type(E_FIGURE_TYPE new_type, Mesh_Data* new_mesh)
{
	if (type == new_type) throw "Attempting to exchange same type with same type.";
	mesh_transmutation_origin = mesh;
	mesh = new_mesh;
	type = new_type;
	this->state = E_MATTER_STATE::TRANSMUTING;
	this->fade = 0;
}

void Figure::set_state(E_MATTER_STATE new_state, bool by_robot)
{
	// Only stable Figures may have their matter state changed by an external force.
	if (state != E_MATTER_STATE::STABLE)
	{
		return;
	} else {
		this->state = new_state;
	}
	if (new_state == E_MATTER_STATE::DISINTEGRATING)
	{
		this->absorption_triggered_by_robot = by_robot;
	}
}

void Figure::set_figure_above(Figure* fg, int c)
{
	if (fg==0)
	{
		figure_above = 0;
		return;
	}
	if ((c++)>1000) throw "Probably a cyclic stack encounter!";
	if (figure_above == 0)
	{
		figure_above = fg;
		fg->figure_below = this;
	} else {
		figure_above->set_figure_above(fg,c);
	}	
}

float Figure::get_mesh_height(E_FIGURE_TYPE type)
{
	float res;
	switch (type)
	{
		case E_FIGURE_TYPE::TREE: res = 2.50; break;
		case E_FIGURE_TYPE::MEANIE: res = 2.00; break;
		case E_FIGURE_TYPE::BLOCK: res = 1.00; break;
		case E_FIGURE_TYPE::ROBOT: res = 1.53; break;
		case E_FIGURE_TYPE::SENTRY: res = 1.80; break;
		case E_FIGURE_TYPE::SENTINEL: res = 2.00; break;
		case E_FIGURE_TYPE::TOWER: res = 2.00; break;
		default: res = 0;
	}
	return res;
}

int Figure::get_height(E_FIGURE_TYPE type)
{
	return type==E_FIGURE_TYPE::BLOCK ? 1 : 2;
}

int Figure::get_energy_value(E_FIGURE_TYPE type)
{
	int res;
	switch (type)
	{
		case E_FIGURE_TYPE::TREE: res = 1; break;
		case E_FIGURE_TYPE::MEANIE: res = 1; break;
		case E_FIGURE_TYPE::BLOCK: res = 2; break;
		case E_FIGURE_TYPE::ROBOT: res = 3; break;
		case E_FIGURE_TYPE::SENTRY: res = 3; break;
		case E_FIGURE_TYPE::SENTINEL: res = 4; break;
		default: res = 0;
	}
	return res;
}

bool Figure::is_gone()
{
	return this->state == E_MATTER_STATE::GONE;
}

bool Figure::is_stable()
{
	return this->state == E_MATTER_STATE::STABLE;
}

bool Figure::is_antagonist()
{
	return (state == E_MATTER_STATE::STABLE || state == E_MATTER_STATE::DISINTEGRATING) &&
		(type == E_FIGURE_TYPE::SENTINEL || type == E_FIGURE_TYPE::SENTRY || type == E_FIGURE_TYPE::MEANIE);
}

bool Figure::progress(float dt, E_ANTAGONIST_ACTION action)
{
	if (this->state == E_MATTER_STATE::GONE) return false; // Nothing to do.
	bool relevant_progress = false;
	
	if (!(action==E_ANTAGONIST_ACTION::STILL))
	{
		float sign = (action == E_ANTAGONIST_ACTION::MOVING_FORWARD) ? 1. : -1.;
		float meanie_factor = (type==E_FIGURE_TYPE::MEANIE) ? DEFAULT_MEANIE_SPEED_FACTOR : 1.0;
		if (spin_period != 0) phi += sign*360. * dt * meanie_factor / spin_period;
		if (phi >= 360.) phi -= 360.;
		if (phi < 0.) phi += 360.;
		relevant_progress = true;
	}
	
	if (state==E_MATTER_STATE::MANIFESTING ||
		state==E_MATTER_STATE::DISINTEGRATING  || state==E_MATTER_STATE::TRANSMUTING)
	{
		float sign = (state==E_MATTER_STATE::MANIFESTING ||
			state==E_MATTER_STATE::TRANSMUTING) ? +1.0 : -1.0;
		fade += sign * dt / fading_time;
		if (fade > 1)
		{
			if (state==E_MATTER_STATE::TRANSMUTING)
			{
				this->mesh_transmutation_origin = 0;
			}
			state = E_MATTER_STATE::STABLE;
			fade = 1.0;
		}
		if (fade <= 0)
		{
			state = E_MATTER_STATE::GONE;
			fade = 0.0;
		}
		relevant_progress = true;
	}
	return relevant_progress;
}

QVector3D Figure::get_eye_position_relative_to_figure(E_FIGURE_TYPE type)
{
	// These values are determined by looking at the .blend files
	// for the respective figure types.
	QVector3D res;
	switch (type)
	{
		case E_FIGURE_TYPE::ROBOT: res = QVector3D(0,0,1.45); break;
		case E_FIGURE_TYPE::SENTINEL: res = QVector3D(0.311, 0.0, 1.74); break;
		case E_FIGURE_TYPE::SENTRY: res = QVector3D(0.22, 0.0, 1.70); break;
		case E_FIGURE_TYPE::MEANIE: res = QVector3D(0.2, 0.0, 1.625); break;
		default: res = QVector3D(0,0,1.5);
	}
	return res;
}

QVector3D Figure::get_eye_position_in_world(QPoint pos, int altitude_base)
{
	QVector3D site((float)pos.x(),(float)pos.y(),(float)altitude_base);
	QVector3D eye = get_eye_position_relative_to_figure(type);
	QMatrix4x4 A; A.setToIdentity();
	A.rotate(phi,QVector3D(0,0,1));
	eye = A * eye;
	eye += site;
	return eye;
}

QVector3D Figure::get_direction()
{
	float phi = (this->phi)*PI/180;
	return QVector3D(cos(phi),sin(phi),0);
}

vector<Figure*> Figure::get_above_figure_stack()
{
	vector<Figure*> stack;
	Figure* next = this;
	while (next != 0)
	{
		stack.push_back(next);
		next = next->figure_above;
		if (stack.size() > 2000) throw "Insanely high stack! This is a bug.";
	}
	return stack;
}

vector<Figure*> Figure::get_below_figure_stack()
{
	vector<Figure*> stack;
	Figure* next = figure_below;
	while (next != 0)
	{
		stack.push_back(next);
		next = next->figure_below;
		if (stack.size() > 2000) throw "Insanely high stack! This is a bug.";
	}
	return stack;
}

Figure* Figure::get_top_figure()
{
	vector<Figure*> vec = get_above_figure_stack();
	vector<Figure*>::const_reverse_iterator CRI = vec.rbegin();
	Figure* res = *CRI;
	if (res == 0) throw "Top Figure* is 0.";
	return res;
}

int Figure::get_altitude_above_square()
{
	int alt = 0;
	vector<Figure*> stack = get_below_figure_stack();
	for (vector<Figure*>::const_iterator CI=stack.begin();CI!=stack.end();CI++)
	{
		alt += Figure::get_height((*CI)->get_type());
	}
	return alt;
}

int Figure::get_energy_for_robot()
{
	return absorption_triggered_by_robot ? get_energy_value(type) : 0;
}

int Figure::check_for_and_delete_top_figure(bool only_if_GONE)
{
	if (figure_above == 0) return 0; // Nothing to do.
	vector<Figure*> stack = get_above_figure_stack();
	vector<Figure*>::reverse_iterator RI = stack.rbegin();
	Figure* top = *RI;
	int res = 0;
	if (top->is_gone() || (!only_if_GONE))
	{
		RI++;
		Figure* below = *RI;
		below->figure_above = 0;
		res = top->get_energy_for_robot();
		delete top;
	}
	return res;
}

QString Figure::get_figure_name()
{
	QString res = "";
	switch (type)
	{
		case E_FIGURE_TYPE::TREE : res = QObject::tr("tree"); break;
		case E_FIGURE_TYPE::BLOCK : res = QObject::tr("boulder"); break;
		case E_FIGURE_TYPE::MEANIE : res = QObject::tr("meanie"); break;
		case E_FIGURE_TYPE::ROBOT : res = QObject::tr("robot"); break;
		case E_FIGURE_TYPE::SENTRY : res = QObject::tr("sentry"); break;
		case E_FIGURE_TYPE::SENTINEL : res = QObject::tr("Sentinel"); break;
		case E_FIGURE_TYPE::TOWER : res = QObject::tr("tower"); break;
		default: throw "Unknown type encountered.";
	}
	return res;
}

Figure::Figure(E_FIGURE_TYPE type, E_MATTER_STATE state, Mesh_Data* mesh,
	float phi, float theta, float spin_period, float fov, float fading_time)
{
	this->type = type;
	this->state = state;
	this->fade =
		(this->state == E_MATTER_STATE::MANIFESTING) ||
		(this->state == E_MATTER_STATE::GONE) ? 0.0 : 1.0;
	this->fading_time = fading_time;
	this->mesh = mesh;
	this->phi = phi;
	this->theta = theta;
	this->spin_period = spin_period;
	this->fov = fov;
	this->figure_above = 0;
	// Leave that as 0! For setting a figure below use the method set_figure_above()
	// from the intended base-figure!
	this->figure_below = 0;
	this->absorption_triggered_by_robot = false;
}

Figure::Figure(const Figure& orig) :
	Figure(orig.type, orig.state, orig.mesh, orig.phi, orig.theta, orig.spin_period,
		orig.fov, orig.fading_time)
{
	if (orig.figure_above != 0)
	{
		Figure* above = new Figure(*(orig.figure_above));
		this->set_figure_above(above);
	}
}

Figure::~Figure()
{
	delete this->figure_above;
	// No! Don't do this! Consider that we already delete this->figure_above.
	// Then we self-destruct. And the figure_above accesses us trying to set
	// this->figure_above to 0. Not good!
	// if (figure_below) figure_below->figure_above = 0;
}
//< ------------------------------------------------------------------

//> Square. ----------------------------------------------------------
QVector3D Square::qvec4_x_qvec4(QVector4D u, QVector4D v, bool normalize)
{
	QVector3D u_(u);
	QVector3D v_(v);
	QVector3D n = QVector3D::crossProduct(u_,v_);
	// Note that the Qt function QVector3D::normalize() sets 0.normalize() -> 0.
	if (normalize) n.normalize();
	return n;
}

vector<QVector3D> Square::get_sloped_normal_vectors(vector<QVector4D>& verts)
{
	if (verts.size() != 4) throw "Expected 4 vertices exactly.";
	vector<QVector3D> norm;
	norm.push_back(qvec4_x_qvec4(verts.at(1),verts.at(3),true));
	norm.push_back(qvec4_x_qvec4(verts.at(2),verts.at(0),true));
	norm.push_back(qvec4_x_qvec4(verts.at(3),verts.at(1),true));
	norm.push_back(qvec4_x_qvec4(verts.at(0),verts.at(2),true));
	return norm;
}

bool Square::turn_sloped_square_by_90_degrees_if_necessary(vector<Vertex_Data>& vertex_data)
{
	if (vertex_data.size() != 4) throw "Expected 4 vertex_data sets exactly.";
	//> Step 1: Determine if there is a flat-foot situation. ---------
	GLushort tr0[] = {
		mesh_prototype->elements.at(0),
		mesh_prototype->elements.at(1),
		mesh_prototype->elements.at(2)
	};
	GLushort tr1[] = {
		mesh_prototype->elements.at(3),
		mesh_prototype->elements.at(4),
		mesh_prototype->elements.at(5)
	};
	float heights[] =
	{
		vertex_data.at(0).vertex.z(),
		vertex_data.at(1).vertex.z(),
		vertex_data.at(2).vertex.z(),
		vertex_data.at(3).vertex.z()
	};
	// A candidate does not touch a diagonal. I.e.: Is not in both triangles at once.
	// In addition he is the only point on his height.
	bool found_flat_foot = false;
	for (GLushort j=0;j<4;j++)
	{
		// Question 1: Is the j on the intersection of both triangles?
		bool found_in_tr0 = false;
		bool found_in_tr1 = false;
		for (GLushort k=0;k<3;k++) if (tr0[k] == j) { found_in_tr0 = true; break; }
		for (GLushort k=0;k<3;k++) if (tr1[k] == j) { found_in_tr1 = true; break; }
		if (found_in_tr0 && found_in_tr1) continue; // Not flat foot candidate here.

		float a0 = heights[j];
		float a1 = heights[(j+1)%4];
		float a2 = heights[(j+2)%4];
		float a3 = heights[(j+3)%4];
		// Question 2: Do all other corners share the identical height?
		if (!(a1==a2 && a1==a3)) continue;
		// Question 3: Is the candidate the only one with his height?
		if (a0 == a1) continue;

		// Still here? We found a flat-foot!
		found_flat_foot = true;
		break;
	}
	if (!found_flat_foot) return false; // Nothing to do.
	//< --------------------------------------------------------------
	//> Step 2: Correct the flat-foot situation. ---------------------
	vertex_data.push_back(vertex_data.at(0));
	vertex_data.erase(vertex_data.begin());
	//< --------------------------------------------------------------
	return true;
}

void Square::set_altitude(int level, int x, int y)
{
	if (type == E_SQUARE_TYPE::CONNECTION) throw "attempt to set altitude for CONNECTION square.";
	if (type == E_SQUARE_TYPE::UNDEFINED) throw "attempt to set altitude for UNDEFINED square.";
	if (mesh_prototype == 0) throw "set_altitude needs a valid mesh_prototype.";
	this->level = level;
	float fx = (float)x;
	float fy = (float)y;
	float fz = (float)level;
	// Due to the 'black face' bug sloped face vertices must not have the
	// altitude 0.0. To prevent a 'levitating landscape' flat squares must not either.
	if (level == 0) level = 0.0001f;

	// As for the vertex positions. Simply use a translation matrix!
	QMatrix4x4 A;
	A.setToIdentity();
	A.translate(fx,fy,fz);
	this->vertices.clear();
	for (int j=0;j<4;j++)
	{
		this->vertices.push_back(Vertex_Data(
			A * mesh_prototype->vertices.at(j).vertex,
			QVector3D(0,0,1),
			mesh_prototype->vertices.at(j).tex_coord,	
			QVector4D(1,1,1,1)
		 ));
	}
}

void Square::set_sloped_altitudes(float alt_pp, float alt_mp, float alt_mm, float alt_pm,
	int x, int y)
{
	if (alt_pp < 0 || alt_mp <0 || alt_pm < 0 || alt_mm < 0)
	{
		cout << "pp: " << alt_pp << ", mp: " << alt_mp << ", pm: " << alt_pm << ", mm:" << alt_mm <<
		  " at (x=" << x << ",y=" << y << ")." << endl; throw "Invalid slope heights.";
	}
	if (type != E_SQUARE_TYPE::CONNECTION) throw "attempt to set sloped altitude for non-CONNECTION square.";
	if (mesh_prototype == 0) throw "set_sloped_altitudes requires a valid mesh_prototype.";
	this->level = -1;
	vector<QVector4D> verts;
	for (int j=0;j<4;j++)
	{
		QVector4D vert = mesh_prototype->vertices.at(j).vertex;
		// Note that (x,y) are all in { -0.5, 0.5 }
		// vert.setZ(0.);
		// The qMax(*,0.0001f) fixes the 'black face' bug.
		if (vert.x() < 0. && vert.y() < 0.) { vert.setZ(qMax(alt_mm,.0001f)); }
		if (vert.x() > 0. && vert.y() < 0.) { vert.setZ(qMax(alt_pm,.0001f)); }
		if (vert.x() < 0. && vert.y() > 0.) { vert.setZ(qMax(alt_mp,.0001f)); }
		if (vert.x() > 0. && vert.y() > 0.) { vert.setZ(qMax(alt_pp,.0001f)); }
		vert.setX( vert.x()+((float)(x)) );
		vert.setY( vert.y()+((float)(y)) );
		verts.push_back(vert);
	}
	vector<QVector3D> norm = get_sloped_normal_vectors(verts);

	this->vertices.clear();
	for (int j=0;j<4;j++)
	{
		this->vertices.push_back(Vertex_Data(
			verts.at(j),
			norm.at(j),
			mesh_prototype->vertices.at(j).tex_coord,
			QVector4D(1,1,1,1)
		 ));
	}
	turn_sloped_square_by_90_degrees_if_necessary(this->vertices);
}

bool Square::transfer_vertices_to_GPU()
{
	string caller = "Square::transfer_vertices_to_GPU()";
	E_DEBUG_LEVEL dl = E_DEBUG_LEVEL::WARNING;
	//> Actual transfer. ---------------------------------------------
	if (!buf_vertices.create())
	{
		if (io) io->println(dl, caller,
			"Failure to create buf_vertices.");
		return false;
	}
	if (buf_vertices.bind())
	{
		// vertices.size() would be 4 ...
		buf_vertices.allocate(&(vertices[0]), vertices.size() * sizeof(Vertex_Data));
		buf_vertices.release();
	} else {
		if (io) io->println(dl, caller,
			"Failure to bind buf_vertices to active openGL context.");
		return false;
	}
	//< --------------------------------------------------------------
	return true;
}
	
Square::Square(Io_Qt* io)
{
	this->type = E_SQUARE_TYPE::UNDEFINED;
	this->level = -1;
	this->mesh_prototype = 0;
	this->io = io;
	this->plateau_id = -1;
}

Square::~Square()
{
	if (buf_vertices.isCreated()) buf_vertices.destroy();
}
//< ------------------------------------------------------------------
//> Board. -----------------------------------------------------------
template <class T> T* Board<T>::get(int x, int y)
{
	if (x < 0 || y < 0 || x >= width || y >= height)
	{
		cout << "x: " << x << ", y: " << y << " not in board dimensions." << endl;
		throw "Out of range.";
	}
	return board[y*width+x];
}

template <class T> T* Board<T>::get(QPoint p)
{
	return get(p.x(),p.y());
}

template <class T> bool Board<T>::set(int x, int y, T* item)
{
	if (x < 0 || y < 0 || x >= width || y >= height) throw "Out of range.";
	board[y*width+x] = item;
	return true;
}

template <class T> bool Board<T>::set(QPoint p, T* item)
	{ return set(p.x(), p.y(), item); }

template <class T> string Board<T>::toString(bool dummy)
{
	ostringstream oss;
	for (int y=0; y<get_width(); y++)
	{
		for (int x=0; x<get_width(); x++)
		{
			oss << ((get(x,y)==0) ? '.' : '*');
		}
		oss << endl;
	}
	return oss.str();
}

template <> string Board<Square>::toString(bool by_plateau_id)
{
	ostringstream oss;
	for (int y=0; y<get_height(); y++)
	{
		for (int x=0; x<get_width(); x++)
		{
			Square* sq = get(x,y);
			if (sq == 0) { oss << '.'; } else {
				oss << (by_plateau_id ? sq->plateau_id : sq->get_altitude()) % 10;
			}
			
		}
		oss << endl;
	}
	return oss.str();
}

template <class T> Board<T>::Board(int width, int height, bool delete_contents_upon_destruction)
{
	this->width = width;
	this->height = height;
	this->delete_contents_upon_destruction = delete_contents_upon_destruction;
	board = 0;
	if (width > 0 && height > 0)
	{
		board = (T**)malloc(width*height*sizeof(T*));
		for (int j=0;j<width*height;j++)
		{
			board[j] = 0;
		}
	}
}

template <class T> Board<T>::~Board()
{
	if (delete_contents_upon_destruction)
	{
		for (int j=0;j<width*height;j++)
		{
			delete board[j];
		}
	}
	free (board);
}

//The explicit instantiation part
template class Board<Figure>; 
template class Board<Square>;
//< ------------------------------------------------------------------
//> Landscape. -------------------------------------------------------
vector<int> Landscape::random_permutation(int n)
{
	vector<int> perm(n,0);
	if (n==0) return perm; // Nothing to do.
	vector<bool> still_free(n,true);
	for (int val=0;val<n;val++)
	{
		int index = qrand() % n;
		if (still_free[index])
		{
			still_free[index] = false;
		} else {
			// Sanity counter just in case I goofed this function.
			int c=0;
			while (!still_free[index])
			{
				index = (index+1) % n;
				if (n<(c++))
				{
					throw "Bug encountered!";
				}
			}
			still_free[index] = false;
		}
		perm[index] = val;
	}
	return perm;
}

Mesh_Data* Landscape::get_mesh(E_FIGURE_TYPE type)
{
	Mesh_Data* res = 0;
	switch (type)
	{
		case E_FIGURE_TYPE::SENTINEL: res = mesh_sentinel; break;
		case E_FIGURE_TYPE::TOWER: res = mesh_sentinel_tower; break;
		case E_FIGURE_TYPE::SENTRY: res = mesh_sentry; break;
		case E_FIGURE_TYPE::TREE: res = mesh_tree; break;
		case E_FIGURE_TYPE::ROBOT: res = mesh_robot; break;
		case E_FIGURE_TYPE::BLOCK: res = mesh_block; break;
		case E_FIGURE_TYPE::MEANIE: res = mesh_meanie; break;
		default: throw "Non-figure mesh requested.";
	}
	return res;
}

float Landscape::get_board_diagonal_length()
{
	return sqrt(width*width+height*height);
}

Board<Figure>* Landscape::get_new_initialized_board_fg()
{
	Board<Figure>* board = new Board<Figure>(width, height, false);
	for (int y=0;y<height;y++)
	{
		for (int x=0;x<width;x++)
		{
			Figure* fg = initial_board_fg.get(x,y);
			if (fg!=0) board->set(x,y,new Figure(*fg));
		}
	}
	return board;
}

QPoint Landscape::get_player_starting_position()
{
	if (initial_robot_position.x() < 0 || initial_robot_position.x() >= width ||
		initial_robot_position.y() < 0 || initial_robot_position.y() >= height)
		throw "Invalid initial robot position.";
	return initial_robot_position;
}

int Landscape::get_altitude(int x, int y)
{
	Square* sq = board_sq.get(x,y);
	if (sq==0)
	{
		ostringstream oss;
		oss << "Requested coordinates x=" << x << ", y=" << y <<
			" do not exist on the board or the target pointer is zero:"
			" board(x,y) = "<< sq;
		if (io) io->println(E_DEBUG_LEVEL::WARNING,"Landscape::get_altitude(..)",
			oss.str());
		return -2;
	}
	return sq->get_altitude();
}

map<string,Square*> Landscape::get_adjacent_odd_even_squares(QPoint pos)
{
	map<string,Square*> res;
	vector<string> keys;
	keys.push_back("mm"); keys.push_back("_m"); keys.push_back("pm");
	keys.push_back("m_"); keys.push_back("__"); keys.push_back("p_");
	keys.push_back("mp"); keys.push_back("_p"); keys.push_back("pp");
	int key_index = 0;
	for (int y=pos.y()-1;y<=pos.y()+1;y++)
	{
		for (int x=pos.x()-1;x<=pos.x()+1;x++)
		{
			if (x>=0 && y>=0 && x<width && y<height)
			{
				// Ignore slope squares.
				Square* sq = board_sq.get(x,y);
				if (sq==0||(sq->get_type() != E_SQUARE_TYPE::ODD &&
					sq->get_type() != E_SQUARE_TYPE::EVEN))
				{
					res[keys.at(key_index)] =  0;
				} else {
					res[keys.at(key_index)] = board_sq.get(x,y);
				}
			} else {
				res[keys.at(key_index)] = 0;
			}
			key_index++;
		}
	}
	return res;
}

vector<QPoint> Landscape::get_possible_nucleus_list()
{
	vector<QPoint> res;
	for (int k=1;k<width-1;k++)
	{
		for (int j=1;j<height-1;j++)
		{
			bool could_be_good = true;
			for (int k0=qMax(0,k-1);k0<=qMin(width-1,k+1);k0++)
			{
				for (int j0=qMax(0,j-1);j0<=qMin(height-1,j+1);j0++)
				{
					if (board_sq.get(k0,j0)!=0)
					{
						could_be_good = false;
						break;
					}
				}
				if (!could_be_good) break;
			}
			if (could_be_good) res.push_back(QPoint(k,j));
		}
	}
	return res;
}

QPoint Landscape::get_random_sq_eligible_for_new_plateau_nucleus()
{
	vector<QPoint> possible = get_possible_nucleus_list();
	if (possible.size() == 0) return QPoint(-1,-1);
	return possible[qrand() % possible.size()];
}

bool Landscape::is_valid_new_plateau_square(QPoint pos, int plateau_id)
{
	int x0=pos.x();
	int y0=pos.y();
	// Target square is not even empty!
	if (board_sq.get(x0,y0)) return false;
	for (int x = qMax(0,x0-1); x<= qMin(width-1, x0+1); x++)
	{
		for (int y = qMax(0,y0-1); y<= qMin(height-1, y0+1); y++)
		{
			Square* sq = board_sq.get(x,y);
			if (sq && (sq->plateau_id != plateau_id)) return false;
		}
	}
	return true;
}

int Landscape::get_plateau_size(int plateau_id)
{
	int c = 0;
	for (int j=0;j<height;j++)
	{
		for (int k=0;k<width;k++)
		{
			Square* sq = board_sq.get(k,j);
			if (sq && sq->plateau_id == plateau_id) { c++; }
		}
	}
	return c;
}

vector<int> Landscape::get_altitudes_by_cubic_polynomial(
	int max_altitude, int n, float x0, float x1)
{
	vector<int> alt;
	alt.push_back(0);
	for (int k=1;k<n;k++)
	{
		float nominator_left = -x0+ ((float)k)*(x1+x0)/((float)(n-1));
		alt.push_back((int)round(((float)(max_altitude)) *
			(nominator_left*nominator_left*nominator_left+x0*x0*x0) /
			(x1*x1*x1+x0*x0*x0)));
	}
	return alt;
}

void Landscape::remove_altitude_gaps(vector<int>& altitudes)
{
	for (uint j=1;j<altitudes.size();j++)
	{
		int diff = altitudes.at(j) - altitudes.at(j-1);
		if (diff > 2) { altitudes[j] = altitudes.at(j)-diff+2; }
	}
}

void Landscape::set_square_type_depending_on_xy(int x, int y, Square* sq)
{
	if (sq == 0) throw "Null pointer exception.";
	sq->set_type
	(
		((x+y)%2)==0 ? E_SQUARE_TYPE::EVEN : E_SQUARE_TYPE::ODD,
		((x+y)%2)==0 ? mesh_even : mesh_odd
	);
}

vector<QPoint> Landscape::generate_nuclei()
{
	//> How many nuclei? ---------------------------------------------
	// A young world has a more rugged scenery.
	// Young: 10 squares per plateau, normal: 30 s/pp, aged: 50 s/pp.
	// Say that roughly 1/3 of the squares are connections.
	int n = 2*width*height/3;
	if (age <= 0) n/=40;
	if (age == 1) n/=30;
	if (age == 2) n/=18;
	if (age >= 3) n/=12;
	// Minimum: 2.
	if (n < 2) n=2;
	//< --------------------------------------------------------------
	//> Define these nuclei. -----------------------------------------
	vector<QPoint> nuclei;
	for (int j=0;j<n;j++)
	{
		QPoint cand = get_random_sq_eligible_for_new_plateau_nucleus();
		if (cand.x() == -1)
		{
			/** Obviously the desired number of nucleus sites was not found. */
			break;
		}
		Square* sq = new Square(io);
		sq->plateau_id = j;
		board_sq.set(cand,sq);
		nuclei.push_back(cand);
	}
	//< --------------------------------------------------------------
	return nuclei;
}

vector<QPoint> Landscape::get_plateau_positions(int plateau_id)
{
	vector<QPoint> res;
	for (int x=0;x<width;x++)
	{
		for (int y=0;y<height;y++)
		{
			Square* sq = board_sq.get(x,y);
			if (sq!=0 && sq->plateau_id == plateau_id)
			{
				res.push_back(QPoint(x,y));
			}
		}
	}
	return res;
}

void Landscape::expand_nuclei(int n, bool neglect)
{
	E_DEBUG_LEVEL dl = E_DEBUG_LEVEL::ERROR;
	string caller = "Landscape::expand_nuclei(..)";
	bool found_new_sq = true;
	int c=0;
	// The smaller such a block the faster neglection will progress.
	int block_size = qMax(1,(int)(0.02*(float)width*(float)height));
	// Repeat this loop until no plateau will grow any further.
	while (found_new_sq)
	{
		found_new_sq = false;
		if (neglect && (((++c) % block_size)==0)) n = (int)(0.8*(float)n);
		vector<int> plateau_ids = random_permutation(n);
		for (vector<int>::const_iterator CI=plateau_ids.begin();CI!=plateau_ids.end();CI++)
		{
			int plateau_id = *CI;
			// After this plateau has been expanded by 1 square move on.
			bool found_for_this_plateau = false;
			// Iterate randomly through all squares within this plateau.
			vector<QPoint> plateau_positions = get_plateau_positions(plateau_id);
			if (plateau_positions.size() == 0)
			{
				if (io) io->println(dl,caller,"Plateau id was not found at all. "
					"There should be at least the nucleus. This constitutes a bug!");
				throw "Bug encountered!";
			}
			vector<int> plateau_indecies = random_permutation(plateau_positions.size());
			vector<int> directions = random_permutation(4);
			for (vector<int>::const_iterator CJ=plateau_indecies.begin();CJ!=plateau_indecies.end();++CJ)
			{
				QPoint pos_orig = plateau_positions[*CJ];
				for (vector<int>::const_iterator CK = directions.begin();CK!=directions.end();CK++)
				{
					QPoint pos = pos_orig;
					switch (*CK)
					{
						case 0: pos.rx()++; break;
						case 1: pos.rx()--; break;
						case 2: pos.ry()++; break;
						case 3: pos.ry()--; break;
						default:
							if (io) io->println(dl, caller, "expand_nuclei()", "Invalid perm4.");
							throw "Bug encountered!";
					}
					// pos now points to a neighbouring square of the original pos.
					if (
						pos.x() > 0 && pos.x() < width-1 &&
						pos.y() > 0 && pos.y() < height-1 &&
						board_sq.get(pos) == 0 &&
						is_valid_new_plateau_square(pos, plateau_id))
					{
						Square* sq = new Square(io);
						sq->plateau_id = plateau_id;
						board_sq.set(pos,sq);
						found_for_this_plateau = true;
						// Something was found. Do not leave the while() just yet.
						found_new_sq = true;
						break;
					}
				}
				// if (found_for_this_plateau) move on to the next plateau id.
				if (found_for_this_plateau) break;
			}
		}
	}
}

map<int,int> Landscape::assign_altitudes(vector<QPoint>& nuclei)
{
	int n = nuclei.size();
	//> Step 1: Identify the smallest plateau for The Sentinel. ------
	int plateau_id_smallest = -1;
	{
		int plateau_smallest = -1;
		map<int,int> plateau_sizes;
		for (int j=0;j<n;j++)
		{
			plateau_sizes[j] = get_plateau_size(j);
			if (plateau_id_smallest == -1 || plateau_smallest > plateau_sizes.at(j))
			{
				plateau_id_smallest = j;
				plateau_smallest = plateau_sizes.at(j);
			}
		}
	}
	//< --------------------------------------------------------------
	//> Step 2: Determine maximum height. ----------------------------
	// height lives in { 0,..,n-1 }
	int max_height=0;
	switch (gravity)
	{
		case 0: max_height = 4; break;
		case 1: max_height = 6; break;
		case 2: max_height = 9; break;
		case 3: max_height = 13; break;
		case 4: max_height = 17; break;
		default: max_height = 18; // case >= 5
//		case 1: max_height = qMax(n/3, 4); break;
//		case 2: max_height = qMax(n/2, 5); break;
//		case 3: max_height = qMax(2*n/3, 6); break;
//		case 4: max_height = qMax(n-3, 7); break;
//		default: max_height = qMax(n-2, 8); // case >= 5
	}
	if (max_height >= (n-1)) max_height = n-2;
	if (max_height <= 1) max_height = 2;
	ostringstream oss;
	oss << "Maximum height will be " << max_height << " units,";
	if (io) io->println(E_DEBUG_LEVEL::VERBOSE, "assign_altitudes(..)",
		oss.str());
	this->peak_altitude = max_height;
	//< --------------------------------------------------------------
	//> Distribute the heights. --------------------------------------
	map<int,int> plateau_altitude;
	vector<int> alt = get_altitudes_by_cubic_polynomial(max_height, n, -.3, .5);
	remove_altitude_gaps(alt);
	for (int j=plateau_id_smallest;j<plateau_id_smallest+n;j++)
	{
		plateau_altitude[j%n] = alt.at(n-1-(j-plateau_id_smallest));
	}
	for (int x=0;x<width;x++)
	{
		for (int y=0;y<height;y++)
		{
			Square* sq = board_sq.get(x,y);
			if (sq)
			{
				set_square_type_depending_on_xy(x,y,sq);
				sq->set_altitude(plateau_altitude.at(sq->plateau_id),x,y);
			}
		}
	}
	//< --------------------------------------------------------------
	return plateau_altitude;
}

void Landscape::bridge_equi_contour_plateaus()
{
	bool found_any = true;
	while (found_any)
	{
		found_any = false;
		for (int x0=0;x0<width;x0++)
		{
			for (int y0=0;y0<height;y0++)
			{
				bool do_bridge = true;
				Square* sq = board_sq.get(x0,y0);
				if (sq == 0)
				{
					int suggested_plateau_id=-1;
					int alt=-1;
					// Bridge if all adjacent squares are either 0
					// or have identical height.
					for (int x=qMax(0,x0-1);x<=qMin(width-1,x0+1);x++)
					{
						for (int y=qMax(0,y0-1);y<=qMin(height-1,y0+1);y++)
						{
							Square* sq_neighbor = board_sq.get(x,y);
							if (sq_neighbor != 0)
							{
								if (alt==-1)
								{
									alt = sq_neighbor->get_altitude();
									suggested_plateau_id = sq_neighbor->plateau_id;
								} else {
									if ((alt != sq_neighbor->get_altitude()) ||
										((x0==0||y0==0||x0==width-1||y0==height-1) &&
										 sq_neighbor->get_altitude() > 0)
										)
									{
										do_bridge = false;
										break;
									}
								}
							}
						}
						if (!do_bridge) break;
					}
					// Last chance to avoid bridging: Corners with altitudes.
					// TODO: This block is a hack. Remove sometime and improve the code above!
					if (alt > 0 && (x0==0||x0==width-1||y0==0||y0==height-1))
					{
						do_bridge = false;
					}
					if (do_bridge && alt != -1)
					{
						sq = new Square(io);
						set_square_type_depending_on_xy(x0,y0,sq);
						sq->set_altitude(qMax(0,alt),x0,y0);
						sq->plateau_id = suggested_plateau_id;
						board_sq.set(x0,y0,sq);
					}
				}
			}
		}
	}
}

void Landscape::assign_odd_even()
{
	for (int x=0;x<width;x++)
	{
		for (int y=0;y<height;y++)
		{
			Square* sq = board_sq.get(x,y);
			if (sq!=0)
			{
				set_square_type_depending_on_xy(x,y,sq);
			}
		}
	}
}

void Landscape::assign_slopes()
{
	for (int x=0;x<width;x++)
	{
		for (int y=0;y<height;y++)
		{
			Square* sq = board_sq.get(x,y);
			if (sq == 0)
			{
				//> Step 1: Determine the 4 vertex altitudes. --------
				map<string,Square*> ad = get_adjacent_odd_even_squares(QPoint(x,y));
				// If there are no flat neighbors it is guaranteed by now
				// that the vertex is situated at the edge of the board.
				// In that case assign vertex altitude 0.
				//>> 'pp'. -------------------------------------------
				float alt_pp = 0;
				if (ad.at("p_") != 0)
				{
					alt_pp = ad.at("p_")->get_altitude();
				} else if (ad.at("_p") != 0)
				{
					alt_pp = ad.at("_p")->get_altitude();
				} else if (ad.at("pp") != 0)
				{
					alt_pp = ad.at("pp")->get_altitude();
				}
				//<< -------------------------------------------------
				//>> 'mp'. -------------------------------------------
				float alt_mp = 0;
				if (ad.at("_p") != 0)
				{
					alt_mp = ad.at("_p")->get_altitude();
				} else if (ad.at("m_") != 0)
				{
					alt_mp = ad.at("m_")->get_altitude();
				} else if (ad.at("mp") != 0)
				{
					alt_mp = ad.at("mp")->get_altitude();
				}
				//<< -------------------------------------------------
				//>> 'mm'. -------------------------------------------
				float alt_mm = 0;
				if (ad.at("_m") != 0)
				{
					alt_mm = ad.at("_m")->get_altitude();
				} else if (ad.at("m_") != 0)
				{
					alt_mm = ad.at("m_")->get_altitude();
				} else if (ad.at("mm") != 0)
				{
					alt_mm = ad.at("mm")->get_altitude();
				}
				//<< -------------------------------------------------
				//>> 'pm'. -------------------------------------------
				float alt_pm = 0;
				if (ad.at("p_") != 0)
				{
					alt_pm = ad.at("p_")->get_altitude();
				} else if (ad.at("_m") != 0)
				{
					alt_pm = ad.at("_m")->get_altitude();
				} else if (ad.at("pm") != 0)
				{
					alt_pm = ad.at("pm")->get_altitude();
				}
				//<< -------------------------------------------------
				//< --------------------------------------------------
				//> Step 2: Create the CONNECTION square. ------------
				sq = new Square(io);
				sq->plateau_id = -1;
				sq->set_type(E_SQUARE_TYPE::CONNECTION, mesh_connection);
				sq->set_sloped_altitudes(alt_pp, alt_mp, alt_mm, alt_pm, x, y);
				board_sq.set(x,y,sq);
				//< --------------------------------------------------
			}
		}
	}
}

vector<QPoint> Landscape::get_coordinates_by_altitude(int min_altitude, int max_altitude)
{
	vector<QPoint> res;
	for (int y=0;y<height;y++)
	{
		for (int x=0;x<width;x++)
		{
			Square* sq = this->board_sq.get(x,y);
			int altitude = sq ? sq->get_altitude() : -1;
			if (altitude >= min_altitude && (max_altitude==-1 || altitude <=max_altitude))
			{
				res.push_back(QPoint(x,y));
			}
		}
	}
	return res;
}

float Landscape::get_random_angle()
{
	return (float)(qrand() % 360);
}

QPoint Landscape::pick_initially_free_random_square(vector<QPoint>& squares_by_height)
{
	QPoint res = QPoint(-1,-1);
	int n=squares_by_height.size();
	if (n == 0) return res;
	int index_offset = qrand() % n;
	vector<QPoint>::const_iterator CI = squares_by_height.begin();
	for (int j=0;j<index_offset;j++) CI++;

	int c=0;
	for (int j=0;j<n;j++)
	{
		QPoint site = *CI;
		if (initial_board_fg.get(site.x(),site.y()))
		{ // Square already taken. Move on.
			CI++;
			if (CI == squares_by_height.end()) { CI = squares_by_height.begin(); }
			if (c++ >= n) return res; // No free squares found.
		} else { // Free square found!
			return site;
		}
	}
	return res;
	
}

float Landscape::get_random_spin_sign()
{
	float sign = 1.0;
	switch (randomized_spin)
	{
		case 1: sign = -1.0; break;
		case 2: sign = ((float)(qrand() % 2))*2.0-1.0; break;
		default: break;
	}
	return sign;
}

bool Landscape::place_figure_on_free_random_square(E_FIGURE_TYPE type,
	Mesh_Data* mesh, vector<QPoint>& squares_by_height, QPoint& site_feedback)
{
	site_feedback = this->pick_initially_free_random_square(squares_by_height);
	if (site_feedback.x() == -1) return false;
	Figure* tpl = new Figure(type, E_MATTER_STATE::STABLE,
		mesh, get_random_angle(), 90,
		get_random_spin_sign()*spin_period, fov, fading_time);
	initial_board_fg.set(site_feedback,tpl);
	return true;
}

void Landscape::distribute_objects()
{
	//> Robot. -------------------------------------------------------
	{
		QPoint site;
		vector<QPoint> squares_by_height = get_coordinates_by_altitude(0,0);
		if(!place_figure_on_free_random_square(E_FIGURE_TYPE::ROBOT,
			mesh_robot, squares_by_height,site))
		{
			throw "No base square found for robot. This constitutes a bug.";
		}
		initial_robot_position = site;
	}
	//< --------------------------------------------------------------
	//> The Sentinel and his tower. ----------------------------------
	{
		QPoint site;
		vector<QPoint> squares_by_height = get_coordinates_by_altitude(peak_altitude, peak_altitude);
		if(!place_figure_on_free_random_square(E_FIGURE_TYPE::TOWER,
			mesh_sentinel_tower, squares_by_height, site))
		{
			throw "No peak square found for The Sentinel. This constitutes a bug.";
		}
		Figure* tpl = new Figure(E_FIGURE_TYPE::SENTINEL, E_MATTER_STATE::STABLE,
			mesh_sentinel, get_random_angle(), 90,
				get_random_spin_sign()*spin_period, fov, fading_time*2.5);
		initial_board_fg.get(site)->set_figure_above(tpl);
	}
	//< --------------------------------------------------------------
	//> Sentries. ----------------------------------------------------
	if (sentries)
	{
		QPoint site;
		int min = (int)ceil(((float)peak_altitude)*2./3.);
		vector<QPoint> squares_by_height = get_coordinates_by_altitude(min,peak_altitude);
		for (int j=0;j<sentries;j++)
		{
			place_figure_on_free_random_square(E_FIGURE_TYPE::SENTRY,
				mesh_sentry, squares_by_height, site);
		}
	}
	//< --------------------------------------------------------------
	//> Trees. -------------------------------------------------------
	if (trees)
	{
		QPoint site;
		int max_rich = (int)floor(((float)peak_altitude)*1./2.);
		int max_poor = (int)floor(((float)peak_altitude)*3./4.);
		for (int level=0;level<=max_poor;level++)
		{
			vector<QPoint> squares_by_height = get_coordinates_by_altitude(level,level);
			float trees_per_sq = level > max_rich ? .6 : 1.2;
			trees_per_sq *= ((float)(trees))/200.;
			float n = squares_by_height.size() * trees_per_sq;
			
			for (int j=0;j<((int)n);j++)
			{
				place_figure_on_free_random_square(E_FIGURE_TYPE::TREE,
					mesh_tree, squares_by_height, site);
			}
		}
	}
	//< --------------------------------------------------------------
}

void Landscape::send_board_sq_to_GPU()
{
	for (int x=0;x<width;x++)
	{
		for (int y=0;y<height;y++)
		{
			Square* sq = board_sq.get(x,y);
			if (!sq) throw "Attempt to send 0 pointer square to GPU.";
			sq->transfer_vertices_to_GPU();
		}
	}
}

void Landscape::generate_landscape()
{
	string caller = "Landscape::generate_landscape()";
	ostringstream oss;
	oss << "Generating landscape using random seed " << seed << "." << endl;
	vector<QPoint> nuclei = generate_nuclei();
	oss << "Added " << nuclei.size() << " plateau nuclei." << endl;
	
	expand_nuclei(nuclei.size(),true);
	oss << "Expanding nuclei, neglection step." << endl << this->board_sq.toString().c_str() << endl;
	expand_nuclei(nuclei.size(),false);
	oss << "Expanding nuclei, thorough step." << endl <<
	  this->board_sq.toString().c_str() << endl << "Assigning plateau heights." << endl;

	assign_altitudes(nuclei);
	oss << board_sq.toString(false).c_str() << endl << "Bridging gaps." << endl;
	
	bridge_equi_contour_plateaus();
	oss << this->board_sq.toString(false).c_str() << endl;
	
	assign_odd_even();
	oss << "Assigning ODD-EVEN info to flat squares." << endl;
	
	assign_slopes();
	oss << "Assigning CONNECTION slope tiles." << endl;
	
	distribute_objects();

	send_board_sq_to_GPU();
	oss << "Conveying board data to GPU." << endl;
	
	if (io) io->println(E_DEBUG_LEVEL::VERBOSE, "generate_landscape()", oss.str());
}

Landscape::Landscape(uint seed, int width, int height, int gravity, int age,
	float spin_period, float fov, float fading_time,
	int trees, int sentries, int randomized_spin,
	Io_Qt* io, Mesh_Data* mesh_connection,
	Mesh_Data* mesh_odd, Mesh_Data* mesh_even,
	Mesh_Data* mesh_sentinel, Mesh_Data* mesh_sentinel_tower,
	Mesh_Data* mesh_sentry, Mesh_Data* mesh_tree, Mesh_Data* mesh_robot,
	Mesh_Data* mesh_block, Mesh_Data* mesh_meanie
	) :
		initial_board_fg(qMax<int>(4,width), qMax<int>(4,height), true),
		board_sq(qMax<int>(4,width), qMax<int>(4,height), true),
		initial_robot_position(-1,-1)
{
	this->io = io;
	this->seed = seed;
	qsrand(seed);
	//if (seed > (uint)0) qsrand(seed);
	this->gravity = gravity;
	this->age = age;
	this->spin_period = spin_period;
	this->fov = fov;
	this->fading_time = fading_time;
	this->trees = trees;
	this->sentries = sentries;
	this->randomized_spin = randomized_spin;
	this->width = board_sq.get_width();
	this->height = board_sq.get_height();
	
	if (!(mesh_connection && mesh_odd && mesh_even && mesh_sentinel &&
		mesh_sentinel_tower && mesh_sentry && mesh_tree && mesh_robot))
	{
		throw "All mesh pointers are required as valid.";
	}

	this->mesh_connection = mesh_connection;
	this->mesh_odd = mesh_odd;
	this->mesh_even = mesh_even;
	this->mesh_sentinel = mesh_sentinel;
	this->mesh_sentinel_tower = mesh_sentinel_tower;
	this->mesh_sentry = mesh_sentry;
	this->mesh_tree = mesh_tree;
	this->mesh_robot = mesh_robot;
	this->mesh_block = mesh_block;
	this->mesh_meanie = mesh_meanie;
	this->peak_altitude = 0;
	
	this->generate_landscape();
}
//< ------------------------------------------------------------------
}
