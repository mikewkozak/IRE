#pragma once

#include "GUI/GeneratedFiles/ui_QtWindow.h"
#include "GUI/QSignal.h"
#include "GUI/CustomItems.h"

#include "Search/ActionGenerator.h"
#include "Search/UnitInfoLearner.h"
#include "Search/ABCD.h"
#include "Search/MCTSCD.h"
#include "Search/EvaluationFunctionBasic.h"
#include "Search/CombatSimSustained.h"
#include "Search/CombatSimDecreased.h"
#include "Search/CombatSimLanchester.h"
#include "Search/TargetSorting.h"
#include "Search/EvalCombat.h"

#include "SparCraft.h"
#include "InformationManager.h"

class myQtApp : public QWidget, private Ui::myQtAppDLG
{
	Q_OBJECT

public:
	myQtApp(QWidget *parent = 0);
	~myQtApp();

	QGraphicsScene * mapScene;
	int regionIdSelected;

public slots:
	// Options
	void changeSpeed(int value);
	void pauseGame();
	void resumeGame();

	void changeDisplayBWTA(int state);
	void changeBuildMap(int state);
	void changeGroundDPS(int state);
	void changeAirDPS(int state);
	void changeBuildOrder(int state);
	void changeRegionId(int state);

	// Game State
	void changeBWTAdata();
	void drawPolygons(const std::vector<BWTA::Polygon*> &polygons, QGraphicsScene* scene);
	void drawPolygon(const BWTA::Polygon &p, QColor qc, QGraphicsScene* scene, double scale = 1);
	void drawCustomPolygon(const BWTA::Polygon &p, QGraphicsScene* scene, int regionId);
	void updateGameStateTable();
	void gameStateTabChanged(int tabIndex);
	void updateUnitMap();
	void updateUnitTable();
	void drawUnitEllipse(int x, int y, int xOffset, QColor color, int regionId, QGraphicsScene* scene);
	void updateEffectivenessMap();
	void drawCircleText(int x, int y, int xOffset, QColor color, int regionId, QGraphicsScene* scene);

	// Game Search
	void importState();
	void loadFile();
	void mctsSearch();

	void generateActions();
	void expandNode();
    void expandRandomNode();
	void executeAction();
	void rolloutGame();
    
	GameState getGameState(SparCraft::GameState sparCraftGameState);
	void addUnitsFromReplaytoGameState(std::map<int, unitInfo> armyUnits, GameState &myGameState, SparCraft::GameState &sparCraftState,
		int playerID, bool &hasMines, bool &hasDropships, bool &skipSparCraft,
		bool SKIP_TRANSPORTS, bool ONLY_SPARCRAFT_SUPPORTED, bool IMPORT_FULL_HP_SPARCAFT);
	void addCombatStats(Statistic &statsCombatLength, Statistic &statsCombatGroups, Statistic &statsCombatUnits, 
		const combatInfo &combat, const GameState &gameState);

	BWAPI::Position getCenterRegionId(int regionId);

	// Combat simulator
	void parseCombats();
	void clearCombatsParsed();
	void learnCombat();
	void getDPF();
	void combatSimulator();
	void testUnitSelected(int index);
	void addToCombat(QTableWidget* table);
	void addToCombat1(){ addToCombat(groupTestTable1); };
	void addToCombat2(){ addToCombat(groupTestTable2); };
	void clearGroup1();
	void clearGroup2();
	void testCombat();

private:
	// Tab BWTA -> Game State
	QStringList gameStateLabels;
	QStringList unitMapLabels;

	std::string spacePartition;

	ActionGenerator testActions;

	void printError(QTextEdit * textEdit, const QString & text);

	UnitInfoLearner learner;
	void crossValidation();

};
