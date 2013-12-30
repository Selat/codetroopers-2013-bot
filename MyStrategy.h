#ifndef MY_STRATEGY_H
#define MY_STRATEGY_H

#include "Strategy.h"
#include "Point.h"
#include <vector>

class TrooperOptions;

class MyStrategy : public Strategy {
public:
    MyStrategy();
    void move(const model::Trooper& self, const model::World& world, const model::Game& game, model::Move& move);
private:
    unsigned int chooseEnemy(const std::vector <model::Trooper>& enemies,
			     const std::vector <model::Trooper>& teammates);
    void updateAliveTroopers(const std::vector <model::Trooper>& teammates);
    bool tryHeal(const model::Game& game,
		 model::Move& move,
		 const std::vector <model::Trooper>& teammates,
		 const std::vector <model::Trooper>& enemies,
		 const model::Trooper& self);
    bool tryEscape(const model::Game& game,
		   model::Move& move,
		   const std::vector <std::vector <model::CellType> >& field,
		   const std::vector <model::Trooper>& teammates,
		   const std::vector <model::Trooper>& enemies,
		   const model::Trooper& self,
		   bool setup_last_point);
    bool tryLowStance(const model::Game& game,
		      model::Move& move,
		      const std::vector <model::Trooper>& teammates,
		      const std::vector <model::Trooper>& enemies,
		      const model::Trooper& self);
    bool tryRaiseStance(const model::Game& game,
			model::Move& move,
			const std::vector <model::Trooper>& teammates,
			const std::vector <model::Trooper>& enemies,
			const model::Trooper& self);
    model::Direction getNextStep(int x1, int y1, int x2, int y2);
    void updateDistance(const std::vector <std::vector <model::CellType> >& field,
			int used[30][20], int x, int y);
    Point updateTargetPoints(const std::vector <std::vector <model::CellType> >& field,
			     const std::vector <model::Bonus>& bonuses,
			     const std::vector <model::Trooper>& teammates,
			     const std::vector <model::Trooper>& enemies,
			     const model::Trooper& self,
			     bool force_update);
    Point updateMedicTargetPoint(const std::vector <std::vector <model::CellType> >& field,
				 const std::vector <model::Trooper>& teammates,
				 const std::vector <model::Trooper>& enemies);
    bool isHoldingBonus(const model::Trooper& trooper, model::BonusType bonus_type);
    void setTrooperTargetPoint(const model::Trooper& trooper, int x, int y);
    Point getTrooperTargetPoint(const model::Trooper& trooper);
    Point getNearestTargetPoint(const std::vector <std::vector <model::CellType> >& field,
				const std::vector <model::Trooper>& teammates,
				int x1, int y1, int x2, int y2);

    Point getTrooperPos(const model::Trooper& trooper);

    void setupStartPoints(const std::vector <model::Trooper>& troopers);

    TrooperOptions& getTrooper(const model::Trooper& trooper);
    model::Trooper getTrooper(const std::vector <model::Trooper>& troopers, model::TrooperType trooper_type);
    
    bool bonusAt(const std::vector <model::Bonus>& bonuses, int x, int y);
    bool isFreeTargetPoint(int x, int y);
    bool isFreeCell(const std::vector <model::Trooper>& teammates, int x, int y);
    bool tryThrowGrenade(model::Move move, const std::vector <model::Trooper>& enemies, int x, int y);

    int getMoveCost(model::TrooperStance stance, const model::Game& game);
    int getDangerEnemiesCount(const std::vector <model::Trooper>& enemies,
			      int x, int y, model::TrooperStance stance);
    int getAvailableEnemiesCount(const std::vector <model::Trooper>& enemies,
				 double range, int x, int y, model::TrooperStance stance);

    static int width, height;
    static TrooperOptions commander;
    static TrooperOptions soldier;
    static TrooperOptions medic;
    static TrooperOptions sniper;
    static TrooperOptions scout;

    const model::World* world;
    std::vector <model::Player> players;

    static bool need_scouting;
    static bool done_scouting;
    static bool healing_process;
    static model::TrooperType healing_target;

    int unchanged_steps_count;
    int laying_steps;
    int used[30][20];
};

#endif
