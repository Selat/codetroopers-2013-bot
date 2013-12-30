#include "MyStrategy.h"
#include "Point.h"
#include "TrooperOptions.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <queue>
#include <cstdio>
#include <iostream>
#include <climits>

using namespace std;

TrooperOptions MyStrategy::commander;
TrooperOptions MyStrategy::soldier;
TrooperOptions MyStrategy::medic;
TrooperOptions MyStrategy::sniper;
TrooperOptions MyStrategy::scout;

bool MyStrategy::need_scouting = false;
bool MyStrategy::done_scouting = false;
bool MyStrategy::healing_process = false;
model::TrooperType MyStrategy::healing_target;

int MyStrategy::width = -1;
int MyStrategy::height = -1;

MyStrategy::MyStrategy()
{
    srand(time(NULL));
    unchanged_steps_count = 0;
    laying_steps = 0;
    freopen("test.log", "w", stdout);
}

void MyStrategy::move(const model::Trooper& self, const model::World& global_world, const model::Game& game, model::Move& move)
{
    // cout << endl << "## NEW TURN ##" << endl;
    cout << need_scouting << endl;
    world = &global_world;
    players = world -> getPlayers();
    if(width == -1)
    {
        width = world -> getWidth();
        height = world -> getHeight();
    }

    vector <model::Trooper> enemies;
    vector <model::Trooper> teammates;
    vector <model::Trooper> troopers = world -> getTroopers();
    const vector <vector <model::CellType> > field = world -> getCells();
    vector <vector <model::CellType> > units_field(width);
    for(int i = 0; i < width; ++i)
    {
        units_field[i].resize(height);
        for(int j = 0; j < height; ++j)
        {
            units_field[i][j] = field[i][j];
        }
    }
    vector <model::Bonus> bonuses(world -> getBonuses());
    for(unsigned int i = 0; i < bonuses.size();)
    {
        if(!((bonuses[i].getType() == model::MEDIKIT) ||(bonuses[i].getType() == model::GRENADE)))
        {
            bonuses.erase(bonuses.begin() + i);
        } else {
            ++i;
        }
    }

    for(unsigned int i = 0; i < troopers.size(); ++i)
    {
        if(troopers[i].isTeammate())
        {
            teammates.push_back(troopers[i]);
        } else
        {
            enemies.push_back(troopers[i]);
        }
    }
    setupStartPoints(teammates);
    updateAliveTroopers(teammates);

    if(!commander.isAlive())
    {
        need_scouting = false;
        done_scouting = false;
    }

    for(unsigned int i = 0; i < enemies.size(); ++i)
    {
        Point tmp_point;
        for(unsigned int j = 0; j < teammates.size(); ++j)
        {
            if(world -> isVisible(teammates[j].getShootingRange(), teammates[j].getX(), teammates[j].getY(),
                               teammates[j].getStance(), enemies[i].getX(), enemies[i].getY(),
                               enemies[i].getStance()))
            {
                tmp_point.set(teammates[j].getX(), teammates[j].getY());
                getTrooper(teammates[j]).setTargetPoint(tmp_point);
            }
        }
    }
    for(unsigned int i = 0; i < teammates.size(); ++i)
    {
        if((teammates[i].getHitpoints() > 0)
           && ((getTrooper(teammates[i]).getTargetPoint() == getTrooperPos(teammates[i]))
               || ((getTrooper(teammates[i]).getLastPoint() == getTrooperPos(teammates[i]))
		   && (teammates[i].getActionPoints() < teammates[i].getMaximalHitpoints()))))
        {
            units_field[teammates[i].getX()][teammates[i].getY()] = model::LOW_COVER;
        }
    }

    if(medic.isAlive())
    {
        for(unsigned int i = 0; i < teammates.size(); ++i)
        {
            if((teammates[i].getHitpoints() > 0)
               && (teammates[i].getHitpoints() < teammates[i].getMaximalHitpoints()))
            {
                healing_process = true;
                healing_target = teammates[i].getType();
                break;
            }
        }
    }
    if(self.isHoldingGrenade() && (self.getActionPoints() >= game.getGrenadeThrowCost()))
    {
        Point from = getTrooperPos(self);
        Point to;
        for(unsigned int i = 0; i < enemies.size(); ++i)
        {
            to = getTrooperPos(enemies[i]);
            if(from.getSquareDistance(to) <= 25)
            {
                move.setAction(model::THROW_GRENADE);
                move.setX(enemies[i].getX());
                move.setY(enemies[i].getY());
                return;
            }
        }
	if(self.getActionPoints() >= game.getGrenadeThrowCost() + getMoveCost(self.getStance(), game))
	{
	    if((self.getX() > 0) && (field[self.getX() - 1][self.getY()] == model::FREE) && 
	       isFreeCell(teammates, self.getX() - 1, self.getY()))
	    {
		if(tryThrowGrenade(move, enemies, self.getX() - 1, self.getY()))
		{
		    move.setAction(model::MOVE);
		    move.setX(self.getX() - 1);
		    move.setY(self.getY());
		    return;
		}
	    }
	    if((self.getX() < width - 1) && (field[self.getX() + 1][self.getY()] == model::FREE) && 
	       isFreeCell(teammates, self.getX() + 1, self.getY()))
	    {
		if(tryThrowGrenade(move, enemies, self.getX() + 1, self.getY()))
		{
		    move.setAction(model::MOVE);
		    move.setX(self.getX() + 1);
		    move.setY(self.getY());
		    return;
		}
	    }
	    if((self.getY() > 0) && (field[self.getX()][self.getY() - 1] == model::FREE) && 
	       isFreeCell(teammates, self.getX(), self.getY() - 1))
	    {
		if(tryThrowGrenade(move, enemies, self.getX(), self.getY() - 1))
		{
		    move.setAction(model::MOVE);
		    move.setX(self.getX());
		    move.setY(self.getY() - 1);
		    return;
		}
	    }
	    if((self.getY() < height - 1) && (field[self.getX()][self.getY() + 1] == model::FREE) && 
	       isFreeCell(teammates, self.getX(), self.getY() + 1))
	    {
		if(tryThrowGrenade(move, enemies, self.getX(), self.getY() + 1))
		{
		    move.setAction(model::MOVE);
		    move.setX(self.getX());
		    move.setY(self.getY() + 1);
		    return;
		}
	    }
	}
    }
    cout << "Throwing ok" << endl;
    if(tryHeal(game, move, teammates, enemies, self))
    {
        return;
    }
    cout << "Healing ok" << endl;

    Point target_point;
    updateDistance(units_field, used, self.getX(), self.getY());
    /*
      Updating positions in case of blocking
    */
    if((getTrooperPos(self) == getTrooper(self).getLastPoint()) && (enemies.size() == 0))
    {
	++unchanged_steps_count;
    } else {
	unchanged_steps_count = 0;
    }
    if(unchanged_steps_count > 5)
    {
	cout << "Force updating..." << endl;
	unchanged_steps_count = 0;
	target_point = updateTargetPoints(units_field, bonuses, teammates, enemies, self, true);
    } else {
	target_point = updateTargetPoints(units_field, bonuses, teammates, enemies, self, false);
    }

    cout << "Targeting ok" << endl;
    if((((enemies.size() > 0) && (teammates.size() > 1)) || healing_process) && (self.getType() == model::FIELD_MEDIC))
    {
        target_point = updateMedicTargetPoint(units_field, teammates, enemies);
        cout << "Medic: " << target_point.getX() << " " << target_point.getY() << endl;
    }
    if(need_scouting && (self.getType() == model::COMMANDER) && (self.getActionPoints() >= 10 ))
    {
        move.setAction(model::REQUEST_ENEMY_DISPOSITION);
        need_scouting = false;
        done_scouting = true;
        return;
    }
    if(target_point.getX() == -1)
    {
        move.setAction(model::END_TURN);
        return;
    }
    if((getTrooperPos(self) == getTrooperTargetPoint(self)) && (enemies.size() == 0))
    {
        Point last_point = getTrooper(self).getTargetPoint();
        getTrooper(self).setLastPoint(last_point);
        move.setAction(model::END_TURN);
        return;
    }
    if(self.isHoldingFieldRation() && (self.getActionPoints() >= game.getFieldRationEatCost()))
    {
        move.setAction(model::EAT_FIELD_RATION);
        return;
    }
    cout << "<" << target_point.getX() << " " << target_point.getY() << ">" << endl;
    cout << "<" << self.getX() << " " << self.getY() << ">" << endl;
    cout << "<" << getTrooper(self).getLastPoint().getX() << " " << getTrooper(self).getLastPoint().getY() << ">" << endl;
    cout << "Stance: " << self.getStance() << endl;
    bool can_shoot = false;
    for(unsigned int i = 0; i < enemies.size(); ++i)
    {
        if(world -> isVisible(self.getShootingRange(),
                           self.getX(), self.getY(),
                           self.getStance(),
                           enemies[i].getX(), enemies[i].getY(),
                           enemies[i].getStance()))
        {
            can_shoot = true;
        }
    }
    if(self.getStance() != model::STANDING)
    {
	++laying_steps;
    }
    if(can_shoot)
    {
	if(tryLowStance(game, move, teammates, enemies, self))
	{
	    if(self.getActionPoints() == game.getStanceChangeCost())
	    {
		Point pos(self.getX(), self.getY());
		getTrooper(self).setLastPoint(pos);
	    }
	    return;
	}
    } else if((target_point.getSquareDistance(getTrooperPos(self)) > 8) || (enemies.size() == 0))
    {
	if((laying_steps >= 2) && tryRaiseStance(game, move, teammates, enemies, self))
	{
	    laying_steps = 0;
	    Point pos(self.getX(), self.getY());
	    getTrooper(self).setLastPoint(pos);
	    return;
	}
    }
    cout << "Healing: " << healing_process << endl;
    cout << "Can shoot: " << can_shoot << endl << endl;
    if((enemies.size() > 0) && can_shoot && (self.getActionPoints() >= self.getShootCost())
       && !((self.getType() == model::FIELD_MEDIC) && healing_process))
    {
        // unsigned int enemy_id = chooseEnemy(enemies, teammates, world);
        for(unsigned int i = 0; i < enemies.size(); ++i)
        {
            if(world -> isVisible(self.getShootingRange(), self.getX(),
                               self.getY(), self.getStance(),
                               enemies[i].getX(), enemies[i].getY(), enemies[i].getStance()))
            {
                move.setAction(model::SHOOT);
                move.setX(enemies[i].getX());
                move.setY(enemies[i].getY());
                return;
            }
        }
        move.setAction(model::END_TURN);
    } else if(!((self.getActionPoints() < self.getShootCost()) && can_shoot))
    {
        model::Direction direction =
            getNextStep(self.getX(), self.getY(), target_point.getX(), target_point.getY());
        if((direction != model::CURRENT_POINT) && (self.getActionPoints() >= getMoveCost(self.getStance(), game)))
        {
            Point last_point(self.getX(), self.getY());
            move.setAction(model::MOVE);
            move.setDirection(direction);
            if(self.getActionPoints() == getMoveCost(self.getStance(), game))
            {
                switch(direction)
                {
                case model::NORTH:
                    if((self.getY() > 0) && (field[self.getX()][self.getY() - 1] == model::FREE)
                       && isFreeCell(teammates, self.getX(), self.getY() - 1))
                    {
                        last_point.setY(self.getY() - 1);
                    }
                    break;
                case model::SOUTH:
                    if((self.getY() < height - 1) && (field[self.getX()][self.getY() + 1] == model::FREE)
                       && isFreeCell(teammates, self.getX(), self.getY() + 1))
                    {
                        last_point.setY(self.getY() + 1);
                    }
                    break;
                case model::WEST:
                    if((self.getX() > 0) && (field[self.getX() - 1][self.getY()] == model::FREE)
                       && isFreeCell(teammates, self.getX() - 1, self.getY()))
                    {
                        last_point.setX(self.getX() - 1);
                    }
                    break;
                case model::EAST:
                    if((self.getX() < width - 1) && (field[self.getX() + 1][self.getY()] == model::FREE)
                       && isFreeCell(teammates, self.getX() + 1, self.getY()))
                    {
                        last_point.setX(self.getX() + 1);
                    }
                    break;
                }
                getTrooper(self).setLastPoint(last_point);
            }
        } else {
            Point last_point(self.getX(), self.getY());
            getTrooper(self).setLastPoint(last_point);
            move.setAction(model::END_TURN);
        }
    } else if((self.getActionPoints() >= getMoveCost(self.getStance(), game)) && (enemies.size() > 0))
    {
	if(!tryEscape(game, move, field, teammates, enemies, self,
		      self.getActionPoints() >= getMoveCost(self.getStance(), game)))
	{
	    move.setAction(model::END_TURN);
	}	
    } else {
	Point last_point = getTrooper(self).getTargetPoint();
        getTrooper(self).setLastPoint(last_point);
	move.setAction(model::END_TURN);
    }
}

unsigned int MyStrategy::chooseEnemy(const vector <model::Trooper>& enemies,
                                     const vector <model::Trooper>& teammates)
{
    unsigned int result = enemies.size();
    int max_accesible = 0;
    for(unsigned int i = 0; i < enemies.size(); ++i)
    {
        int accesible_num = 0;
        for(unsigned int j = 0; j < teammates.size(); ++j)
        {
            if(world -> isVisible(teammates[j].getShootingRange(), teammates[j].getX(),
                               teammates[j].getY(), teammates[j].getStance(),
                               enemies[i].getX(), enemies[i].getY(), enemies[i].getStance()))
            {
                ++accesible_num;
            }
        }
        if(accesible_num > max_accesible)
        {
            max_accesible = accesible_num;
            result = i;
        }
    }
    return result;
}

void MyStrategy::updateAliveTroopers(const vector <model::Trooper>& teammates)
{
    bool found = false;
    for(unsigned int i = 0; i < teammates.size(); ++i)
    {
        if(teammates[i].getType() == model::COMMANDER)
        {
            found = true;
        }
    }
    commander.setAlive(found && commander.isAlive());
    found = false;
    for(unsigned int i = 0; i < teammates.size(); ++i)
    {
        if(teammates[i].getType() == model::FIELD_MEDIC)
        {
            found = true;
        }
    }
    medic.setAlive(found && medic.isAlive());
    found = false;
    for(unsigned int i = 0; i < teammates.size(); ++i)
    {
        if(teammates[i].getType() == model::SOLDIER)
        {
            found = true;
        }
    }
    soldier.setAlive(found && soldier.isAlive());
}

bool MyStrategy::tryHeal(const model::Game& game,
                         model::Move& move,
                         const std::vector <model::Trooper>& teammates,
                         const std::vector <model::Trooper>& enemies,
                         const model::Trooper& self)
{
    if(self.isHoldingMedikit() && (self.getActionPoints() >= game.getMedikitUseCost())
       && ((enemies.size() > 0) || !medic.isAlive()))
    {
        for(unsigned int i = 0; i < teammates.size(); ++i)
        {
            if((teammates[i].getX() == self.getX() - 1) && (teammates[i].getY() == self.getY())
               && (teammates[i].getHitpoints() + game.getMedikitBonusHitpoints() - 10
                   < teammates[i].getMaximalHitpoints()))
            {
                move.setAction(model::USE_MEDIKIT);
                move.setX(self.getX() - 1);
                move.setY(self.getY());
                return true;
            }
        }
        for(unsigned int i = 0; i < teammates.size(); ++i)
        {
            if((teammates[i].getX() == self.getX() + 1) && (teammates[i].getY() == self.getY())
               && (teammates[i].getHitpoints() + game.getMedikitBonusHitpoints() - 10
                   < teammates[i].getMaximalHitpoints()))
            {
                move.setAction(model::USE_MEDIKIT);
                move.setX(self.getX() + 1);
                move.setY(self.getY());
                return true;
            }
        }
        for(unsigned int i = 0; i < teammates.size(); ++i)
        {
            if((teammates[i].getX() == self.getX()) && (teammates[i].getY() == self.getY() - 1)
               && (teammates[i].getHitpoints() + game.getMedikitBonusHitpoints() - 10
                   < teammates[i].getMaximalHitpoints()))
            {
                move.setAction(model::USE_MEDIKIT);
                move.setX(self.getX());
                move.setY(self.getY() - 1);
                return true;
            }
        }
        for(unsigned int i = 0; i < teammates.size(); ++i)
        {
            if((teammates[i].getX() == self.getX()) && (teammates[i].getY() == self.getY() + 1)
               && (teammates[i].getHitpoints() + game.getMedikitBonusHitpoints() - 10
                   < teammates[i].getMaximalHitpoints()))
            {
                move.setAction(model::USE_MEDIKIT);
                move.setX(self.getX());
                move.setY(self.getY() + 1);
                return true;
            }
        }
        if(self.getHitpoints() + game.getMedikitHealSelfBonusHitpoints() - 10 < self.getMaximalHitpoints())
        {
            move.setAction(model::USE_MEDIKIT);
            move.setDirection(model::CURRENT_POINT);
            return true;
        }
    }
    cout << "Medikit ok" << endl;
    if((self.getType() == model::FIELD_MEDIC)
       && (self.getHitpoints() + game.getMedikitHealSelfBonusHitpoints() - 10 < self.getMaximalHitpoints())
       && self.isHoldingMedikit()
       && (self.getActionPoints() >= game.getMedikitUseCost()))
    {
        move.setAction(model::USE_MEDIKIT);
        move.setDirection(model::CURRENT_POINT);
        return true;
    }
    if(healing_process && (self.getType() == model::FIELD_MEDIC)
       && (self.getActionPoints() >= game.getFieldMedicHealCost()))
    {
        healing_process = false;
        for(unsigned int i = 0; i < teammates.size(); ++i)
        {
            if((teammates[i].getX() == self.getX() - 1) && (teammates[i].getY() == self.getY())
               && (teammates[i].getHitpoints() < teammates[i].getMaximalHitpoints()))
            {
                move.setAction(model::HEAL);
                move.setX(self.getX() - 1);
                move.setY(self.getY());
                return true;
            }
        }
        for(unsigned int i = 0; i < teammates.size(); ++i)
        {
            if((teammates[i].getX() == self.getX() + 1) && (teammates[i].getY() == self.getY())
               && (teammates[i].getHitpoints() < teammates[i].getMaximalHitpoints()))
            {
                move.setAction(model::HEAL);
                move.setX(self.getX() + 1);
                move.setY(self.getY());
                return true;
            }
        }
        for(unsigned int i = 0; i < teammates.size(); ++i)
        {
            if((teammates[i].getX() == self.getX()) && (teammates[i].getY() == self.getY() - 1)
               && (teammates[i].getHitpoints() < teammates[i].getMaximalHitpoints()))
            {
                move.setAction(model::HEAL);
                move.setX(self.getX());
                move.setY(self.getY() - 1);
                return true;
            }
        }
        for(unsigned int i = 0; i < teammates.size(); ++i)
        {
            if((teammates[i].getX() == self.getX()) && (teammates[i].getY() == self.getY() + 1)
               && (teammates[i].getHitpoints() < teammates[i].getMaximalHitpoints()))
            {
                move.setAction(model::HEAL);
                move.setX(self.getX());
                move.setY(self.getY() + 1);
                return true;
            }
        }
        if(self.getHitpoints() < self.getMaximalHitpoints())
        {
            move.setAction(model::HEAL);
            move.setDirection(model::CURRENT_POINT);
            return true;
        }
    }
    return false;
}

bool MyStrategy::tryEscape(const model::Game& game,
			   model::Move& move,
			   const std::vector <std::vector <model::CellType> >& field,
			   const std::vector <model::Trooper>& teammates,
			   const std::vector <model::Trooper>& enemies,
			   const model::Trooper& self,
			   bool setup_last_point)
{
    int min_enemies_num = getDangerEnemiesCount(enemies, self.getX(), self.getY(), self.getStance());
    Point target_point(self.getX(), self.getY());
    int current_enemies_num;
    if((self.getX() > 0) && (field[self.getX() - 1][self.getY()] == model::FREE) && 
       isFreeCell(teammates, self.getX() - 1, self.getY()) &&
       ((current_enemies_num = getDangerEnemiesCount(enemies, self.getX() - 1, self.getY(), self.getStance()))
	   < min_enemies_num))
    {
	min_enemies_num = current_enemies_num;
	target_point.set(self.getX() - 1, self.getY());
    }
    if((self.getX() < width - 1) && (field[self.getX() + 1][self.getY()] == model::FREE) && 
       isFreeCell(teammates, self.getX() + 1, self.getY()) &&
       ((current_enemies_num = getDangerEnemiesCount(enemies, self.getX() + 1, self.getY(), self.getStance()))
	   < min_enemies_num))
    {
	min_enemies_num = current_enemies_num;
	target_point.set(self.getX() + 1, self.getY());
    }
    if((self.getY() > 0) && (field[self.getX()][self.getY() - 1] == model::FREE) && 
       isFreeCell(teammates, self.getX(), self.getY() - 1) &&
       ((current_enemies_num = getDangerEnemiesCount(enemies, self.getX(), self.getY() - 1, self.getStance()))
	   < min_enemies_num))
    {
	min_enemies_num = current_enemies_num;
	target_point.set(self.getX(), self.getY() - 1);
    }
    if((self.getY() < height - 1) && (field[self.getX()][self.getY() + 1] == model::FREE) && 
       isFreeCell(teammates, self.getX(), self.getY() + 1) &&
       ((current_enemies_num = getDangerEnemiesCount(enemies, self.getX(), self.getY() + 1, self.getStance()))
	   < min_enemies_num))
    {
	min_enemies_num = current_enemies_num;
	target_point.set(self.getX(), self.getY() + 1);
    }
    if(setup_last_point)
    {
	getTrooper(self).setLastPoint(target_point);
    }
    if((target_point.getX() == self.getX()) && (target_point.getY() == self.getY()))
    {
	return false;
    }
    cout << "Escaping..." << endl;
    move.setAction(model::MOVE);
    move.setX(target_point.getX());
    move.setY(target_point.getY());
    return true;
}

bool MyStrategy::tryLowStance(const model::Game& game,
			      model::Move& move,
			      const std::vector <model::Trooper>& teammates,
			      const std::vector <model::Trooper>& enemies,
			      const model::Trooper& self)
{
    if((self.getStance() == model::PRONE) || (self.getActionPoints() < game.getStanceChangeCost()))
    {
	return false;
    }
    if((self.getActionPoints() - game.getStanceChangeCost()) / self.getShootCost()
       != self.getActionPoints() / self.getShootCost())
    {
	Point from(self.getX(), self.getY());
	Point to;
	for(unsigned int i = 0; i < enemies.size(); ++i)
	{
	    to.set(enemies[i].getX(), enemies[i].getY());
	    if(enemies[i].isHoldingGrenade() && (from.getSquareDistance(to) <= 25))
	    {
		return false;
	    }
	}
    }
    if(self.getType() == model::FIELD_MEDIC)
    {
	if((teammates.size() == 1)/* || !healing_process*/)
	{
	    if(static_cast<int>(self.getActionPoints()) - static_cast<int>(game.getStanceChangeCost())
	       < static_cast<int>(self.getShootCost()))
	    {
		move.setAction(model::LOWER_STANCE);
		return true;
	    } else {
		model::TrooperStance stance = (self.getStance() == model::KNEELING) ?
		    model::PRONE : model::KNEELING;
		int available_enemies = getAvailableEnemiesCount(enemies, self.getShootingRange(),
								 self.getX(), self.getY(), stance);
		if(available_enemies > 0)
		{
		    move.setAction(model::LOWER_STANCE);
		    return true;;
		}
	    }
	}
    } else {
	if(static_cast<int>(self.getActionPoints()) - static_cast<int>(game.getStanceChangeCost())
	   < static_cast<int>(self.getShootCost()))
	{
	    move.setAction(model::LOWER_STANCE);
	    return true;
	} else {
	    model::TrooperStance stance = (self.getStance() == model::KNEELING)
		? model::PRONE : model::KNEELING;
	    int available_enemies = getAvailableEnemiesCount(enemies, self.getShootingRange(),
							     self.getX(), self.getY(), stance);
	    if(available_enemies > 0)
	    {
		move.setAction(model::LOWER_STANCE);
		return true;;
	    }
	}	
    }
    return false;
}

bool MyStrategy::tryRaiseStance(const model::Game& game,
				model::Move& move,
				const std::vector <model::Trooper>& teammates,
				const std::vector <model::Trooper>& enemies,
				const model::Trooper& self)
{
    if((self.getActionPoints() < game.getStanceChangeCost()) || (self.getStance() == model::STANDING))
    {
	return false;
    }
    move.setAction(model::RAISE_STANCE);
    return true;
}

model::Direction MyStrategy::getNextStep(int x1, int y1, int x2, int y2)
{
    Point current, prev;
    if((x1 == x2) && (y1 == y2))
    {
	return model::CURRENT_POINT;
    }
    current.set(x2, y2);
    prev.set(x1, y1);
    int current_num = used[current.getX()][current.getY()];
    do
    {
        if((current.getX() > 0) && (used[current.getX() - 1][current.getY()] == current_num - 1))
        {
            prev = current;
            current.set(current.getX() - 1, current.getY());
            --current_num;
            continue;
        }
        if((current.getX() < width - 1) && (used[current.getX() + 1][current.getY()] == current_num - 1))
        {
            prev = current;
            current.set(current.getX() + 1, current.getY());
            --current_num;
            continue;
        }
        if((current.getY() > 0) && (used[current.getX()][current.getY() - 1] == current_num - 1))
        {
            prev = current;
            current.set(current.getX(), current.getY() - 1);
            --current_num;
            continue;
        }
        if((current.getY() < height - 1) && (used[current.getX()][current.getY() + 1] == current_num - 1))
        {
            prev = current;
            current.set(current.getX(), current.getY() + 1);
            --current_num;
            continue;
        }
    } while(((current.getX() != x1) || (current.getY() != y1)) && (current_num > 1));

    if(prev.getX() > current.getX())
    {
        return model::EAST;
    }
    if(prev.getX() < current.getX())
    {
        return model::WEST;
    }
    if(prev.getY() > current.getY())
    {
        return model::SOUTH;
    }
    if(prev.getY() < current.getY())
    {
        return model::NORTH;
    }
    return model::CURRENT_POINT;
}

void MyStrategy::updateDistance(const std::vector <std::vector <model::CellType> >& field,
                                int used[30][20], int x, int y)
{
    for(int i = 0; i < width; ++i)
    {
        for(int j = 0; j < height; ++j)
        {
            used[i][j] = 0;
        }
    }
    used[x][y] = 1;
    queue <Point> q;
    Point current(x, y);
    Point next, prev;
    q.push(current);
    while(!q.empty())
    {
        current = q.front();
        q.pop();
        if((current.getX() > 0) && (used[current.getX() - 1][current.getY()] == 0)
           && (field[current.getX() - 1][current.getY()] == model::FREE))
        {
            next.set(current.getX() - 1, current.getY());
            used[next.getX()][next.getY()] = used[current.getX()][current.getY()] + 1;
            q.push(next);
        }
        if((current.getX() < width - 1) && (used[current.getX() + 1][current.getY()] == 0)
           && (field[current.getX() + 1][current.getY()] == model::FREE))
        {
            next.set(current.getX() + 1, current.getY());
            used[next.getX()][next.getY()] = used[current.getX()][current.getY()] + 1;
            q.push(next);
        }
        if((current.getY() > 0) && (used[current.getX()][current.getY() - 1] == 0)
           && (field[current.getX()][current.getY() - 1] == model::FREE))
        {
            next.set(current.getX(), current.getY() - 1);
            used[next.getX()][next.getY()] = used[current.getX()][current.getY()] + 1;
            q.push(next);
        }
        if((current.getY() < height - 1) && (used[current.getX()][current.getY() + 1] == 0)
           && (field[current.getX()][current.getY() + 1] == model::FREE))
        {
            next.set(current.getX(), current.getY() + 1);
            used[next.getX()][next.getY()] = used[current.getX()][current.getY()] + 1;
            q.push(next);
        }
    }
}

Point MyStrategy::updateTargetPoints(const std::vector <std::vector <model::CellType> >& field,
                                     const std::vector <model::Bonus>& bonuses,
                                     const std::vector <model::Trooper>& teammates,
                                     const std::vector <model::Trooper>& enemies,
                                     const model::Trooper& self,
				     bool force_update)
{
    Point target_point;
    Point current_point;
    int used[30][20];
    bool have_suitable_bonus = false;
    for(unsigned int i = 0; i < bonuses.size(); ++i)
    {
        for(unsigned int j = 0; j < teammates.size(); ++j)
        {
            if((teammates[j].getHitpoints() > 0) && (!isHoldingBonus(teammates[j], bonuses[i].getType())))
            {
                have_suitable_bonus = true;
            }
        }
    }
    if(enemies.size() > 0)
    {
        cout << "Going to enemies" << endl;
        for(unsigned int i = 0; i < teammates.size(); ++i)
        {
            bool visible_found = false;
            for(unsigned int j = 0; j < enemies.size(); ++j)
            {
                if(world -> isVisible(teammates[i].getShootingRange(), teammates[i].getX(),
                             teammates[i].getY(), teammates[i].getStance(),
                             enemies[j].getX(), enemies[j].getY(), enemies[j].getStance()))
                {
                    visible_found = true;
                    break;
                }
            }
            if(!visible_found)
            {
                updateDistance(field, used, teammates[i].getX(), teammates[i].getY());
                int min_distance = INT_MAX;
                for(int j = 0; j < width; ++j)
                {
                    for(int k = 0; k < height; ++k)
                    {
                        for(unsigned int l = 0; l < enemies.size(); ++l)
                        {
                            if(world -> isVisible(teammates[i].getShootingRange(),
                                               j, k,
                                               teammates[i].getStance(),
                                               enemies[l].getX(), enemies[l].getY(),
                                               enemies[l].getStance())
                               && (used[j][k] > 0) && (used[j][k] < min_distance)
                               && isFreeTargetPoint(j, k))
                            {
                                min_distance = used[j][k];
                                target_point.set(j, k);
                            }
                        }
                    }
                }
                getTrooper(teammates[i]).setTargetPoint(target_point);
            }
        }
    } else if(bonuses.empty() || !have_suitable_bonus)
    {
        cout << "No bonuses" << endl;
        // Если цель ещё не достигнута, то выход.
        for(unsigned int i = 0; i < teammates.size(); ++i)
        {
            if((getTrooperTargetPoint(teammates[i]).getX() != -1)
               && getTrooperTargetPoint(teammates[i]) != getTrooperPos(teammates[i])
               // && (getTrooperTargetPoint(teammates[i]).getSquareDistance(getTrooperPos(teammates[i])) > 8)
               && (teammates[i].getHitpoints() > 0)
               && !done_scouting && !force_update)
            {
                return getTrooperTargetPoint(self);
            }
        }
        if(done_scouting)
        {
            if(self.getType() == model::COMMANDER)
            {
                done_scouting = false;
                Point player_pos;
                for(unsigned int i = 0; i < players.size(); ++i)
                {
                    if((players[i].getId() != self.getPlayerId()) && (players[i].getApproximateX() != -1))
                    {
                        player_pos.set(players[i].getApproximateX(), players[i].getApproximateY());
                        break;
                    }
                }
                for(unsigned int j = 0; j < teammates.size(); ++j)
                {
                    target_point = getNearestTargetPoint(field, teammates,
                                                         player_pos.getX(),
                                                         player_pos.getY(),
                                                         teammates[j].getX(),
                                                         teammates[j].getY());
                    setTrooperTargetPoint(teammates[j],
                                          target_point.getX(), target_point.getY());
                }
            }
        } else if(!commander.isAlive())
        {
            cout << "Commander is dead!" << endl;
            bool left = rand() % 2;
            bool top = rand() % 2;
	    TrooperOptions teammate;
            for(unsigned int i = 0; i < teammates.size(); ++i)
            {
		teammate = getTrooper(teammates[i]);
		target_point.set(teammate.getStartPoint().getX(),
				 teammate.getStartPoint().getY());
                if(left)
                {
                    target_point.setX(width - teammate.getStartPoint().getX() - 1);
                } else {
                    target_point.setX(teammate.getStartPoint().getX());
                }
                if(top)
                {
                    target_point.setY(height - teammate.getStartPoint().getY() - 1);
                } else {
                    target_point.setY(teammate.getStartPoint().getY());
                }
                getTrooper(teammates[i]).setTargetPoint(target_point);
            }
        } else if(commander.isAlive()) 
        {
            need_scouting = true;
            Point commander_pos = getTrooperPos(getTrooper(teammates, model::COMMANDER));
            for(unsigned int i = 0; i < teammates.size(); ++i)
            {
                if(teammates[i].getType() == model::COMMANDER)
                {
                    getTrooper(teammates[i]).setTargetPoint(commander_pos);
                } else {
                    target_point = getNearestTargetPoint(field, teammates,
                                                         commander_pos.getX(),
                                                         commander_pos.getY(),
                                                         teammates[i].getX(),
                                                         teammates[i].getY());
                    getTrooper(teammates[i]).setTargetPoint(target_point);
                }
            }
            cout << "Commander: " << getTrooper(teammates, model::COMMANDER).getHitpoints() << endl;
        }
    } else {
        cout << "ok, some bonuses" << endl;
        // Если цель ещё не достигнута, то выход.
        for(unsigned int i = 0; i < teammates.size(); ++i)
        {
            if((getTrooperTargetPoint(teammates[i]).getX() != -1)
               && getTrooperTargetPoint(teammates[i]) != getTrooperPos(teammates[i])
               // && (getTrooperTargetPoint(teammates[i]).getSquareDistance(getTrooperPos(teammates[i])) > 8)
               && (bonusAt(bonuses, getTrooperTargetPoint(teammates[i]).getX(),
                           getTrooperTargetPoint(teammates[i]).getY()))
               && (teammates[i].getHitpoints() > 0)
	       && !force_update)
            {
                return getTrooperTargetPoint(self);
            }
        }

        bool found = false;
        for(unsigned int i = 0; i < bonuses.size() && !found; ++i)
        {
            for(unsigned int j = 0; j < teammates.size(); ++j)
            {
                if((teammates[j].getHitpoints() > 0)
                    &&!isHoldingBonus(teammates[j], bonuses[i].getType()))
                {
                    setTrooperTargetPoint(teammates[j], bonuses[i].getX(), bonuses[i].getY());
                    for(unsigned int k = 0; k < teammates.size(); ++k)
                    {
                        if(k != j)
                        {
                            target_point = getNearestTargetPoint(field,
                                                                 teammates,
                                                                 teammates[j].getX(),
                                                                 teammates[j].getY(),
                                                                 teammates[k].getX(),
                                                                 teammates[k].getY());
                            setTrooperTargetPoint(teammates[k], target_point.getX(), target_point.getY());
                        }
                    }
                    found = true;
                    break;
                }
            }
        }
    }

    target_point = getTrooperTargetPoint(self);
    return target_point;
}

Point MyStrategy::updateMedicTargetPoint(const std::vector <std::vector <model::CellType> >& field,
                                         const std::vector <model::Trooper>& teammates,
                                         const std::vector <model::Trooper>& enemies)
{
    Point target_point = getTrooper(getTrooper(teammates, model::FIELD_MEDIC)).getTargetPoint();
    Point medic_pos = getTrooperPos(getTrooper(teammates, model::FIELD_MEDIC));
    Point trooper_pos;
    int min_health = INT_MAX;
    for(unsigned int i = 0; i < teammates.size(); ++i)
    {
        if((teammates[i].getHitpoints() > 0) && (teammates[i].getHitpoints() < min_health))
        {
            min_health = teammates[i].getHitpoints();
            trooper_pos = getTrooperPos(teammates[i]);
        }       
    }
    if(target_point.getSquareDistance(trooper_pos) <= 1)
    {
        return target_point;
    }

    target_point = getNearestTargetPoint(field, teammates,
                                         trooper_pos.getX(), trooper_pos.getY(),
                                         medic_pos.getX(), medic_pos.getY());

    getTrooper(getTrooper(teammates, model::FIELD_MEDIC)).setTargetPoint(target_point);
    return target_point;
}

bool MyStrategy::isHoldingBonus(const model::Trooper& trooper, model::BonusType bonus_type)
{
    switch(bonus_type)
    {
    case model::GRENADE:
        return trooper.isHoldingGrenade();
        break;
    case model::MEDIKIT:
        return trooper.isHoldingMedikit();
        break;
    case model::FIELD_RATION:
        return trooper.isHoldingFieldRation();
        break;
    case model::UNKNOWN_BONUS:
        break;
    }
    return false;
}

void MyStrategy::setTrooperTargetPoint(const model::Trooper& trooper, int x, int y)
{
    Point point(x, y);
    switch(trooper.getType())
    {
    case model::COMMANDER:
        commander.setTargetPoint(point);
        break;
    case model::FIELD_MEDIC:
        medic.setTargetPoint(point);
        break;
    case model::SOLDIER:
        soldier.setTargetPoint(point);
        break;
    case model::SNIPER:
        sniper.setTargetPoint(point);
        break;
    case model::SCOUT:
        scout.setTargetPoint(point);
        break;
    default:
        break;
    }
}

Point MyStrategy::getTrooperTargetPoint(const model::Trooper& trooper)
{
    switch(trooper.getType())
    {
    case model::COMMANDER:
        return commander.getTargetPoint();
        break;
    case model::FIELD_MEDIC:
        return medic.getTargetPoint();
        break;
    case model::SOLDIER:
        return soldier.getTargetPoint();
        break;
    case model::SNIPER:
        return sniper.getTargetPoint();
        break;
    case model::SCOUT:
        return scout.getTargetPoint();
        break;
    }    
}

Point MyStrategy::getNearestTargetPoint(const std::vector <std::vector <model::CellType> >& field,
                                        const std::vector <model::Trooper>& teammates,
                                        int x1, int y1, int x2, int y2)
{
    Point target_point;
    int used[30][20];
    int minDist = INT_MAX;
    updateDistance(field, used, x1, y1);
    for(int i = -2; i <= 2; ++i)
    {
        for(int j = -2; j <= 2; ++j)
        {
            if(!((i == 0) && (j == 0)))
            {
                if((x1 + i >= 0) && (x1 + i < width) && (y1 + j >= 0) && (y1 + j < height))
                {
                    if((field[x1 + i][y1 + j] == model::FREE) && isFreeTargetPoint(x1 + i, y1 + j)
                       && (used[x1 + i][y1 + j] < minDist))
                    {
                        minDist = used[x1 + i][y1 + j];
                        target_point.set(x1 + i, y1 + j);
                    }
                }
            }
        }
    }
    return target_point;
}

Point MyStrategy::getTrooperPos(const model::Trooper& trooper)
{
    Point point(trooper.getX(), trooper.getY());
    return point;
}

void MyStrategy::setupStartPoints(const std::vector <model::Trooper> &troopers)
{
    Point point;
    if(commander.getStartPoint().getX() == -1)
    {
	cout << "Start points set." << endl;
        for(unsigned int i = 0; i < troopers.size(); ++i)
        {
            point.set(troopers[i].getX(), troopers[i].getY());
            getTrooper(troopers[i]).setStartPoint(point);
        }
    }
}

TrooperOptions& MyStrategy::getTrooper(const model::Trooper& trooper)
{
    switch(trooper.getType())
    {
    case model::COMMANDER:
        return commander;
	break;
    case model::FIELD_MEDIC:
        return medic;
        break;
    case model::SOLDIER:
        return soldier;
        break;
    case model::SNIPER:
        return sniper;
        break;
    case model::SCOUT:
        return scout;
        break;
    default:
        break;
    }
    return commander;
}

model::Trooper MyStrategy::getTrooper(const vector <model::Trooper>& troopers, model::TrooperType trooper_type)
{
    for(unsigned int i = 0; i < troopers.size(); ++i)
    {
        if(troopers[i].getType() == trooper_type)
        {
            return troopers[i];
        }
    }
    return troopers[0];
}

bool MyStrategy::bonusAt(const std::vector <model::Bonus>& bonuses, int x, int y)
{
    for(unsigned int i = 0; i < bonuses.size(); ++i)
    {
        if((bonuses[i].getX() == x) && (bonuses[i].getY() == y))
        {
            return true;
        }
    }
    return false;
}

bool MyStrategy::isFreeTargetPoint(int x, int y)
{
    Point target_point = commander.getTargetPoint();
    if((target_point.getX() == x) && (target_point.getY() == y) && commander.isAlive())
    {
        return false;
    }
    target_point = medic.getTargetPoint();
    if((target_point.getX() == x) && (target_point.getY() == y) && medic.isAlive())
    {
        return false;
    }
    target_point = soldier.getTargetPoint();
    if((target_point.getX() == x) && (target_point.getY() == y) && soldier.isAlive())
    {
        return false;
    }
    return true;
}
 
bool MyStrategy::isFreeCell(const vector <model::Trooper>& teammates, int x, int y)
{
    for(unsigned int i = 0; i < teammates.size(); ++i)
    {
        if((teammates[i].getX() == x) && (teammates[i].getY() == y))
        {
            return false;
        }
    }
    return true;
}

bool MyStrategy::tryThrowGrenade(model::Move move, const std::vector <model::Trooper>& enemies, int x, int y)
{
    Point from(x, y);
    Point to;
    for(unsigned int i = 0; i < enemies.size(); ++i)
    {
	to.set(enemies[i].getX(), enemies[i].getY());
	if(from.getSquareDistance(to) <= 25)
	{
	    return true;
	}
    }
    return false;
}

int MyStrategy::getMoveCost(model::TrooperStance stance, const model::Game& game)
{
    switch(stance)
    {
    case model::PRONE:
	return game.getProneMoveCost();
	break;
    case model::KNEELING:
	return game.getKneelingMoveCost();
	break;
    case model::STANDING:
	return game.getStandingMoveCost();
	break;
    }
    cout << "Error: Unknown stance type." << endl;
    return 0;
}

int MyStrategy::getDangerEnemiesCount(const std::vector <model::Trooper>& enemies,
				      int x, int y, model::TrooperStance stance)
{
    int result = 0;
    for(unsigned int i = 0; i < enemies.size(); ++i)
    {
	if(world -> isVisible(enemies[i].getShootingRange(),
			      enemies[i].getX(), enemies[i].getY(),
			      enemies[i].getStance(),
			      x, y, stance))
	{
	    ++result;
	}
    }
    return result;
}

int MyStrategy::getAvailableEnemiesCount(const std::vector <model::Trooper>& enemies,
			     double range, int x, int y, model::TrooperStance stance)
{
    int result = 0;
    for(unsigned int i = 0; i < enemies.size(); ++i)
    {
	if(world -> isVisible(range, x, y, stance,
			      enemies[i].getX(), enemies[i].getY(),
			      enemies[i].getStance()))
	{
	    ++result;
	}
    }
    return result;
}
