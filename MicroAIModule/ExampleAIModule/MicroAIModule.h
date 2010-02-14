#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <windows.h>

static bool analyzed;
static bool analysis_just_finished;
static BWTA::Region* home;
static BWTA::Region* enemy_base;
DWORD WINAPI AnalyzeThread();

class zealot
{
private:
	BWAPI::Unit* unit;

public:
	zealot();
	zealot(BWAPI::Unit* z, BWAPI::TilePosition h, BWAPI::TilePosition a);
	~zealot(void);

	bool isTargeted;
	bool inRetreat;
	BWAPI::Unit* lastAttackOrder;
	BWAPI::TilePosition home, away;

	//wrapped methods
	void attackUnit(BWAPI::Unit* u);
	void attackMove(BWAPI::TilePosition u);
	void rightClick(BWAPI::TilePosition p);
	void rightClick(BWAPI::Position p);
	BWAPI::Position getPosition();
	bool exists();
	

	//new methods
	void checkShields();
	void checkTargeted();
	void act();
	bool isAttackingUnit(BWAPI::Unit* u);
	virtual bool retreat();
	virtual BWAPI::TilePosition getSafeZone(int runDistance);
	virtual std::set<BWAPI::Unit*> getAttackers();
	virtual BWAPI::Unit* getNearestEnemy();


	static const int lowShields = 20;
	static const int goodShields = 40;
};


class Platoon
{

	public:
		
		Platoon();
		//Platoon(std::set<BWAPI::Unit*> units, BWAPI::TilePosition h, BWAPI::TilePosition a);
		Platoon(std::set<BWAPI::Unit*> units, BWAPI::TilePosition h, BWAPI::TilePosition a, BWAPI::Color* c);
		
		void act();
		void setRally(BWAPI::TilePosition r);
		bool targetValid();
		BWAPI::Position center();
		double maxDistFromCenter();
		void acquireTarget();

		std::set<zealot*> zealots;
		int numUnits, numZealots;
		BWAPI::TilePosition home, away;
		BWAPI::Color* color;

		BWAPI::TilePosition rallyPoint;
		BWAPI::Unit* target;



};

class MicroAIModule : public BWAPI::AIModule
{
public:
  virtual void onStart();
  virtual void onEnd(bool isWinner);
  virtual void onFrame();
  virtual bool onSendText(std::string text);
  virtual void onPlayerLeft(BWAPI::Player* player);
  virtual void onNukeDetect(BWAPI::Position target);
  virtual void onUnitCreate(BWAPI::Unit* unit);
  virtual void onUnitDestroy(BWAPI::Unit* unit);
  virtual void onUnitMorph(BWAPI::Unit* unit);
  virtual void onUnitShow(BWAPI::Unit* unit);
  virtual void onUnitHide(BWAPI::Unit* unit);
  virtual void onUnitRenegade(BWAPI::Unit* unit);
  
  void drawStats(); //not part of BWAPI::AIModule
  void showPlayers();
  void showForces();
  bool show_visibility_data;

  //BWAPI::Unit* closestEnemy(BWAPI::Unit* unit);
  static std::set<BWAPI::Unit*> allEnemyUnits();

	
	BWAPI::TilePosition home, away;
	std::set<Platoon*> platoons;
	int frameNum, numUnits, eUnits;
	
	
};


