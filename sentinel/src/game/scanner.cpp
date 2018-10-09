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
#include "scanner.h"

using std::pair;

//> DEBUGGING CODE. ---
#include <iostream>
using std::cout;
using std::endl;
//< DEBUGGING CODE. ---

namespace game
{
float Scanner::det_2x2(float m11, float m12, float m21, float m22)
{
	return m11*m22 - m12*m21;
}

bool Scanner::angle_larger_than(QVector2D a, QVector2D b, float min_angle)
{
	// Note: normalize will leave the 0 vector as 0. Suits us fine if
	// we want small angles for the 0 vector.
	a.normalize();
	b.normalize();
	float dp = QVector2D::dotProduct(a,b);
	float alpha = acos(dp)*180/PI;
	return alpha > min_angle;
}

bool Scanner::inv_2x2(float in11, float in12, float in21, float in22,
		float& out11, float& out12, float& out21, float& out22,
		float min_angle)
{
	QVector2D a(in11,in21);
	QVector2D b(in12,in22);
	if (!angle_larger_than(a,b,min_angle)) return false;
	float det = det_2x2(in11,in12,in21,in22);
	out11 = in22/det;
	out12 = -in12/det;
	out21 = -in21/det;
	out22 = in11/det;
	return true;
}

QMatrix4x4 Scanner::get_complete_transformation_for_figure(
	Figure* figure, QPoint board_pos, int altitude_square, const QMatrix4x4& camera)
{
	float phi = figure->get_phi();
	QMatrix4x4 A; A = camera;
	A.translate((float)board_pos.x(),(float)board_pos.y(),(float)altitude_square);
	A.rotate(phi,QVector3D(0,0,1));
	return A;
}

QPoint Scanner::get_board_pos_from_QVector3D(QVector3D& vec)
{
	return QPoint((int)round(vec.x()),(int)round(vec.y()));
}

bool Scanner::is_in_triangle(float mouse_gl_x, float mouse_gl_y,
	QVector4D a, QVector4D b, QVector4D c)
{
	b -= a;
	c -= a;
	// Question: are there lambda0, lambda1 in [0,1] with lambda0+lambda1<=1 and
	// (x,y) = a+b*lambda0+c*lambda1? Ignoring all z and w values for that one!
	//
	// [ x - a1 ]   [ b1 c1 ] [ lambda0 ]
	// [ y - a2 ] = [ b2 c2 ] [ lambda1 ] <=> v=A*l
	// We want A^(-1)*v.
	// Hence I'd like to have an inverse of A. Given that b and c are independent
	// or the problem is trivial (no projection on a malformed triangle)
	// I dare an inversion of QMatrix2x2.
	// Note: If it is not invertible the triangle has its span into the
	// z dimension and appears as a line on the screen and hence is not clickable.
	float inv11, inv12, inv21, inv22;
	bool invertible = inv_2x2(b.x(),c.x(),b.y(),c.y(),inv11,inv12,inv21,inv22,.02);
	if (!invertible) return false;
	float v1 = mouse_gl_x - a.x();
	float v2 = mouse_gl_y - a.y();
	float lambda0 = inv11*v1 + inv12*v2;
	float lambda1 = inv21*v1 + inv22*v2;
	return (lambda0 >= 0 && lambda1 >= 0 && lambda0 + lambda1 <= 1);
}

bool Scanner::is_figure_under_xray_mouse(float mouse_gl_x, float mouse_gl_y,
	Figure* figure, const QMatrix4x4& A)
{
	// Copy the prototype meshes from the figure. Never modify the latter!
	vector<Vertex_Data> vertex_data(figure->get_mesh_prototype()->vertices);
	vector<GLushort> elements(figure->get_mesh_prototype()->elements);
	
	// Remember: Each triplet of elements makes up a triangle!
	if (elements.size()%3!=0) throw "Apparently I made a wrong assumption here...";
	
	vector<QVector4D> vertices;
	for (uint j=0;j<vertex_data.size();j++)
	{
		QVector4D vec = A*(vertex_data[j].vertex);
		// http://stackoverflow.com/questions/30320144/perspectivelookat-transformation-in-qt-opengl-behaving-unexpectedly-not-even-ke/30320197#30320197
		vec /= vec.w();
		vertices.push_back(vec);
	}
	
	// n is the number of triangles. See: http://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_05#Elements_-_Index_Buffer_Objects_.28IBO.29
	uint n = elements.size() / 3;
	// Check each triangle whether or not it is under the mouse.
	for (uint j=0;j<n;j++)
	{
		QVector4D a = vertices[elements[3*j]];
		QVector4D b = vertices[elements[3*j+1]];
		QVector4D c = vertices[elements[3*j+2]];
	if (a.z()<-1. || a.z()>1. || b.z()<-1. || b.z()>1. ||
		c.z()<-1. || c.z()>1.) return false;
		if (is_in_triangle(mouse_gl_x,mouse_gl_y,a,b,c)) return true;
	}
	return false;
}

bool Scanner::is_square_under_xray_mouse(float mouse_gl_x, float mouse_gl_y,
	Square* square, const QMatrix4x4& camera)
{
	vector<Vertex_Data> vertex_data(square->vertices);
	// Note that these vertices are already in world coordinates.
	// Note that this shape is _not_ a kite due to the perspectivial distortion.
	QVector4D a = camera * square->vertices[0].vertex;
	QVector4D b = camera * square->vertices[1].vertex;
	QVector4D c = camera * square->vertices[2].vertex;
	QVector4D d = camera * square->vertices[3].vertex;
	// http://stackoverflow.com/questions/30320144/perspectivelookat-transformation-in-qt-opengl-behaving-unexpectedly-not-even-ke/30320197#30320197
	a /= a.w();
	b /= b.w();
	c /= c.w();
	d /= d.w();
	if (a.z()<-1. || a.z()>1. || b.z()<-1. || b.z()>1. ||
		c.z()<-1. || c.z()>1. || d.z()<-1. || d.z()>1.) return false;
	return (is_in_triangle(mouse_gl_x,mouse_gl_y,a,b,c) ||
		is_in_triangle(mouse_gl_x,mouse_gl_y,a,c,d));
}

QVector3D Scanner::get_mouse_direction(float mouse_gl_x, float mouse_gl_y,
	Viewer_Data* vd)
{
	QVector3D dir = vd->get_direction();
//cout << "x: " << mouse_gl_x << ", " << mouse_gl_y << endl;
//cout << "vorher: " << dir.x() << ", " << dir.y() << ", " << dir.z()  << endl; 
	//> Rotation with respect to phi. --------------------------------
	{
		float h_fov = vd->get_fov_h();
		float alpha = -mouse_gl_x * h_fov / 2.;
		QMatrix4x4 rot; rot.setToIdentity();
		rot.rotate(alpha,QVector3D(0,0,1));
		dir = rot*dir;
	}
	//< --------------------------------------------------------------
	//> Rotation with respect to theta. ------------------------------
	{
		float v_fov = vd->get_opening();
		float dtheta = -mouse_gl_y * v_fov / 2.;
		if (dir.x() == 0 && dir.y() == 0)
		{
			dir = dir.z() > 0 ?
				QVector3D(sin(dtheta),0,cos(dtheta)) :
				QVector3D(-sin(dtheta),0,cos(dtheta));
		} else {
			QVector3D axis(-dir.y(),dir.x(),0);
			QMatrix4x4 rot; rot.setToIdentity();
			rot.rotate(dtheta,axis);
			dir = rot*dir;
//cout << "nachher: " << dir.x() << ", " << dir.y() << ", " << dir.z()  << endl; 
			//QVector3D d2 = dir;
			//float theta = acos(d2.z());
			//d2.setX(d2.x()/sin(theta));
			//d2.setY(d2.y()/sin(theta));
			//d2.normalize();
			//theta += dtheta*PI/180;
			//d2.setX(d2.x()*sin(theta));
			//d2.setY(d2.y()*sin(theta));
			//d2.setZ(cos(theta));
			//dir = d2;
		}
	}
	//< --------------------------------------------------------------
	dir.normalize();
	return dir;
}

multimap<float, QPoint> Scanner::get_all_board_positions_in_h_fov(
	QVector3D eye, QVector3D dir, float h_fov, int width, int height)
{
	multimap<float, QPoint> res;
	if (h_fov < 0) throw "Negative field of view was given.";
	//> Getting left and right view directions. ----------------------
	// Normalize dir such that |(x,y)|==1.0.
	{
		float h = sqrt(dir.x()*dir.x()+dir.y()*dir.y());
		if (h==0) return res;
		dir.setX(dir.x()/h);
		dir.setY(dir.y()/h);
		dir.setZ(dir.z()/h);
	}
	float alpha = h_fov / 2.;
	float sa = sin(alpha*PI/180.);
	if (sa < 0) sa = -sa;
	QMatrix4x4 A; A.setToIdentity();
	A.rotate(-alpha, QVector3D(0,0,1));
	QVector3D dir_left = A*dir;
	A.rotate(h_fov, QVector3D(0,0,1)); // Inverts previous A _and_ turns on to +alpha! :-)
	QVector3D dir_right = A*dir;
	// Note that dir_1.z() == 0.
	QVector3D dir_1 = dir_right-dir_left;
	dir_1.normalize();
	//< --------------------------------------------------------------
	//> Building the field. ------------------------------------------
	float step = 1./sqrt(2.);
	int c_lambda0 = 0;
	int c_lambda1 = 0;
	QPoint first = get_board_pos_from_QVector3D(eye);
	while (true) // Looping through lambda0
	{
		QVector3D eye_plus_left = eye + ((float)c_lambda0)*step*dir_left;
//		if ((float)c_lambda0*step > 2. && eye_plus_left.z() < -step) break;
		bool found_any_valid = false;
		while (true) // Looping through lambda1
		{
			QVector3D current = eye_plus_left + ((float)c_lambda1)*step*dir_1;
			QPoint next = get_board_pos_from_QVector3D(current);
			if (next == first || next.x() < 0 || next.x() >= width ||
				next.y() < 0 || next.y() >= height)
			{
				if ((float)c_lambda1 <= 2.*sa*((float)c_lambda0))
				{
					// Never stop unless the full line sweep was completed.
					c_lambda1++;
					continue;
				}
			}
			if ((float)c_lambda1 > 2.*sa*((float)c_lambda0)) break;
			found_any_valid = true;
			float dx = ((float)(next.x()))-eye.x();
			float dy = ((float)(next.y()))-eye.y();
			float dist = dx*dx+dy*dy;
			bool is_new = true;
			{
				// Check whether the new value is really new. If not so ignore it.
				pair<multimap<float,QPoint>::const_iterator,
					multimap<float,QPoint>::const_iterator> range = res.equal_range(dist);
				for (multimap<float,QPoint>::const_iterator CI=range.first;
					CI!=range.second; ++CI)
				{
					if (CI->second == next) { is_new = false; break; }
				}
			}
			if (is_new)
			{
				res.insert(pair<float,QPoint>(dist,next));
			}
			c_lambda1++;
		}
		if ((!found_any_valid) && (float)c_lambda0*step > 2.) break;
		c_lambda1 = 0;
		c_lambda0++;
	}
	//< --------------------------------------------------------------
	return res;
}

multimap<float,QPoint> Scanner::get_all_board_positions_in_h_fov(
	float mouse_gl_x, float mouse_gl_y, Viewer_Data* viewer_data,
	float h_fov, int width, int height)
{
	QVector3D dir = get_mouse_direction(mouse_gl_x, mouse_gl_y, viewer_data);
	QVector3D eye = viewer_data->get_site();
	return get_all_board_positions_in_h_fov(eye,dir,h_fov, width, height);
}

vector<QPoint> Scanner::convert_ordered_multimap_to_vector(multimap<float,QPoint>& mm)
{
	vector<QPoint> res;
	for (multimap<float,QPoint>::const_iterator CI=mm.begin();CI!=mm.end();CI++)
	{
		res.push_back(CI->second);
	}
	return res;
}

vector<QPoint> Scanner::restrict_to_squares_under_xray_mouse(float mouse_gl_x,
		float mouse_gl_y, vector<QPoint> candidates, Landscape* landscape,
		const QMatrix4x4& camera, bool stop_after_first)
{
	vector<QPoint> res;
	for (vector<QPoint>::const_iterator CI=candidates.begin();CI!=candidates.end();CI++)
	{
		QPoint current = *CI;
		Square* sq = landscape->get_board_sq()->get(current);
		if (sq==0) throw "0 pointer encountered.";
		if (is_square_under_xray_mouse(mouse_gl_x,mouse_gl_y,sq,camera))
		{
			res.push_back(current);
			if (stop_after_first) break;
		}
	}
	return res;
}

QPoint Scanner::get_square_under_mouse(float mouse_gl_x, float mouse_gl_y,
	vector<QPoint>& candidates, Landscape* landscape, Viewer_Data* viewer_data)
{
	vector<QPoint> restricted = restrict_to_squares_under_xray_mouse(
		mouse_gl_x,
		mouse_gl_y,
		candidates,
		landscape,
		viewer_data->get_camera(),
		true // Only report the closest hit.
	);
	return (restricted.size() == 0) ? QPoint(-1,-1) : restricted[0];
}

vector<QPoint_Figure> Scanner::get_all_stable_figures_in_line(
	QPoint board_pos_under_mouse,
	vector<QPoint>& board_positions_in_line,
	Board<Figure>* board_fg)
{
	vector<QPoint_Figure> res;
	for (vector<QPoint>::const_iterator CI=board_positions_in_line.begin();
		CI!=board_positions_in_line.end();CI++)
	{
		QPoint pos = *CI;
		// Do not get any object on the square under mouse!
		if (pos == board_pos_under_mouse) break;
		Figure* figure = board_fg->get(pos);
		if (!figure) continue;
		vector<Figure*> stack = figure->get_above_figure_stack();
		for (vector<Figure*>::const_iterator CI2=stack.begin();CI2!=stack.end();CI2++)
		{
			if ((*CI2)->get_state() == E_MATTER_STATE::STABLE)
			{
				res.push_back(QPoint_Figure(pos,*CI2));
			}
		}
	}
	return res;
}

QPoint_Figure Scanner::get_figure_under_mouse(float mouse_gl_x, float mouse_gl_y,
		Landscape* landscape, vector<QPoint_Figure>& candidates,
		QMatrix4x4& camera)
{
	for (vector<QPoint_Figure>::const_iterator CI=candidates.begin();
			CI!=candidates.end();CI++)
	{
		QPoint pos = CI->pos;
		Figure* figure = CI->fig;
		QMatrix4x4 complete_transformation;
		float altitude = ((float)landscape->get_altitude(pos.x(),pos.y())) +
			((float)figure->get_altitude_above_square());
		float phi = figure->get_phi();
		complete_transformation = camera;
		complete_transformation.translate((float)pos.x(),(float)pos.y(),altitude);
		complete_transformation.rotate(phi,QVector3D(0,0,1));
		// Note: At this moment in time no xray is necessary since
		// all figures are _befor_ the board_pos_under_mouse used during
		// get_all_stable_figures_in_line(..).
		if (is_figure_under_xray_mouse(mouse_gl_x, mouse_gl_y, figure, complete_transformation))
		{
			return QPoint_Figure(pos,figure);
		}
	}
	return QPoint_Figure(); // Nothing found.
}


// DEBUGGING CODE. REMOVE LATER ON. ---
/*
namespace
{
void print_mm(multimap<float,QPoint> mm)
{
cout << "Got the following field of view: " << endl;
for (multimap<float,QPoint>::const_iterator CI=mm.begin(); CI!=mm.end();CI++)
{
	float dist = CI->first;
	QPoint pos=CI->second;
	cout << "[d=" << dist << "; (" << pos.x() << ", " << pos.y() << "); ";
}
cout << endl;
}
}
*/
// ------------------------------------

void Scanner::get_mouse_target(float mouse_gl_x, float mouse_gl_y,
		Viewer_Data* viewer_data, Landscape* landscape,
		Board<Figure>* board_fg, QPoint player_board_pos,
		QPoint& board_pos, Figure*& figure)
{
	//> First get all squares' board coords within line of sight. ----
//	vector<QPoint> candidates = get_all_board_positions_in_line(
//		mouse_gl_x, mouse_gl_y,
//		viewer_data,
//		landscape->get_width(),
//		landscape->get_height()
//	);
	multimap<float,QPoint> mm = get_all_board_positions_in_h_fov(
			mouse_gl_x, mouse_gl_y,
			viewer_data,
			viewer_data->get_fov_h(),
			landscape->get_width(),
			landscape->get_height()
	);
//cout << "FOV: " << viewer_data->get_fov_h() << endl;
	vector<QPoint> candidates = convert_ordered_multimap_to_vector(mm);
	
//print_mm(mm);
//cout << "Using the following points: "<< endl;
//for (uint j=0;j<candidates.size();j++)
//	cout << "(" << candidates[j].x() << "," << candidates[j].y() << "), ";
	//< --------------------------------------------------------------
	//> Get the closest square under mouse if any. -------------------
	board_pos = get_square_under_mouse(
		mouse_gl_x, mouse_gl_y,
		candidates,
		landscape,
		viewer_data);
	//< --------------------------------------------------------------
	//> Get the closest figure if any. -------------------------------
	vector<QPoint_Figure> fig_line = get_all_stable_figures_in_line(
		board_pos,
		candidates,
		board_fg
	);
	QMatrix4x4 camera = viewer_data->get_camera();
	QPoint_Figure qpf = get_figure_under_mouse(
		mouse_gl_x, mouse_gl_y,
		landscape,
		fig_line,
		camera
	);
	if (qpf.fig != 0)
	{
		board_pos = qpf.pos;
		figure = qpf.fig;
	}
//cout << "Square: " << board_pos.x() << ", " << board_pos.y() << "), figure pointer: " << figure << endl;
	//< --------------------------------------------------------------
}

int Scanner::get_block_stack_height(Figure* figure)
{
	int c=0;
	while (figure && figure->get_type()==E_FIGURE_TYPE::BLOCK)
	{
		c++;
		figure = figure->get_above_figure();
		if (c>1000) throw "Circular stack detected?";
	}
	return c;
}

bool Scanner::can_see_square(Landscape* landscape,
	QVector3D eye, QPoint target_sq, float alt_target_sq, bool can_see_from_below)
{
//cout << "alt_target: " << alt_target_sq << ", alt_self: " << eye.z() << endl;
	if ((!can_see_from_below) && alt_target_sq >= eye.z()) return false;
	int width = landscape->get_width();
	int height = landscape->get_height();
	QVector3D target((float)target_sq.x(),(float)target_sq.y(),alt_target_sq);
	QVector3D dir = target - eye;
	int steps = 100;
	dir.setX(dir.x()/(float)steps);
	dir.setY(dir.y()/(float)steps);
	dir.setZ(dir.z()/(float)steps);
	QVector3D current = eye;
	for (int j=1;j<steps;j++)
	{
		current += dir;
		QPoint pos = get_board_pos_from_QVector3D(current);
		if (pos == target_sq ||
			pos.x()<0 || pos.x()>=width || pos.y()<0 || pos.y()>=height) return true;
		Square* sq = landscape->get_board_sq()->get(pos);
		if (sq->get_altitude() >= current.z()) return false;
	}
	return true;
}

vector<Antagonist_target> Scanner::get_antagonist_targets(QVector3D eye,
		QVector2D direction_2D, float fov_horizontal, Landscape* landscape,
		Board<Figure>* board_fg, bool seek_trees)
{
	QVector3D direction(direction_2D.x(),direction_2D.y(),0);
	multimap<float,QPoint> mm = get_all_board_positions_in_h_fov(
		eye,
		direction,
		fov_horizontal,
		landscape->get_width(),
		landscape->get_height()
	);
	vector<QPoint> view = convert_ordered_multimap_to_vector(mm);
	vector<Antagonist_target> res;
	for (vector<QPoint>::const_iterator CI=view.begin();CI!=view.end();CI++)
	{
		QPoint site = *CI;
		Figure* base = board_fg->get(site);
		if (!base) continue;
		E_FIGURE_TYPE base_type = base->get_type();
		bool tree_on_top = base->get_top_figure()->get_type() == E_FIGURE_TYPE::TREE;
		if (
			(seek_trees && !tree_on_top) ||
			( (!seek_trees) && base_type != E_FIGURE_TYPE::BLOCK &&
				base_type != E_FIGURE_TYPE::ROBOT )
		) continue;
		// If the atagonist can see alt_square and is above it visibility is FULL.
		// If he is below it visibility is PARTIAL at best unless number_of_blocks > 0.
		int number_of_blocks = get_block_stack_height(base);
		Figure* figure = base->get_top_figure();
		if ((!figure->is_stable()) && figure->get_state()!=E_MATTER_STATE::DISINTEGRATING)
			continue; // Attack stable and disintegrating objects only.
		E_FIGURE_TYPE type = figure->get_type();
		if (type == E_FIGURE_TYPE::BLOCK) number_of_blocks--; // Don't count the target itself.
		float alt_target_feet = (float)(landscape->get_altitude(site.x(),site.y()) +
			number_of_blocks);
		bool can_see_sq = can_see_square(
			landscape,
			eye,
			site,
			(float)(landscape->get_altitude(site.x(),site.y())),
			false
		);
		E_VISIBILITY vis = E_VISIBILITY::HIDDEN;
		if (can_see_sq) // Anything goes if the square itself is visible!
		{
			vis = E_VISIBILITY::FULL;
		} else {
			// If the feet are visible when the square is not then
			// obviously these feet stand on a block and may
			// be interacted with from below.
			bool can_see_feet = can_see_square(
				landscape,
				eye,
				site,
				alt_target_feet,
				true
			);
			if (can_see_feet)
			{
				vis = E_VISIBILITY::FULL;
			} else {
				// If the feet are invisible but the body is seen it is in partial cover.
				bool can_see_body = can_see_square(
					landscape,
					eye,
					site,
					alt_target_feet+ ((float)(Figure::get_mesh_height(figure->get_type()))),
					true
				);
				if (can_see_body) vis = E_VISIBILITY::PARTIAL;
			}
		}
		if (vis != E_VISIBILITY::HIDDEN)
			res.push_back(Antagonist_target(vis,site));
	}
	return res;
}

Scanner::Scanner(Io_Qt* io)
{
	this->io = io;
}
}
