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
#include <sstream>
#include "game.h"

// DEBUGGING CODE!
#include <iostream>
using std::cout;
using std::endl;
// ---------------

using std::ostringstream;
using std::pair;


namespace game
{
// In order to maintain invertibility please do not use values == 0.0.
const QVector4D Game::hyperspace_light_factor(130./256.,112./256.,114./256.,1);
const QVector4D Game::light_attack_light_factor(256./256., 186./256., 192./256.,1);
const QVector4D Game::heavy_attack_light_factor(256./256., 204./256., 51./256.,1);
const QVector4D Game::absorbed_light_factor(1./256., 128./256., 1./256.,1);

Known_Sounds::~Known_Sounds()
{
	for (map<string,QSound*>::iterator IT=sounds.begin(); IT!=sounds.end();IT++)
	{
		delete (IT->second);
	}
}

void Game::update_game_status(E_UPDATE_GAME_STATUS_BY caller)
{
	// Note: I _hate_ switches within switches.
	if (status == E_GAME_STATUS::SURVEY)
	{
		if (caller == E_UPDATE_GAME_STATUS_BY::SURVEYOR)
		{
			this->status = E_GAME_STATUS::PRELIMINARY;
			update_statusBar_text(get_game_status_string());
		}
	}
	else if (status == E_GAME_STATUS::PRELIMINARY)
	{
		if (
			(caller == E_UPDATE_GAME_STATUS_BY::HYPERSPACE) ||
			(caller == E_UPDATE_GAME_STATUS_BY::TRANSFER) ||
			(caller == E_UPDATE_GAME_STATUS_BY::DISINTEGRATOR) ||
			(caller == E_UPDATE_GAME_STATUS_BY::MANIFESTOR)
			)
		{
			this->status = E_GAME_STATUS::RUNNING;
			update_statusBar_text(get_game_status_string());
		}
	}
	else if (status == E_GAME_STATUS::RUNNING)
	{
		// GONER_REMOVER calls only if The Sentinel was absorbed.
		if (caller == E_UPDATE_GAME_STATUS_BY::GONER_REMOVER)
		{
			this->status = E_GAME_STATUS::SENTINEL_ABSORBED;
			sentinel_disintegrating = false;
			update_statusBar_text(get_game_status_string());
			for (int x=0;x<landscape->get_width();x++)
			{
				for (int y=0;y<landscape->get_height();y++)
				{
					Figure* fig = board_fg->get(x,y);
					if (fig != 0) fig->set_spin_period(0);
				}
			}
		}
		if (caller == E_UPDATE_GAME_STATUS_BY::HYPERSPACE ||
			caller == E_UPDATE_GAME_STATUS_BY::ANTAGONIST)
		{
			if (player->get_energy_units() < 0)
			{
				this->status = E_GAME_STATUS::LOST;
				update_statusBar_text(get_game_status_string());
				known_sounds->play("defeat");
			}
		}
	}
	else if (status == E_GAME_STATUS::SENTINEL_ABSORBED)
	{
		if (caller == E_UPDATE_GAME_STATUS_BY::TRANSFER)
		{
			this->status = E_GAME_STATUS::TOWER_TAKEN;
			update_statusBar_text(get_game_status_string());
		}
	}
	else if (status == E_GAME_STATUS::TOWER_TAKEN)
	{
		if (caller == E_UPDATE_GAME_STATUS_BY::HYPERSPACE)
		{
			this->status = E_GAME_STATUS::WON;
			known_sounds->play("victory");
			if (this->game_type==E_GAME_TYPE::CAMPAIGN)
			{
				update_campaign_code(player->get_energy_units());
			} else {
				update_statusBar_text(get_game_status_string());
			}
		}
	}
}

void Game::match_current_robot_to_viewer_Data()
{
	Figure* robot = board_fg->get(player->get_site());
	if (!robot) throw "Invalid player site detected.";
	robot = robot->get_top_figure();
	if (robot->get_type() != E_FIGURE_TYPE::ROBOT) throw "Invalid player type detected?!";
	float phi = player->get_viewer_data()->get_phi();
	robot->set_phi(phi);
	float theta = player->get_viewer_data()->get_theta();
	robot->set_theta(theta);
}

bool Game::is_stack_of_stable_blocks(Figure* figure)
{
	if (!figure) return false;
	vector<Figure*> vec = figure->get_above_figure_stack();
	bool purity = true;
	for (vector<Figure*>::const_iterator CI=vec.begin();CI!=vec.end();CI++)
	{
		Figure* fig = *CI;
		if (!(fig->is_stable() && fig->get_type() == E_FIGURE_TYPE::BLOCK))
		{
			purity = false;
			break;
		}
	}
	return purity;
}

E_POSSIBLE_PLAYER_ACTION Game::get_possible_interactions(QPoint board_pos, Figure* figure)
{
	E_POSSIBLE_PLAYER_ACTION res = E_POSSIBLE_PLAYER_ACTION::NO;
	if (board_pos.x() == -1 ||
		landscape->get_board_sq()->get(board_pos)->get_type() ==
			E_SQUARE_TYPE::CONNECTION)
	{  // We are looking at nothing we could possibly interact with.
		return res;
	}
	bool interacting_with_figure = (figure != 0);
	if (figure == 0) figure = get_board_fg()->get(board_pos);
	Figure* top = figure ? figure->get_top_figure(): 0;
	switch (this->status)
	{
		case E_GAME_STATUS::PRELIMINARY: // Fall-through
		case E_GAME_STATUS::RUNNING:
			// A pure stack of stable boulders has ABSMANI.
			if (is_stack_of_stable_blocks(figure))
			{
				res = E_POSSIBLE_PLAYER_ACTION::ABSMANI;
				break;
			}
			// Something stable atop of a block may be absorbed using the block.
			else if (figure && figure->get_type() == E_FIGURE_TYPE::BLOCK && top->is_stable())
			{
				res = E_POSSIBLE_PLAYER_ACTION::ABSORPTION;
				break;
			}
			if (interacting_with_figure)
			{
				// The Sentinel may be absorbed using the tower when standing above the tower.
				if (figure->get_type()==E_FIGURE_TYPE::TOWER)
				{
					int alt = Figure::get_height(E_FIGURE_TYPE::TOWER) +
						landscape->get_altitude(board_pos.x(),board_pos.y());
					if (player->get_viewer_data()->get_site().z() > alt && top->is_stable())
					{
						res = E_POSSIBLE_PLAYER_ACTION::ABSORPTION;
					}
				}
				// Robots may be transferred to.
				else if (figure->get_type()==E_FIGURE_TYPE::ROBOT)
				{
					res = E_POSSIBLE_PLAYER_ACTION::EXCHANGE;
				}
			} else { // Not interacting with figure but with the square.
				// An empty square may be used for manifestation.
				if (!figure)
				{
					res = E_POSSIBLE_PLAYER_ACTION::MANIFESTATION;
				} else {
					// Note that block cases have already been handled.
					res = E_POSSIBLE_PLAYER_ACTION::ABSORPTION;
				}
			}
			break;
		case E_GAME_STATUS::SENTINEL_ABSORBED:
			// Only the tower may be interacted with and only for MANIFESTATION.
			if (interacting_with_figure && figure->get_type() == E_FIGURE_TYPE::TOWER)
			{
				// Don't forget: Manifestation only means a robot here!
				res = E_POSSIBLE_PLAYER_ACTION::MANIFESTATION;
			}
			if (interacting_with_figure && figure->get_type() == E_FIGURE_TYPE::ROBOT)
			{
				// Only transfer into the robot on the tower is allowed.
				Figure* tower = figure->get_below_figure();
				if (tower && tower->get_type()==E_FIGURE_TYPE::TOWER)
				{
					res = E_POSSIBLE_PLAYER_ACTION::EXCHANGE;
				}
			}
			break;
		default: break; // LOST, WON, SURVEY, TOWER_TAKEN.
	}
	return res;
}

QPoint Game::find_player_in_targets(vector<Antagonist_target>& targets)
{
	for (vector<Antagonist_target>::const_iterator CI=targets.begin();
		CI!=targets.end();CI++)
	{
		QPoint pos = CI->board_pos;

//string vis; switch(CI->visibility) { case HIDDEN: vis = "hidden"; break;	case PARTIAL: vis = "partially"; break;
//case FULL: vis = "fully"; break; }
//std::cout << "I see something "<<vis.c_str()<<" at (" << pos.x() << ", " <<  pos.y() << ")" << std::endl;
		
		if (pos == player->get_site())
		{
			return pos;
		}
	}
	return QPoint(-1,-1);
}

vector<QPoint> Game::restrict_to_free_non_CONNECTION(vector<QPoint> data)
{
	vector<QPoint> res;
	for (vector<QPoint>::const_iterator CI=data.begin();CI!=data.end();CI++)
	{
		QPoint cand = *CI;
		if (cand.x()!=-1 &&
			landscape->get_board_sq()->get(cand)->get_type()!=E_SQUARE_TYPE::CONNECTION &&
			board_fg->get(cand)==0)
		{
			res.push_back(cand);
		}
	}
	return res;
}

QPoint_Figure Game::find_stable_meanie()
{
	bool found_meanie = false;
	Figure* meanie = 0;
	QPoint site(-1,-1);
	for (int x=0;x<landscape->get_width();x++)
	{
		for (int y=0;y<landscape->get_height();y++)
		{
			Figure* cand = board_fg->get(x,y);
			if (!cand) continue;
			cand->get_top_figure();
			if (cand->get_type() == E_FIGURE_TYPE::MEANIE)
			{
				if (cand->is_stable())
				{
					meanie = cand;
					site.setX(x);
					site.setY(y);
				}
				// Note that there is only one meanie anyways, no matter
				// what its matter state is.
				found_meanie = true;
			}
			if (found_meanie) break;
		}
		if (found_meanie) break;
	}
	return QPoint_Figure(site,meanie);
}

void Game::revert_meanie_to_tree()
{
	QPoint_Figure meanie_data = find_stable_meanie();
	QPoint site = meanie_data.pos;
	Figure* meanie = meanie_data.fig;
	if (site.x()!=-1)
	{
		meanie->set_type(E_FIGURE_TYPE::TREE,landscape->get_mesh(E_FIGURE_TYPE::TREE));
		known_sounds->play("frog_reverse");
	}
	QTimer* h = meanie_timer;
	meanie_timer = 0;
	delete h;
}

void Game::meanie_timeout_slot()
{
	revert_meanie_to_tree();
	update_statusBar_text(QObject::tr("Hyperdrive coil flux restabilized."));
}

void Game::antagonist_summon_meanie(QPoint antagonist_pos, Figure* antagonist)
{
	if (meanie_timer) return; // Nothing to do. There is a meanie already.
	//> Step 1: Transmute a tree into a meanie. ----------------------
	vector<Antagonist_target> trees = this->get_antagonist_targets(antagonist_pos, true);
	if (trees.size() == 0) return; // No trees no meanies.
	uint index = qrand() % trees.size();
	Antagonist_target target = trees.at(index);
	Figure* tree = board_fg->get(target.board_pos);
	if (!tree) throw "Null pointer encountered.";
	tree = tree->get_top_figure();
	if (!tree->is_stable()) return; // Never mind. Try again the next frame!
	if (!tree->get_type() == E_FIGURE_TYPE::TREE) throw "Tree is not a tree.";
	tree->set_type(E_FIGURE_TYPE::MEANIE,landscape->get_mesh(E_FIGURE_TYPE::MEANIE));
	known_sounds->play("frog");
	update_statusBar_text(QObject::tr("Warning! Hyperdrive coil flux unstable."));
	//< --------------------------------------------------------------
	//> Step 2: Set up the meanie lifetime timer. --------------------
	{
		meanie_timer = new QTimer(this);
		meanie_timer->setSingleShot(true);
		connect(meanie_timer, SIGNAL(timeout()), this, SLOT(meanie_timeout_slot()));
		
		float spin = tree->get_spin_period();
		if (spin < 0) spin = -spin;
		int lifetime = (int)((spin/DEFAULT_MEANIE_SPEED_FACTOR + 
			 2.0 * DEFAULT_FADING_TIME) * 1000);
		meanie_timer->start(lifetime);
	}
	//< --------------------------------------------------------------
}

void Game::antagonist_tree_manifestation(QPoint antagonist_pos, Figure* antagonist)
{
	int alt = landscape->get_board_sq()->get(antagonist_pos)->get_altitude() +
	antagonist->get_altitude_above_square();
	multimap<float,QPoint> mm = scanner->get_all_board_positions_in_h_fov(
		antagonist->get_eye_position_in_world(antagonist_pos,alt),
		antagonist->get_direction(),
		antagonist->get_fov(),
		landscape->get_width(),
		landscape->get_height()
	);
	vector<QPoint> view = restrict_to_free_non_CONNECTION(
		scanner->convert_ordered_multimap_to_vector(mm));
	QPoint tree_pos = landscape->pick_initially_free_random_square(view);
	if (tree_pos.x()!= -1)
	{
		manifest_figure(tree_pos, E_FIGURE_TYPE::TREE, false);
	}
}

bool Game::antagonist_attack(QPoint pos_player, QPoint pos_antagonist,
	Figure* antagonist, vector<Antagonist_target>& targets)
{
	bool hitPlayer = false;
	E_VISIBILITY player_vis = E_VISIBILITY::HIDDEN;
//cout << "targets:" << targets.size() << endl;
	vector<Attack_duration> attacks = antagonist->mount_attacks(targets);
//cout << "attacks:" << attacks.size() << endl;
	for (vector<Attack_duration>::iterator IT=attacks.begin();IT!=attacks.end();IT++)
	{
		Attack_duration& attack = *IT;
		if (attack.board_pos == pos_player)
		{
			hitPlayer = true;
			player_vis = attack.visibility;
			if (player_vis == E_VISIBILITY::FULL && (!player->is_under_heavy_attack()))
			{
				player->set_under_heavy_attack(true);
				set_light_filtering_factor(heavy_attack_light_factor,0);
			}
			if (player_vis == E_VISIBILITY::PARTIAL && (!player->is_under_light_attack()))
			{
				player->set_under_light_attack(true);
				set_light_filtering_factor(light_attack_light_factor,0);
			}
//cout << "Under attack by "<<antagonist->get_figure_name().toStdString().c_str()<<"; " << qrand() << endl;
			if (player->take_hit(1./framerate))
			{
				if (antagonist->get_type() != E_FIGURE_TYPE::MEANIE)
				{
					if (player_vis == E_VISIBILITY::FULL)
					{
						// Damage control.
						int energy = player->update_energy_units(-1);
						known_sounds->play("tick");
						player->reset_confidence();
						update_statusBar_energy(energy);
						if (energy < 0)
						{
							set_light_filtering_factor(absorbed_light_factor,2);
							update_game_status(E_UPDATE_GAME_STATUS_BY::ANTAGONIST);
							request_paintGL();
							return true;
						}
						// Tree generation.
						antagonist_tree_manifestation(pos_antagonist,antagonist);
					} else if (player_vis == E_VISIBILITY::PARTIAL && do_meanies)
					{
						antagonist_summon_meanie(pos_antagonist,antagonist);
					}
				} else { // antagonist->get_type() == E_FIGURE_TYPE::MEANIE
					if (!hyperspace_timer)
					{
						hyperspace_request();
					}
				}
			}
		} else { // Some inanimate object is under attack.
			if (attack.duration_frames % ((int)(object_resilience * framerate)) == 0)
			{
				Figure* victim = board_fg->get(attack.board_pos);
				if (victim != 0)
				{
					bool is_base = true;
					if (victim->get_type() == E_FIGURE_TYPE::BLOCK)
					{
						if (victim->get_above_figure() != 0)
						{
							is_base = false;
							victim = victim->get_top_figure();
						}
					}
					if (victim->is_stable())
					{
						switch (victim->get_type())
						{
							case E_FIGURE_TYPE::ROBOT:
								victim->set_type(E_FIGURE_TYPE::BLOCK,landscape->get_mesh(E_FIGURE_TYPE::BLOCK));
								break;
							case E_FIGURE_TYPE::BLOCK:
								victim->set_type(E_FIGURE_TYPE::TREE,landscape->get_mesh(E_FIGURE_TYPE::TREE));
								break;
							case E_FIGURE_TYPE::TREE:
								if (!is_base)
								{
									victim->set_state(E_MATTER_STATE::DISINTEGRATING,false);
									break;
								}
							default: io->println(E_DEBUG_LEVEL::WARNING,
								"Antagonist_attack()", "Antagonist attempts to reduce "
								"object that is neither a tree, block nor robot.");
						}
						antagonist_tree_manifestation(pos_antagonist,antagonist);
					}
				}
			}
		}
	}
	return hitPlayer;
}

E_ANTAGONIST_ACTION Game::antagonist_turn_direction(QPoint pos_robot,
	QPoint pos_antagonist, float phi, bool turns_positive)
{
	float dx = (float)(pos_robot.x() - pos_antagonist.x());
	float dy = (float)(pos_robot.y() - pos_antagonist.y());
	QVector3D dir(dx,dy,0);
	QMatrix4x4 A; A.setToIdentity();
	A.rotate(-phi,QVector3D(0,0,1));
	dir = A*dir;
	dir.normalize();
	E_ANTAGONIST_ACTION res = E_ANTAGONIST_ACTION::STILL;
	if (dir.y() <= -0.05 || dir.y() >= 0.05)
	{
		if (turns_positive)
		{
			res = (dir.y() > 0) ? E_ANTAGONIST_ACTION::MOVING_FORWARD : E_ANTAGONIST_ACTION::MOVING_BACKWARD;
		}	else {
			res = (dir.y() > 0) ? E_ANTAGONIST_ACTION::MOVING_BACKWARD : E_ANTAGONIST_ACTION::MOVING_FORWARD;
		}
	}
	return res;
}

E_ANTAGONIST_ACTION Game::antagonist_action(QPoint pos_antagonist, Figure* antagonist, bool& hitPlayer)
{
	if (antagonist == 0) throw "Null pointer encountered.";
	if (pos_antagonist.x()==-1) throw "Invalid board position.";

	E_ANTAGONIST_ACTION action = E_ANTAGONIST_ACTION::STILL;
	if (antagonist->is_antagonist())
	{
		//> Determine turning direction. -----------------------------
		vector<Antagonist_target> targets = this->get_antagonist_targets(pos_antagonist);
		QPoint pos_player = find_player_in_targets(targets);
		if (pos_player.x() >= 0)
		{
//if (figure->get_type()==E_FIGURE_TYPE::SENTINEL)
//cout << "The Sentinel sees the player at (" << pos_player.x() << ", " << pos_player.y() << ")" << endl;
			action = antagonist_turn_direction(
				pos_player, pos_antagonist, antagonist->get_phi(), antagonist->get_spin_period() >= 0);
		} else {
			if (targets.size() == 0) action = E_ANTAGONIST_ACTION::MOVING_FORWARD;
		}
		//< ----------------------------------------------------------
		//> Attack! --------------------------------------------------
		hitPlayer = antagonist_attack(pos_player, pos_antagonist, antagonist, targets);
		//< ----------------------------------------------------------
	}
	return action;
}

vector<QPoint_Figure> Game::get_all_top_figures()
{
	vector<QPoint_Figure> res;
	for (int x=0;x<landscape->get_width();x++)
	{
		for (int y=0;y<landscape->get_height();y++)
		{
			Figure* figure = board_fg->get(x,y);
			if (figure)
			{
				figure = figure->get_top_figure();
				res.push_back(QPoint_Figure(QPoint(x,y),figure));
//vector<Figure*> v = figure->get_above_figure_stack();
//res.insert( res.end(), v.begin(), v.end() );
			}
		}
	}
	return res;
}

bool Game::do_progress(float dt)
{
	//> Check game state for progress-ability. -----------------------
	if (
		(get_status() != E_GAME_STATUS::RUNNING) &&
		(get_status() != E_GAME_STATUS::SENTINEL_ABSORBED) ) return false;
	//< --------------------------------------------------------------
	//> Do progress. -------------------------------------------------
	// Note that, as in real life, progress only ever happens to
	// top-of-the-stack figures.
	bool relevant_progress = false;
	vector<QPoint_Figure> pfigures = get_all_top_figures();
	bool hitPlayerOnce = false;
	for (vector<QPoint_Figure>::const_iterator CI=pfigures.begin();CI!=pfigures.end();CI++)
	{
		QPoint pos = CI->pos;
		Figure* figure = CI->fig;
		bool hitPlayer;
		E_ANTAGONIST_ACTION action = antagonist_action(pos,figure,hitPlayer);
		if (hitPlayer) hitPlayerOnce = true;
		bool new_progress = figure->progress(dt, action);
		relevant_progress = relevant_progress || new_progress;
	}
	//< --------------------------------------------------------------
	//> Regenerate player shields if appropriate. --------------------
	//> Peace attained. Regenerate. ----------------------------------
	// Unlike the last one there was no attack this turn. Stand down attack!
	if ((!hitPlayerOnce) &&
		(player->is_under_light_attack() || player->is_under_heavy_attack()))
	{
		if (player->is_under_heavy_attack())
			set_light_filtering_factor(heavy_attack_light_factor,1);
		if (player->is_under_light_attack())
			set_light_filtering_factor(light_attack_light_factor,1);
		player->set_under_light_attack(false);
		player->set_under_heavy_attack(false);
		player->reset_shields();
		request_paintGL();
	}
	//< --------------------------------------------------------------
	//< --------------------------------------------------------------
	remove_goners_from_board();
	return relevant_progress;
}

E_POSSIBLE_PLAYER_ACTION Game::get_mouse_target(float mouse_gl_x, float mouse_gl_y,
		QPoint& board_pos, Figure*& figure)
{
	//> Analyze the mouse point. -------------------------------------
	QPoint player_pos = player->get_site();
	scanner->get_mouse_target(mouse_gl_x, mouse_gl_y, this->player->get_viewer_data(),
		this->get_landscape(), this->get_board_fg(), player_pos,
		board_pos, figure);
	//< --------------------------------------------------------------
	return get_possible_interactions(board_pos,figure);
}

vector<Antagonist_target> Game::get_antagonist_targets(QPoint board_pos, bool seek_trees)
{
	Figure* antagonist = this->get_board_fg()->get(board_pos);
	antagonist = antagonist->get_top_figure();
	if (antagonist == 0) throw "Target square holds no figure.";
	float phi = antagonist->get_phi();
	int alt = get_landscape()->get_board_sq()->get(board_pos)->get_altitude();
	if (alt < 0) throw "Antagonist situated on slope square.";
	alt += antagonist->get_altitude_above_square();
	QVector3D site((float)board_pos.x(),(float)board_pos.y(),(float)alt);
	QMatrix4x4 trans_rot; trans_rot.setToIdentity();
	trans_rot.translate(site);
	trans_rot.rotate(phi,QVector3D(0,0,1));
	QVector3D eye_prototype = Figure::get_eye_position_relative_to_figure(antagonist->get_type());
	QVector3D eye = trans_rot * eye_prototype;
	QVector2D dir(cos(PI*phi/180.),sin(PI*phi/180.));
	vector<Antagonist_target> res = scanner->get_antagonist_targets(
		eye, dir, antagonist->get_fov(), get_landscape(), get_board_fg(),
		seek_trees);
	return res;
}

QString Game::get_game_status_string()
{
	QString res;
	ostringstream oss;
	switch (status)
	{
		case E_GAME_STATUS::SURVEY: res = QObject::tr("Surveying the landscape."); break;
		case E_GAME_STATUS::PRELIMINARY: res = QObject::tr("Stealth mode active."); break;
		case E_GAME_STATUS::RUNNING: res = QObject::tr("Survival situation."); break;
		case E_GAME_STATUS::SENTINEL_ABSORBED: res = QObject::tr("Survival ensured."); break;
		case E_GAME_STATUS::TOWER_TAKEN: res = QObject::tr("Dominating landscape."); break;
		case E_GAME_STATUS::WON:
			oss <<
				QObject::tr(
				"Victory! Interlandscape warp in progress. "
				"Missed landscape energy: ").toStdString().c_str() <<
				(count_landscape_energy()-8) <<
				".";
			res = oss.str().c_str();
			break;
		case E_GAME_STATUS::LOST: res = QObject::tr("Defeat."); break;
		default: throw "Unknown game state encountered.";
	}
	return res;
}

int Game::count_landscape_energy()
{
	int energy = 0;
	for (int x=0;x<landscape->get_width();x++)
	{
		for (int y=0;y<landscape->get_height();y++)
		{
			Figure* base = board_fg->get(QPoint(x,y));
			if (base)
			{
				vector<Figure*> stack = base->get_above_figure_stack();
				for (vector<Figure*>::const_iterator CI=stack.begin();CI!=stack.end();CI++)
				{
					E_FIGURE_TYPE type = (*CI)->get_type();
					energy += Figure::get_energy_value(type);
				}
			}
		}
	}
	return energy;
}

void Game::set_survey_view_data(float dphi, float dtheta)
{
	QVector3D center((float)landscape->get_width()/2., (float)landscape->get_height()/2.,0);
	float target_distance_to_center =
		sqrt((float)(landscape->get_width()*landscape->get_width()+
			landscape->get_height()*landscape->get_height()))*4./5.;
	QVector3D site(-target_distance_to_center, 0, 0);
	QMatrix4x4 A; A.setToIdentity();
	A.translate(center);
	A.rotate(player->get_viewer_data()->get_phi() + dphi, QVector3D(0,0,1));


	float abs_theta = qMin<float>(player->get_viewer_data()->get_theta() -90 + dtheta,80.);
	abs_theta = qMax<float>(10.,abs_theta);
	
	A.rotate(abs_theta, QVector3D(0,1,0));
	site = A*site;
	player->get_viewer_data()->set_viewer_data(
		site, center-site, player->get_viewer_data()->get_opening());
}

void Game::handle_scan_key(E_POSSIBLE_PLAYER_ACTION action, QPoint board_pos, Figure* figure)
{
	ostringstream oss;
	if (board_pos.x()==-1)
	{
		QString empty = tr("No interactable matter detected.");
		oss << empty.toStdString().c_str();
	} else {
		if (figure == 0)
		{
			QString hue;
			if (landscape->get_board_sq()->get(board_pos)->get_type()==E_SQUARE_TYPE::CONNECTION)
			{
				hue = "Sloped";
			} else {
				hue = ((board_pos.x()+board_pos.y())%2==0) ?
					tr("Dark") : tr("Light");
			}
			oss << hue.toStdString() <<
				tr(" square detected at coordinates (").toStdString() <<
				board_pos.x() << "," << board_pos.y() << ").";
		} else {
			QString name = figure->get_figure_name();
			QString task;
			switch (action)
			{
				case E_POSSIBLE_PLAYER_ACTION::ABSMANI: task = tr("Interactible"); break;
				case E_POSSIBLE_PLAYER_ACTION::ABSORPTION: task = tr("Absorbable"); break;
				case E_POSSIBLE_PLAYER_ACTION::MANIFESTATION: task = tr("Manifestible"); break;
				case E_POSSIBLE_PLAYER_ACTION::EXCHANGE: task = tr("Transferable"); break;
				default: task = tr("Isolated");
			}
			QString h = tr(" detected at coordinates (");
			oss << task.toStdString().c_str() << " " <<
				name.toStdString().c_str() << h.toStdString().c_str() <<
				board_pos.x() << "," << board_pos.y() << ").";
		}
	}
	update_statusBar_text(tr(oss.str().c_str()));
}

void Game::do_u_turn()
{
	if (status != E_GAME_STATUS::SURVEY &&
		status != E_GAME_STATUS::LOST &&
		status != E_GAME_STATUS::WON)
	player->get_viewer_data()->turn_around_z_axis();
}

void Game::end_survey()
{
	//> Getting pointers. --------------------------------------------
	QPoint board_site = player->get_site();
	Square* sq = landscape->get_board_sq()->get(board_site);
	Figure* fg = board_fg->get(board_site);
	if (sq==0||fg==0) throw "Something is queer. No 0 pointer expected at this point.";
	// Note that there can be no stack at this time.
	//< --------------------------------------------------------------
	//> Setting the view parameters. ---------------------------------
	QVector3D eye = Figure::get_eye_position_relative_to_figure(E_FIGURE_TYPE::ROBOT);
	QMatrix4x4 A; A.setToIdentity();
	A.rotate(fg->get_phi(), QVector3D(0,0,1));
	eye = A * eye;
	QVector3D site(
		((float)(board_site.x())) + eye.x(),
		((float)(board_site.y())) + eye.y(),
		((float)(sq->get_altitude()))+eye.z()
	);
	player->get_viewer_data()->set_site(site);
	player->get_viewer_data()->set_direction(fg->get_phi(),90,0);
	//< --------------------------------------------------------------
	update_game_status(E_UPDATE_GAME_STATUS_BY::SURVEYOR);
}

bool Game::manifest_figure(QPoint pos, E_FIGURE_TYPE type, bool by_robot)
{
	//> Make sure SENTINEL_ABSORBED state is handled properly. .......
	if (by_robot && status == E_GAME_STATUS::SENTINEL_ABSORBED &&
		type != E_FIGURE_TYPE::ROBOT) return false;
	Figure* base_figure = board_fg->get(pos);
	if (base_figure && base_figure->get_type() == E_FIGURE_TYPE::TOWER &&
		base_figure->get_above_figure() != 0)
	{
		return false;
	}
	//< --------------------------------------------------------------
	if (base_figure) base_figure = base_figure->get_top_figure();
	if (by_robot)
	{
		int cost = Figure::get_energy_value(type);
		int energy = player->get_energy_units();
		if (cost > energy) return false;
		player->update_energy_units(-cost);
		update_statusBar_energy(energy-cost);
	}
	float phi = landscape->get_random_angle();
	float theta = 90.;
	//> A new robot shall look at the player. ------------------------
	if (type==E_FIGURE_TYPE::ROBOT && by_robot)
	{
		// Getting phi for the new robot.
		QVector3D dir(
			player->get_site().x()-pos.x(),
			player->get_site().y()-pos.y(),
			0.
		);
		dir.normalize();
		phi = 180.*acos(dir.x())/PI;
		if (dir.y() < 0) phi = -phi+360.;
		// Getting theta for the new robot.
		float dx = player->get_site().x()-pos.x();
		float dy = player->get_site().y()-pos.y();
		float dist_h = sqrt(dx*dx+dy*dy);
		float dz =
			(base_figure ?
			 (base_figure->get_altitude_above_square() +
			  Figure::get_height(base_figure->get_type())) : 0) +
			landscape->get_altitude(pos.x(),pos.y()) -
			landscape->get_altitude(player->get_site().x(),player->get_site().y());
		theta = 90.+180.*atan(dz/dist_h)/PI;
	}
	//< --------------------------------------------------------------
	Figure* new_figure = new Figure(
		type,
		E_MATTER_STATE::MANIFESTING,
		landscape->get_mesh(type),
		phi,
		theta,
		// Trees and co are STILL anyways. Note however, that trees may turn
		// into meanies and meanies will base their rotation speeds on the
		// "rotation speed" of the tree they are made of. So it _is_ sensible
		// that a tree has the rotation speed of an antagonist.
		landscape->get_antagonist_spin_period(),
		landscape->get_antagonist_fov(),
		landscape->get_antagonist_fading_time()
	);
	if (base_figure)
	{
		base_figure->set_figure_above(new_figure);
	} else {
		board_fg->set(pos,new_figure);
	}
	update_game_status(E_UPDATE_GAME_STATUS_BY::MANIFESTOR);
	known_sounds->play("delayed_plop");
	return true;
}

void Game::disintegrate_figure(QPoint pos, bool by_robot)
{
	Figure* fig = board_fg->get(pos);
	// Don't absorb what you are standing on.
	if (pos == player->get_site()) return;
	if (fig)
	{
		fig = fig->get_top_figure();
		if (fig->get_state() != E_MATTER_STATE::STABLE) return;
		fig->set_state(E_MATTER_STATE::DISINTEGRATING, by_robot);
		if (fig->get_type()==E_FIGURE_TYPE::SENTINEL)
		{
			known_sounds->play("absorption_sentinel");
			sentinel_disintegrating = true;
		} else {
			known_sounds->play("absorption");
		}
	}
	update_game_status(E_UPDATE_GAME_STATUS_BY::DISINTEGRATOR);
}

bool Game::hyperspace_request()
{
	if ((status != E_GAME_STATUS::PRELIMINARY && status != E_GAME_STATUS::RUNNING &&
		status != E_GAME_STATUS::TOWER_TAKEN) || sentinel_disintegrating)
	{
		update_statusBar_text(tr("Hyperdrive not available at this time."));
		return false;
	}
	if (hyperspace_timer != 0)
	{
		update_statusBar_text(tr("Hyperdrive already in charging process."));
		return false;
	}
	update_statusBar_text(tr("Hyperdrive activated. Charging coils now."));
	hyperspace_timer = new QTimer(this);
	hyperspace_timer->setSingleShot(true);
    connect(hyperspace_timer, SIGNAL(timeout()), this, SLOT(hyperspace_jump()));
    hyperspace_timer->start(DEFAULT_HYPERDRIVE_CHARGING_TIME);
	set_light_filtering_factor(hyperspace_light_factor,0);
	return true;
}

void Game::hyperspace_jump()
{
	if (!hyperspace_timer) throw "Unauthorized hyperdrive request!! Use hyperspace_timer.";
	delete hyperspace_timer;
	hyperspace_timer = 0;

	player->update_energy_units(-3);
	update_statusBar_energy(player->get_energy_units());
	// This update_game_status() command needs to be here exactly.
	update_game_status(E_UPDATE_GAME_STATUS_BY::HYPERSPACE);
	if (status == E_GAME_STATUS::LOST || status == E_GAME_STATUS::WON) { return; }
	
	known_sounds->play("plop");
	float new_phi = landscape->get_random_angle();
	QPoint old_site = player->get_site();
	int old_alt = landscape->get_board_sq()->get(old_site)->get_altitude();
	vector<QPoint> sqbyht = restrict_to_free_non_CONNECTION(
		landscape->get_coordinates_by_altitude(0,old_alt));
	QPoint new_site = landscape->pick_initially_free_random_square(sqbyht);
	
	if (new_site.x()==-1)
	{
		update_statusBar_text(tr("Hyperjump aborted. Wormhole unstable."));
		return;
	}
	
	match_current_robot_to_viewer_Data();
	
	Figure* new_robot = new Figure(
		E_FIGURE_TYPE::ROBOT,
		E_MATTER_STATE::STABLE, // Jep. Instant stability after hyperjump!
		landscape->get_mesh(E_FIGURE_TYPE::ROBOT),
		new_phi,
		90,
		player->get_self_spin(), // Not that it is important...
		landscape->get_antagonist_fov(),
		landscape->get_antagonist_fading_time()
	);
	if (board_fg->get(new_site)) throw "Hyperspace target square not empty. This is a bug.";
	board_fg->set(new_site,new_robot);
	set_light_filtering_factor(hyperspace_light_factor,1);
	transfer(new_site);
	revert_meanie_to_tree();
}

void Game::transfer(QPoint destination)
{
	if (sentinel_disintegrating)
	{
		update_statusBar_text(tr("Mind transfer failed."));
		return;
	}
	match_current_robot_to_viewer_Data();
	Figure* new_robot = board_fg->get(destination);
	if (!new_robot) throw "No new robot found.";
	new_robot = new_robot->get_top_figure();
	if (new_robot->get_type()!=E_FIGURE_TYPE::ROBOT) throw "New robot is no robot.";
	player->set_site(destination);
	float phi = new_robot->get_phi();
	QVector3D eye(
		(float)destination.x(),
		(float)destination.y(),
		(float)landscape->get_altitude(destination.x(), destination.y())
	);
	eye = eye + Figure::get_eye_position_relative_to_figure(E_FIGURE_TYPE::ROBOT);
	eye.setZ(eye.z() + (float)new_robot->get_altitude_above_square());
	Viewer_Data* vd = player->get_viewer_data();
	vd->set_site(eye);
	vd->set_direction(phi,new_robot->get_theta());
	update_statusBar_text(tr("Transfer completed."));
	update_game_status(E_UPDATE_GAME_STATUS_BY::TRANSFER);
}

void Game::remove_goners_from_board()
{
	bool absorbed_the_sentinel = false;
	int energy_for_robot = 0;
	for (int x=0;x<this->landscape->get_width();x++)
	{
		for (int y=0;y<this->landscape->get_height();y++)
		{
			Figure* fig = board_fg->get(x,y);
			if (fig)
			{
				Figure* top = fig->get_top_figure();
				if (top->is_gone() && top->get_type() == E_FIGURE_TYPE::SENTINEL)
				{
					absorbed_the_sentinel = true;
				}
				// Case 1: Check for gone top-of-the-stack-figure.
				energy_for_robot += fig->check_for_and_delete_top_figure(true);
				// Case 2: Check if this base figure is gone.
				if (fig->is_gone())
				{
					energy_for_robot += fig->get_energy_for_robot();
					board_fg->set(x,y,0);
					delete fig;
				}
			}
		}
	}
	player->update_energy_units(energy_for_robot);
	update_statusBar_energy(player->get_energy_units());
	if (absorbed_the_sentinel) update_game_status(E_UPDATE_GAME_STATUS_BY::GONER_REMOVER);
}

void Game::pause_timers()
{
	if (hyperspace_timer && hyperspace_timer->isActive())
	{
		hyperspace_timer_remaining = hyperspace_timer->remainingTime();
		hyperspace_timer->stop();
		hyperspace_timer_paused = true;
	}
	if (meanie_timer && meanie_timer->isActive())
	{
		meanie_timer_remaining = meanie_timer->remainingTime();
		meanie_timer->stop();
		meanie_timer_paused = true;
	}
}

void Game::unpause_timers()
{
	if (hyperspace_timer && hyperspace_timer_paused)
	{
		hyperspace_timer->start(hyperspace_timer_remaining);
	}
	if (meanie_timer && meanie_timer_paused)
	{
		meanie_timer->start(meanie_timer_remaining);
	}
	reset_timer_helpers();
}

void Game::reset_timer_helpers()
{
	meanie_timer_paused = false;
	meanie_timer_remaining = 0;
	hyperspace_timer_paused = false;
	hyperspace_timer_remaining = 0;
}

Game::Game(E_GAME_TYPE type, uint seed, Setup_game_data* setup, QOpenGLWidget* parent,
	Io_Qt* io, Known_Sounds* known_sounds, float framerate,
	Mesh_Data* mesh_connection, Mesh_Data* mesh_odd, Mesh_Data* mesh_even,
	Mesh_Data* mesh_sentinel, Mesh_Data* mesh_sentinel_tower, Mesh_Data* mesh_sentry,
	Mesh_Data* mesh_tree, Mesh_Data* mesh_robot,
	Mesh_Data* mesh_block, Mesh_Data* mesh_meanie)
{
	this->framerate = framerate;
	this->object_resilience = (float)(setup->spinBox_object_resilience);
	this->meanie_timer = 0;
	this->io = io;
	this->known_sounds = known_sounds;
	this->scanner = new Scanner(io);
	this->game_type = type;
	this->status = E_GAME_STATUS::SURVEY;
	this->hyperspace_timer = 0;
	this->do_meanies = setup->checkBox_meanies;
	this->sentinel_disintegrating = false;
	reset_timer_helpers();
	//> Setup Landscape object. --------------------------------------
	this->landscape = new Landscape(
		seed,
		setup->spinBox_cols,
		setup->spinBox_rows,
		setup->combobox_gravity,
		setup->combobox_age,
		setup->spinBox_spin_period,
		setup->spinBox_antagonist_fov,
		DEFAULT_FADING_TIME,
		setup->spinBox_energy,
		setup->spinBox_sentries,
		setup->combobox_rotation_type,
		io,
		mesh_connection,
		mesh_odd,
		mesh_even,
		mesh_sentinel,
		mesh_sentinel_tower,
		mesh_sentry,
		mesh_tree,
		mesh_robot,
		mesh_block,
		mesh_meanie
   	);
	this->board_fg = landscape->get_new_initialized_board_fg();
	//< --------------------------------------------------------------
	//> Setup Player object. -----------------------------------------
	this->player = new Player_Data(
		parent,
		setup,
		io,
		landscape->get_player_starting_position(),
		DEFAULT_OPENING_MIN,
		DEFAULT_OPENING_ANGLE,
		DEFAULT_OPENING_MAX
	);
	//< --------------------------------------------------------------
	//> Setup viewer data for the SURVEY mode. -----------------------
	player->get_viewer_data()->set_direction(0,115);
	set_survey_view_data(0,0);
	//< --------------------------------------------------------------
}

Game::~Game()
{
	delete scanner;
	delete player;
	delete landscape;
	delete hyperspace_timer;
}
}
