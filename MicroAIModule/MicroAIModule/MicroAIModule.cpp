#include "MicroAIModule.h"
#include <Math.h>


using namespace BWAPI;
void MicroAIModule::onStart()
{
	
  Broodwar->sendText("Hello world!");
  Broodwar->printf("v3");
  // Broodwar->printf("The map is %s, a %d player map",Broodwar->mapName().c_str(),Broodwar->getStartLocations().size());
  // Enable some cheat flags
  Broodwar->enableFlag(Flag::UserInput);
  // Uncomment to enable complete map information
  //Broodwar->enableFlag(Flag::CompleteMapInformation);

  //read map information into BWTA so terrain analysis can be done in another thread
  BWTA::readMap();
  analyzed=false;
  analysis_just_finished=false;
  show_visibility_data=false;
	frameNum = 0;


	// Get home and away locations
	home = Broodwar->self()->getStartLocation();
	
	for(std::set<TilePosition>::iterator i=Broodwar->getStartLocations().begin();i!=Broodwar->getStartLocations().end();i++)
	{
		if (home.getDistance((*i))>1)
			away = (*i);
	}



	// Construct Zealot array
	numUnits = 0;
	for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
		numUnits++;

	std::set<Unit*> group = std::set<Unit*>();

	int at = 0;
	int platoonSize = 3;
	int platoonAt = 0;

	platoons = std::set<Platoon*>();
	
	//std::list<BWAPI::Color*> colors = std::list<BWAPI::Color*>();
	BWAPI::Color* r = new Color(Colors::Red);
	BWAPI::Color* g = new Color(Colors::Green);
	BWAPI::Color* b = new Color(Colors::Orange);
	
	BWAPI::Color** colors = new BWAPI::Color*[3];

	colors[0] = r;
	colors[1] = g;
	colors[2] = b;
	
	for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
	{
		group.insert((*i));
		at++;
		
		if(at%3==0)
		{
			Platoon* p = new Platoon(group, home, away, colors[platoons.size()]);
			platoons.insert(p);
			group.clear();
		}
		
	}
	
	Broodwar->sendText("num units = %i", numUnits);
	eUnits = 9;
  
	Broodwar->sendText("init correctly");
  
}
void MicroAIModule::onEnd(bool isWinner)
{
  if (isWinner)
	  Broodwar->sendText("gg");
  else
	  Broodwar->sendText("dang it!");

}

void MicroAIModule::onFrame()
{
	//Broodwar->sendText("framing");

	int split = 200;

	if (Broodwar->getFrameCount()<split)
	{
		Broodwar->printf("making me platoons");
		int dx = away.x()-home.x();
		int dy = away.y()-home.y();

		int num = -1;
		//Broodwar->sendText("drawing");
		Broodwar->drawCircleMap(Position(home).x(), Position(home).y(), 5, Colors::Black, true );
		//Broodwar->sendText("drawed");

		for (std::set<Platoon*>::iterator i=platoons.begin(); i!=platoons.end(); i++)
		{
			TilePosition t = TilePosition(home.x()+5*(dx>0 ? 1:-1)+(5)*(dx>0 ? 1:-1)*(num==0 ? 0:1), home.y()+num*7+1);
			Broodwar->drawCircleMap(Position(t).x(), Position(t).y(), 5, Colors::Red, false );
			(*i)->setRally(t);
			num++;
			//Broodwar->sendText("acting");
			(*i)->act();
			//Broodwar->sendText("acted");
		}

		//Broodwar->sendText("done framing");
		return;
	}

	if (Broodwar->getFrameCount()==split)
	{
		//Broodwar->printf("%i frames", frameNum);
		for (std::set<Platoon*>::iterator i=platoons.begin(); i!=platoons.end(); i++)
		{
			(*i)->setRally(away);
		}
	}

	//if (Broodwar->getFrameCount()%5>0)
	//	return;

	

	//Broodwar->printf("acting");
	for (std::set<Platoon*>::iterator i=platoons.begin(); i!=platoons.end(); i++)
	{
		(*i)->act();
	}

}


std::set<BWAPI::Unit*> MicroAIModule::allEnemyUnits()
{
	std::set<BWAPI::Unit*> enemies = Broodwar->getAllUnits();


	for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
		enemies.erase((*i));

	return enemies;
	
}

void MicroAIModule::onUnitCreate(BWAPI::Unit* unit)
{
  if (!Broodwar->isReplay())
    Broodwar->printf("A %s [%x] has been created at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
  else
  {
    /*if we are in a replay, then we will print out the build order
    (just of the buildings, not the units).*/
    if (unit->getType().isBuilding() && unit->getPlayer()->isNeutral()==false)
    {
      int seconds=Broodwar->getFrameCount()/24;
      int minutes=seconds/60;
      seconds%=60;
      Broodwar->printf("%.2d:%.2d: %s creates a %s",minutes,seconds,unit->getPlayer()->getName().c_str(),unit->getType().getName().c_str());
    }
  }
}
void MicroAIModule::onUnitDestroy(BWAPI::Unit* unit)
{	

	bool allUnitsAlive = Broodwar->self()->getUnits().size()==numUnits;

	/*
	for(int i = 0; i<numUnits; i++)
		if (unit==zealots[i].unit)
		{
			allUnitsAlive = false;
			break;
		}
	*/

	if(!allUnitsAlive)
	{
		/*
		zealot* aliveZealots = new zealot[numUnits-1];
		int at = 0;
		
		for(int i = 0; i<numUnits; i++)
		{
			if (unit!=zealots[i].unit)
			{
				aliveZealots[at] = zealots[i];
				at++;
			}
		}

		zealots = aliveZealots;
		*/
		numUnits--;
		Broodwar->printf("Lost Zealot");
	} else
	{
		eUnits--;
		Broodwar->printf("Destroyed Zealot");
	}
}

void MicroAIModule::onUnitMorph(BWAPI::Unit* unit)
{
  if (!Broodwar->isReplay())
    Broodwar->printf("A %s [%x] has been morphed at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
  else
  {
    /*if we are in a replay, then we will print out the build order
    (just of the buildings, not the units).*/
    if (unit->getType().isBuilding() && unit->getPlayer()->isNeutral()==false)
    {
      int seconds=Broodwar->getFrameCount()/24;
      int minutes=seconds/60;
      seconds%=60;
      Broodwar->printf("%.2d:%.2d: %s morphs a %s",minutes,seconds,unit->getPlayer()->getName().c_str(),unit->getType().getName().c_str());
    }
  }
}
void MicroAIModule::onUnitShow(BWAPI::Unit* unit)
{
	;
  //if (!Broodwar->isReplay())
    //Broodwar->printf("A %s [%x] has been spotted at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());

}
void MicroAIModule::onUnitHide(BWAPI::Unit* unit)
{
  //if (!Broodwar->isReplay())
  //  Broodwar->printf("A %s [%x] was last seen at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x(),unit->getPosition().y());
}
void MicroAIModule::onUnitRenegade(BWAPI::Unit* unit)
{
  if (!Broodwar->isReplay())
    Broodwar->printf("A %s [%x] is now owned by %s",unit->getType().getName().c_str(),unit,unit->getPlayer()->getName().c_str());
}
void MicroAIModule::onPlayerLeft(BWAPI::Player* player)
{
  Broodwar->printf("%s left the game.",player->getName().c_str());
}
void MicroAIModule::onNukeDetect(BWAPI::Position target)
{
  if (target!=Positions::Unknown)
    Broodwar->printf("Nuclear Launch Detected at (%d,%d)",target.x(),target.y());
  else
    Broodwar->printf("Nuclear Launch Detected");
}

bool MicroAIModule::onSendText(std::string text)
{
  if (text=="/show players")
  {
    showPlayers();
    return false;
  } else if (text=="/show forces")
  {
    showForces();
    return false;
  } else if (text=="/show visibility")
  {
    show_visibility_data=true;
  } else if (text=="/analyze")
  {
    if (analyzed == false)
    {
      Broodwar->printf("Analyzing map... this may take a minute");
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AnalyzeThread, NULL, 0, NULL);
    }
    return false;
  } else
  {
    Broodwar->printf("You typed '%s'!",text.c_str());
  }
  return true;
}

DWORD WINAPI AnalyzeThread()
{
  BWTA::analyze();
  analyzed   = true;
  analysis_just_finished = true;

  //self start location only available if the map has base locations
  if (BWTA::getStartLocation(BWAPI::Broodwar->self())!=NULL)
  {
    home       = BWTA::getStartLocation(BWAPI::Broodwar->self())->getRegion();
  }
  //enemy start location only available if Complete Map Information is enabled.
  if (BWTA::getStartLocation(BWAPI::Broodwar->enemy())!=NULL)
  {
    enemy_base = BWTA::getStartLocation(BWAPI::Broodwar->enemy())->getRegion();
  }
  return 0;
}

void MicroAIModule::drawStats()
{
  std::set<Unit*> myUnits = Broodwar->self()->getUnits();
  Broodwar->drawTextScreen(5,0,"I have %d units:",myUnits.size());
  std::map<UnitType, int> unitTypeCounts;
  for(std::set<Unit*>::iterator i=myUnits.begin();i!=myUnits.end();i++)
  {
    if (unitTypeCounts.find((*i)->getType())==unitTypeCounts.end())
    {
      unitTypeCounts.insert(std::make_pair((*i)->getType(),0));
    }
    unitTypeCounts.find((*i)->getType())->second++;
  }
  int line=1;
  for(std::map<UnitType,int>::iterator i=unitTypeCounts.begin();i!=unitTypeCounts.end();i++)
  {
    Broodwar->drawTextScreen(5,16*line,"- %d %ss",(*i).second, (*i).first.getName().c_str());
    line++;
  }
}

void MicroAIModule::showPlayers()
{
  std::set<Player*> players=Broodwar->getPlayers();
  for(std::set<Player*>::iterator i=players.begin();i!=players.end();i++)
  {
    Broodwar->printf("Player [%d]: %s is in force: %s",(*i)->getID(),(*i)->getName().c_str(), (*i)->getForce()->getName().c_str());
  }
}
void MicroAIModule::showForces()
{
  std::set<Force*> forces=Broodwar->getForces();
  for(std::set<Force*>::iterator i=forces.begin();i!=forces.end();i++)
  {
    std::set<Player*> players=(*i)->getPlayers();
    Broodwar->printf("Force %s has the following players:",(*i)->getName().c_str());
    for(std::set<Player*>::iterator j=players.begin();j!=players.end();j++)
    {
      Broodwar->printf("  - Player [%d]: %s",(*j)->getID(),(*j)->getName().c_str());
    }
  }
}




zealot::zealot()
{
	unit = NULL;
	inRetreat = false;
	isTargeted = false;
}

zealot::zealot(BWAPI::Unit* z, BWAPI::TilePosition h, BWAPI::TilePosition a)
{
	unit = z;
	inRetreat = false;
	isTargeted = false;
	home = h;
	away = a;

	lastAttackOrder = NULL;
}


zealot::~zealot(void)
{
}


BWAPI::Position zealot::getPosition()
{
	return unit->getPosition();
}



void zealot::checkShields()
{
	if(inRetreat)
	{
		if(unit->getShields()>=goodShields)
			inRetreat = false;
	} else
	{
		if(unit->getShields()<lowShields)
			inRetreat = true;
	}
}

void zealot::checkTargeted()
{


	// check for enemy units visible
	
	int numVisEnemies = Broodwar->getAllUnits().size() - Broodwar->self()->getUnits().size();

	if (numVisEnemies==0)
	{
		// No visible enemy units
		isTargeted = false;
		return;
	}


	std::set<BWAPI::Unit*> enemies = MicroAIModule::allEnemyUnits();
	
	
	isTargeted = false;
	for(std::set<Unit*>::iterator i=enemies.begin(); i!=enemies.end(); i++)
	{
		if((*i)==unit)
		{
			isTargeted = true;
			return;
		}
	}


}

void zealot::attackUnit(BWAPI::Unit* u)
{
	unit->attackUnit(u);
	lastAttackOrder = u;
}


bool zealot::isAttackingUnit(BWAPI::Unit* u)
{
	return lastAttackOrder == u;
}
void zealot::attackMove(BWAPI::TilePosition u)
{
	unit->attackMove(u);
}


void zealot::rightClick(BWAPI::TilePosition p)
{
	unit->rightClick(p);
	lastAttackOrder = NULL;
}

void zealot::rightClick(BWAPI::Position p)
{
	unit->rightClick(p);
	lastAttackOrder = NULL;
}

bool zealot::exists()
{
	return unit->exists();
}


void zealot::act()
{

	checkShields();
	checkTargeted();
	
	if(false && isTargeted && inRetreat)
	{
		// Retreat away from closest enemy
		BWAPI::TilePosition here = unit->getTilePosition();
		BWAPI::TilePosition enemy = getNearestEnemy()->getTilePosition();

		TilePosition* moveTo = new TilePosition( here.x()+(here.x()-enemy.x()), here.y()+(here.y()-enemy.y()) );
		unit->rightClick(*moveTo);
		return;
	}


	BWAPI::Unit* target = getNearestEnemy();


	if (target!=NULL && unit->getTarget()==target)
	{
		// already attacking closest unit
		return;
	}

	

	if(target==NULL)
	{
		attackMove(away);
	} else
	{
		attackUnit(target);
	}

}




bool zealot::retreat()
{

  int framesPerAct = 5;					//Number of frames per execution of algorithm
  float threshold = 2;					//Scale factor for retreat threshold vs shield differential
  static int shields_then = 0;			//Shields 20 frames ago
  float shields_differential = 0;		//How much shields lost in last 20 frames, scaled up
  static bool now = true;				//Turns false upon taking first instance of damage, turns true when healed up
  bool canattack = false;

  BWAPI::TilePosition safeZone = Broodwar->self()->getStartLocation();

  if (true || Broodwar->getFrameCount()%framesPerAct==0)
  {
	//Broodwar->printf("Current frame: %d",Broodwar->getFrameCount());
    int j = 0;
	//******************************
	if (unit->getType()== UnitTypes::Protoss_Zealot)
	{
		if (unit->getShields() < unit->getType().maxShields())
		{
  			if (now)
			{
				//We keep saving shields as "shields_then" until now is flipped
				shields_then = unit->getShields();

				if (unit->getShields() > unit->getType().maxShields()/2)
				{
					//get ready to go back in when shields are above certain threshold
					 now = true;
				}
				else
				{
					//get ready to check differential and prepare to retreat
					  now = false;
				}
			}
			else
			{
				//Shields have changed, check differential
				shields_differential = threshold*(unit->getShields() - shields_then);
				shields_then = unit->getShields();
			}

		} else
			now = true;

	if (unit->getOrder()!=Orders::AttackUnit || shields_differential <= 0)
	{	  
	  if(getAttackers().size()<4 && unit->getGroundWeaponCooldown()<5)
	  {
		return false;
		//Broodwar->printf("Attackers dropped below count and attack cooled down, attacking");
	  }
	}
	//Broodwar->printf("Zealot (%d) is %s-ing. Shields at %d, retreat threshold @ %f. Number of Attackers: %d",
	//				j,(*i)->getOrder().getName().c_str(),(*i)->getShields(),shields_differential[j], getAttackers(*i).size());
	if (unit->getOrder() == Orders::PlayerGuard)
	{
		Broodwar->printf("player guarding");
		return false;
	}
	if (	  (unit->getShields() <= shields_differential && unit->getOrder() != Orders::Move) )
	{
			safeZone = getSafeZone(300);
			rightClick(safeZone);
			if(unit->getShields()<3)
				Broodwar->drawCircleMap(Position(safeZone).x(), Position(safeZone).y(), 5, Colors::Red, true);
			Broodwar->drawCircleMap(Position(safeZone).x(), Position(safeZone).y(), 5, Colors::Yellow, true);
			
			return true;
	}

		if ((unit->getGroundWeaponCooldown()>5)	 )
		{
			safeZone = getSafeZone(300);
			rightClick(safeZone);
			Broodwar->drawCircleMap(Position(safeZone).x(), Position(safeZone).y(), 5, Colors::White, true);
			return true;
		}
	}
  }
  return false;

}

//int MicroAIModule::getCdThreshold(BWAPI::Unit* unit, BWAPI::TilePosition)

BWAPI::TilePosition zealot::getSafeZone(int runDistance)
{
  BWAPI::TilePosition returnSafeZone = Broodwar->self()->getStartLocation();
  std::set<BWAPI::Unit*> enemyUnits = Broodwar->getAllUnits(), myUnits = Broodwar->self()->getUnits();

  double *hostileDist = new double [enemyUnits.size()];
  double *weight = new double [enemyUnits.size()];
  double *otherX = new double [enemyUnits.size()];
  double *otherY = new double [enemyUnits.size()];
  int hereX = unit->getPosition().x(), hereY = unit->getPosition().y();
  double safeX = 0, safeY = 0;

  for(std::set<Unit*>::const_iterator i=enemyUnits.begin();i!=enemyUnits.end();i++)
  {
	  if( Broodwar->self()->getUnits().find(*i) != Broodwar->self()->getUnits().end())
		  enemyUnits.erase(*i);
  }
  myUnits.erase(myUnits.find(unit));

  int i = 0;
  for(std::set<Unit*>::const_iterator E=enemyUnits.begin();E!=enemyUnits.end();E++)
  {
	  hostileDist[i] = unit->getDistance(*E);
	weight[i] = runDistance/(10+hostileDist[i]);
	otherX[i] = (hereX-(*E)->getPosition().x())/hostileDist[i];
	otherY[i] = (hereY-(*E)->getPosition().y())/hostileDist[i];

	safeX += weight[i]*otherX[i];
	safeY += weight[i]*otherY[i];
	i++;
  }
  safeX /= enemyUnits.size();
  safeY /= enemyUnits.size();
  safeX += hereX;
  safeY += hereY;

  //Broodwar->drawLineMap(int(hereX),int(hereY),int(safeX),int(safeY),Colors::Red);
  BWAPI::Position safe = BWAPI::Position(int(safeX),int(safeY));
  //Broodwar->printf("Distance to safeZone = %f",unit->getDistance(safe));
  returnSafeZone = BWAPI::TilePosition(safe);
  return returnSafeZone;
}

/*
BWAPI::TilePosition zealot::getEnemyZone()
{
  BWAPI::TilePosition returnEnemyZone = enemy_start;
  std::set<BWAPI::Unit*> enemyUnits = Broodwar->getAllUnits(), myUnits = Broodwar->self()->getUnits();

  double enemyX = 0, enemyY = 0;

  for(std::set<Unit*>::const_iterator i=enemyUnits.begin();i!=enemyUnits.end();i++)
  {
	  if( Broodwar->self()->getUnits().find(*i) != Broodwar->self()->getUnits().end())
		  enemyUnits.erase(*i);
  }
  if (enemyUnits.empty())
	  return returnEnemyZone;

  int i = 0;
  for(std::set<Unit*>::const_iterator E=enemyUnits.begin();E!=enemyUnits.end();E++)
  {
	enemyX += (*E)->getPosition().x();
	enemyY += (*E)->getPosition().y();
	i++;
  }
  enemyX /= enemyUnits.size();
  enemyY /= enemyUnits.size();

  BWAPI::Position enemy = BWAPI::Position(int(enemyX),int(enemyY));
  returnEnemyZone = BWAPI::TilePosition(enemy);
  return returnEnemyZone;
}
*/

std::set<BWAPI::Unit*> zealot::getAttackers()
{
	std::set<BWAPI::Unit*> enemyUnits = MicroAIModule::allEnemyUnits();

  for(std::set<Unit*>::const_iterator i=enemyUnits.begin();i!=enemyUnits.end();i++)
  {
  	if (/*(*i)->getOrderTarget() == unit||*/(*i)->getTarget() != unit)
	  enemyUnits.erase(*i);
  }
  return enemyUnits;
}


BWAPI::Unit* zealot::getNearestEnemy()
{
  std::set<BWAPI::Unit*> enemyUnits = Broodwar->getAllUnits(), myUnits = Broodwar->self()->getUnits();  
  
  for(std::set<Unit*>::const_iterator i=enemyUnits.begin();i!=enemyUnits.end();i++)
  {
	if( Broodwar->self()->getUnits().find(*i) != Broodwar->self()->getUnits().end())
	  enemyUnits.erase(*i);
  }
  
  BWAPI::Unit* closestEnemy = *(enemyUnits.begin());
  
  for(std::set<Unit*>::const_iterator i=enemyUnits.begin();i!=enemyUnits.end();i++)
  {
	if( closestEnemy->getDistance(unit) > (*i)->getDistance(unit))
	  closestEnemy = (*i);
  }
  return closestEnemy;
}




Platoon::Platoon()
{
	numZealots = 0;
}

Platoon::Platoon(std::set<BWAPI::Unit*> units, BWAPI::TilePosition h, BWAPI::TilePosition a, BWAPI::Color* c = NULL)
{
	if (c==NULL)
	{
		Broodwar->printf("null color");
		c = new Color(Colors::Black);
	}
	home = h;
	away = a;
	numUnits = units.size();
	target = NULL;
	away = a;
	rallyPoint = away;
	color = new Color(*c);


	numZealots = 0;

	for(std::set<Unit*>::const_iterator i=units.begin();i!=units.end();i++)
	{
		// is zealot? (id=65)
		if ((*i)->getType().getID()==65)
		{
			numZealots++;
			zealots.insert(new zealot((*i), home, away));
		}
	}

	Broodwar->printf("Platoon created. Units: %i; Zealots: %i", numUnits, numZealots);
}

void Platoon::setRally(BWAPI::TilePosition r)
{
	rallyPoint = r;
}

double Platoon::maxDistFromCenter()
{
	double maxDistance = 0;

	BWAPI::Position c = center();

	for(std::set<zealot*>::const_iterator i=zealots.begin(); i!=zealots.end(); i++)
	{
		BWAPI::Position here = (*i)->getPosition();
		double distance = here.getDistance(c);
		if (distance>maxDistance)
			maxDistance = distance;
	}

	return maxDistance;

}

BWAPI::Position Platoon::center()
{
	if (numUnits==0)
		return Position(0,0);

	int x = 0;
	int y = 0;

	for(std::set<zealot*>::const_iterator i=zealots.begin(); i!=zealots.end(); i++)
	{
		x += (*i)->getPosition().x();
		y += (*i)->getPosition().y();
	}

	
	return Position(x/numUnits, y/numUnits);
}

bool Platoon::targetValid()
{
	// No target
	if (target==NULL)
		return false;


	// Target does exist
	if (Broodwar->getAllUnits().find(target)==Broodwar->getAllUnits().end())
	{
		//Broodwar->printf("Unit does not exist no more");
		return false;
	}
			
	// Target far away
	BWAPI::Position there = target->getPosition();
	for(std::set<zealot*>::const_iterator i=zealots.begin(); i!=zealots.end(); i++)
	{
		BWAPI::Position here = (*i)->getPosition();
		double distance = here.getDistance(there);
		//Broodwar->printf("%f", distance);
		
		if (distance>100)
		{
			//Broodwar->printf("Target too far away");
			return false;
		}
	}

	return true;
}


void Platoon::acquireTarget()
{

	double maxScore = -1;
	std::set<BWAPI::Unit*> enemies = MicroAIModule::allEnemyUnits();

	for(std::set<BWAPI::Unit*>::iterator i = enemies.begin(); i!=enemies.end(); i++)
	{	
		double distFromCenter = (*i)->getPosition().getDistance(center());
		int health = (*i)->getHitPoints();
		int shields = (*i)->getShields();

		double score = 50000/distFromCenter-health-shields-50*(shields==60 ? 1:0);

		if (score>maxScore)
		{
			target = *i;
			maxScore = score;
		}
	}

	//Broodwar->printf("max score: %f", maxScore);

}

void Platoon::act()
{
	//Draw center of platoon
	Broodwar->drawCircleMap(center().x(), center().y(), 5, *color, true);
	//Broodwar->drawCircleMap(center().x(), center().y(), 150, *color, false);

	for(std::set<zealot*>::const_iterator i=zealots.begin(); i!=zealots.end(); i++)
	{
		
		

		//Broodwar->sendText("zealot");
		if(!(*i)->exists())
		{
			Broodwar->printf("unit died");
			zealots.erase((*i));
			numUnits--;
			numZealots--;
			continue;
		}
		
		

		// Draw zealot
		Broodwar->drawCircleMap((*i)->getPosition().x(), (*i)->getPosition().y(), 2, *color, true);


		// Draw target if it exists
		if(target!=NULL)
		{
			int x = target->getPosition().x();
			int y = target->getPosition().y();

			Broodwar->drawBoxMap(x-2, y-2, x+2, y+2, *color, true);
		}


		if ((*i)->retreat())
		{
			//Broodwar->sendText("retreat");
			continue;
		}
		
		// If platoon spread out and not engaged in active battle, regroup
		if(maxDistFromCenter()>1000 && !targetValid())
		{
			Broodwar->printf("moving center");
			(*i)->rightClick(center());
			continue;
		}
				

		
		BWAPI::Position here = (*i)->getPosition();
		
		// If no need to retarget
		if (targetValid() && (*i)->isAttackingUnit(target))
			continue;

		//Broodwar->printf("%d", target->getPosition().getDistance((*i)->getPosition()));

		if(!targetValid())
		{
			acquireTarget();
			/*
			if (target!=NULL)
				Broodwar->printf("%d", here.getApproxDistance(target->getPosition()));
			*/

			/*
			BWAPI::Unit* u = (*i)->closestEnemy();

			if (u!=NULL)
				target = u;
			*/
		}

		if(target!=NULL)
		{
			//Broodwar->sendText("attack platoon target");
			//(*i)->attackMove(target->getTilePosition());
			(*i)->attackUnit(target);
		}
		else
		{
			//Broodwar->printf("moving to platoon rally");

			double x = Position(rallyPoint).x()-here.x();
			double y = Position(rallyPoint).y()-here.y();

			double xunit = x/sqrt(x*x+y*y);
			double yunit = y/sqrt(x*x+y*y);


			//Broodwar->printf("Unit: %f, %f", xunit, yunit);
			(*i)->rightClick(Position(int (here.x()+100*xunit), int (here.y()+100*yunit)));
		}

	}
}