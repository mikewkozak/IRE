#include "QtWindow.h"

#include <numeric> // for std::accumulate

myQtApp::myQtApp(QWidget *parent)
	:regionIdSelected(-1),
	learner(unitStatic->typeDPF)
{
	spacePartition = LoadConfigString("high_level_search", "space_partition", "REGIONS_AND_CHOKEPOINTS");

	// this sets up GUI
	setupUi(this); 

	// -------------------- INITIALIZING VARIABLES --------------------
	// Tab Options
	chkMapAnalysisBWTA->setChecked(false);
	chkBuildMap->setChecked(PRINT_BUILD_MAP);
	chkGroundDPS->setChecked(PRINT_GROUND_DPS);
	chkAirDPS->setChecked(PRINT_AIR_DPS);
	chkBuildOrder->setChecked(PRINT_BUILD_ORDER);
	// Tab Game State
	mapScene = new QGraphicsScene();
	chkRegionIdMap->setChecked(PRINT_REGION_ID_MAP);

	gameStateLabels << tr("Unit type") << tr("Number") << tr("Region") << tr("Order") << tr("Target") << tr("End frame");
	unitsToLocationTable->setColumnCount(6);
	unitsToLocationTable->setHorizontalHeaderLabels(gameStateLabels);
	unitsToLocationTable->setColumnWidth(0,150);

	unitsToLocationTable_2->setColumnCount(6);
	unitsToLocationTable_2->setHorizontalHeaderLabels(gameStateLabels);
	unitsToLocationTable_2->setColumnWidth(0,150);

	unitMapLabels << tr("Unit type") << tr("Number") << tr("Order") << tr("End frame");
	unitsToLocationTable_3->setColumnCount(4);
	unitsToLocationTable_3->setHorizontalHeaderLabels(unitMapLabels);
	unitsToLocationTable_3->setColumnWidth(0,150);

	unitsToLocationTable_4->setColumnCount(4);
	unitsToLocationTable_4->setHorizontalHeaderLabels(unitMapLabels);
	unitsToLocationTable_4->setColumnWidth(0,150);

	// Default data for Effectiveness tab
	QStringList unitTypesLabels;
	unitTypesLabels << tr("SVC") << tr("Marine") << tr("Medic") << tr("Firebat") << tr("Ghost")
					<< tr("Vulture") << tr("Tank") << tr("Goliath") << tr("Wraith") << tr("Science Vessel")
					<< tr("Battlecruiser") << tr("Valkyrie");
	unitEffecTable->setColumnCount(12);
	unitEffecTable->setRowCount(12);
	unitEffecTable->setHorizontalHeaderLabels(unitTypesLabels);
	unitEffecTable->setVerticalHeaderLabels(unitTypesLabels);

	// Tab Combat Simulator
	for (auto unitType : BWAPI::UnitTypes::allUnitTypes()) {
		if (unitType.getID() > 163) break; // no interesting units farther this point
		if ((unitType.canAttack() || unitType.isSpellcaster() || unitType.spaceProvided() > 0) && !unitType.isHero()) {
			comboDPF1->addItem(unitType.c_str(), unitType.getID());
			comboDPF2->addItem(unitType.c_str(), unitType.getID());
		}
	}
	comboDPF1->model()->sort(0);
	comboDPF2->model()->sort(0);
	for (auto unitType : BWAPI::UnitTypes::allUnitTypes()) {
		comboTestUnits->addItem(unitType.c_str(), unitType.getID());
	}
	comboTestUnits->model()->sort(0);
	testUnitSelected(comboTestUnits->currentIndex());
	
	// Initialize with tab selected
	gameStateTabChanged(gameStateTab->currentIndex());


	// -------------------- SIGNLAS/SLOTS ------------------------
	// Tab Options
	connect(speedSlider, &QSlider::valueChanged, this, &myQtApp::changeSpeed);
	connect(pauseButton, &QPushButton::clicked, this, &myQtApp::pauseGame);
	connect(resumeButton, &QPushButton::clicked, this, &myQtApp::resumeGame);
	connect(chkBuildMap, &QCheckBox::stateChanged, this, &myQtApp::changeBuildMap);
	connect(chkGroundDPS, &QCheckBox::stateChanged, this, &myQtApp::changeGroundDPS);
	connect(chkAirDPS, &QCheckBox::stateChanged, this, &myQtApp::changeAirDPS);
	connect(chkBuildOrder, &QCheckBox::stateChanged, this, &myQtApp::changeBuildOrder);
	// Tab Game State
	connect(chkRegionIdMap, &QCheckBox::stateChanged, this, &myQtApp::changeRegionId);
	connect(informationManager->_GUIsignal, &Qsignal::mapInfoChanged, this, &myQtApp::changeBWTAdata);
	connect(informationManager->_GUIsignal, &Qsignal::gameStateChanged, this, &myQtApp::updateGameStateTable);
	connect(gameStateTab, &QTabWidget::currentChanged, this, &myQtApp::gameStateTabChanged);
	// -- SubTab Game Search
	connect(importStateButton, &QPushButton::clicked, this, &myQtApp::importState);
	connect(loadButton, &QPushButton::clicked, this, &myQtApp::loadFile);
	connect(mctsButton, &QPushButton::clicked, this, &myQtApp::mctsSearch);

	connect(generateActionsButton, &QPushButton::clicked, this, &myQtApp::generateActions);
	connect(nextActionButton, &QPushButton::clicked, this, &myQtApp::expandNode);
	connect(randomActionButton, &QPushButton::clicked, this, &myQtApp::expandRandomNode);
	connect(executeActionButton, &QPushButton::clicked, this, &myQtApp::executeAction);
	connect(rolloutButton, &QPushButton::clicked, this, &myQtApp::rolloutGame);
	// Tab Combat Simulator
	connect(combatParseButton, &QPushButton::clicked, this, &myQtApp::parseCombats);
	connect(combatClearButton, &QPushButton::clicked, this, &myQtApp::clearCombatsParsed);
	connect(crossValidationButton, &QPushButton::clicked, this, &myQtApp::crossValidation);
	connect(learnButton, &QPushButton::clicked, this, &myQtApp::learnCombat);
	connect(getDPFbutton, &QPushButton::clicked, this, &myQtApp::getDPF);
	connect(combatSimulatorButton, &QPushButton::clicked, this, &myQtApp::combatSimulator);
	// -- SubTab Combat Test
// 	connect(comboTestUnits, &QComboBox::currentIndexChanged, this, &myQtApp::testUnitSelected);
	connect(comboTestUnits, SIGNAL(currentIndexChanged(int)), SLOT(testUnitSelected(int)));
	connect(addToCombat1Button, &QPushButton::clicked, this, &myQtApp::addToCombat1);
	connect(addToCombat2Button, &QPushButton::clicked, this, &myQtApp::addToCombat2);
	connect(clearGroup1Button, &QPushButton::clicked, this, &myQtApp::clearGroup1);
	connect(clearGroup2Button, &QPushButton::clicked, this, &myQtApp::clearGroup2);
	connect(testCombatButton, &QPushButton::clicked, this, &myQtApp::testCombat);
}

myQtApp::~myQtApp()
{
	delete mapScene;
}

void myQtApp::changeSpeed(int value)
{
	BWAPI::Broodwar->setLocalSpeed(value);
	speedLabel->setText( "Game speed: " + QString::number(value) );
}

void myQtApp::pauseGame()
{
	BWAPI::Broodwar->pauseGame();
}

void myQtApp::resumeGame()
{
	BWAPI::Broodwar->resumeGame();
}

void myQtApp::changeBWTAdata()
{
// 	textEdit->append("myQtApp::changeBWTAdata() called");
	mapScene->clear();
	// draw regions
	drawPolygons(BWTA::getUnwalkablePolygons(),mapScene);

	double x0, y0, x1, y1;
	QPen linePen(QColor(0, 0, 0));
	linePen.setWidth(2);

	// draw chokepoints borders
	for (auto chokepoint : BWTA::getChokepoints()) {
		const auto positions = chokepoint->getSides();
		x0 = (double)positions.first.x / 8;
		y0 = (double)positions.first.y / 8;
		x1 = (double)positions.second.x / 8;
		y1 = (double)positions.second.y / 8;
		mapScene->addLine(QLineF(x0, y0, x1, y1));
	}

	// draw center of regions
	QColor color = Qt::blue;
	for (auto region : BWTA::getRegions()) {
		x0 = (double)(region->getCenter().x) / 8;
		y0 = (double)(region->getCenter().y) / 8;
		mapScene->addEllipse(QRectF(x0 - 6, y0 - 6, 12, 12), QPen(color), QBrush(color));
		QGraphicsTextItem* text = mapScene->addText(QString::number(informationManager->_regionID[region]));
		text->setPos(x0, y0 + 6);
		text->setDefaultTextColor(color);
	}

	if (spacePartition == "REGIONS_AND_CHOKEPOINTS") {
		color = Qt::red;
		// draw center of chokepoints and lines to regions
		for (auto chokepoint : BWTA::getChokepoints()) {
			x0 = (double)(chokepoint->getCenter().x) / 8;
			y0 = (double)(chokepoint->getCenter().y) / 8;

			// lines
			const auto regions = chokepoint->getRegions();
			x1 = (double)regions.first->getCenter().x / 8;
			y1 = (double)regions.first->getCenter().y / 8;
			mapScene->addLine(QLineF(x0, y0, x1, y1), linePen);
			x1 = (double)regions.second->getCenter().x / 8;
			y1 = (double)regions.second->getCenter().y / 8;
			mapScene->addLine(QLineF(x0, y0, x1, y1), linePen);

			// chokepoint center
			mapScene->addEllipse(QRectF(x0 - 3, y0 - 3, 6, 6), QPen(color), QBrush(color));
			QGraphicsTextItem* text = mapScene->addText(QString::number(informationManager->_chokePointID[chokepoint]));
			text->setPos(x0, y0 + 6);
			text->setDefaultTextColor(color);
		}
	} else {
		// draw lines between regions
		for (auto chokepoint : BWTA::getChokepoints()) {
			const auto regions = chokepoint->getRegions();
			x0 = (double)regions.first->getCenter().x / 8;
			y0 = (double)regions.first->getCenter().y / 8;
			x1 = (double)regions.second->getCenter().x / 8;
			y1 = (double)regions.second->getCenter().y / 8;
			mapScene->addLine(QLineF(x0, y0, x1, y1), linePen);
		}
	}

	//textEdit->append( "Scene size " + QString::number(mapScene->width()) + "," + QString::number(mapScene->height()) );

	// Render map
	mapView->setScene(mapScene);
	mapView->setRenderHint(QPainter::Antialiasing);
	mapView->show();
}

void myQtApp::updateGameStateTable()
{
	if (gameStateTab->currentIndex() == 1) {
		updateUnitMap();
	} else {
		unitGroupVector friendlyUnits = informationManager->gameState._army.friendly;
		int rows = friendlyUnits.size();
		unitsToLocationTable->setRowCount(rows);
		for( int i = 0; i < rows; ++i ) {
			BWAPI::UnitType unitType = BWAPI::UnitType(friendlyUnits[i]->unitTypeId);
			//BWAPI::Order unitCommand = BWAPI::Order(friendlyUnits[i].orderId);
			//std::string unitCommand = BWAPI::Order(friendlyUnits[i].orderId).getName();
			std::string unitCommand = informationManager->gameState.getAbstractOrderName(friendlyUnits[i]->orderId);
			QTableWidgetItem *item0 = new QTableWidgetItem(tr("%1").arg(unitType.getName().c_str()));
			QTableWidgetItem *item1 = new QTableWidgetItem(tr("%1").arg(friendlyUnits[i]->numUnits));
			QTableWidgetItem *item2 = new QTableWidgetItem(tr("%1").arg(friendlyUnits[i]->regionId));
			QTableWidgetItem *item3 = new QTableWidgetItem(tr("%1").arg(unitCommand.c_str()));
			//QTableWidgetItem *item4 = new QTableWidgetItem( tr("%1").arg(informationManager->gameState.getAbstractOrderName(unitCommand).c_str()) );
			QTableWidgetItem *item5 = new QTableWidgetItem(tr("%1").arg(friendlyUnits[i]->targetRegionId));
			int endFrame = friendlyUnits[i]->endFrame;
			if (endFrame > 0) endFrame -= BWAPI::Broodwar->getFrameCount();
			QTableWidgetItem *item6 = new QTableWidgetItem( tr("%1").arg(endFrame) );
			unitsToLocationTable->setItem( i, 0, item0 );
			unitsToLocationTable->setItem( i, 1, item1 );
			unitsToLocationTable->setItem( i, 2, item2 );
			unitsToLocationTable->setItem( i, 3, item3 );
			unitsToLocationTable->setItem( i, 4, item5 );
			unitsToLocationTable->setItem( i, 5, item6 );
	// 		unitsToLocationTable->setItem( i, 4, item4 );
	// 		unitsToLocationTable->setItem( i, 5, item5 );
		}
		unitsToLocationTable->sortByColumn(0, Qt::AscendingOrder);

		//enemy
		unitGroupVector enemyUnits = informationManager->gameState._army.enemy;
		rows = enemyUnits.size();
		unitsToLocationTable_2->setRowCount(rows);
		for( int i = 0; i < rows; ++i ) {
			BWAPI::UnitType unitType = BWAPI::UnitType(enemyUnits[i]->unitTypeId);
			//BWAPI::Order unitCommand = BWAPI::Order(enemyUnits[i].orderId);
			//std::string unitCommand = BWAPI::Order(enemyUnits[i].orderId).getName();
			std::string unitCommand = informationManager->gameState.getAbstractOrderName(enemyUnits[i]->orderId);
			QTableWidgetItem *item0 = new QTableWidgetItem(tr("%1").arg(unitType.getName().c_str()));
			QTableWidgetItem *item1 = new QTableWidgetItem(tr("%1").arg(enemyUnits[i]->numUnits));
			QTableWidgetItem *item2 = new QTableWidgetItem(tr("%1").arg(enemyUnits[i]->regionId));
			QTableWidgetItem *item3 = new QTableWidgetItem(tr("%1").arg(unitCommand.c_str()));
			//QTableWidgetItem *item4 = new QTableWidgetItem( tr("%1").arg(informationManager->gameState.getAbstractOrderName(unitCommand).c_str()) );
			QTableWidgetItem *item5 = new QTableWidgetItem(tr("%1").arg(enemyUnits[i]->targetRegionId));
			int endFrame = enemyUnits[i]->endFrame;
			if (endFrame > 0) endFrame -= BWAPI::Broodwar->getFrameCount();
			QTableWidgetItem *item6 = new QTableWidgetItem( tr("%1").arg(endFrame) );
			unitsToLocationTable_2->setItem( i, 0, item0 );
			unitsToLocationTable_2->setItem( i, 1, item1 );
			unitsToLocationTable_2->setItem( i, 2, item2 );
			unitsToLocationTable_2->setItem( i, 3, item3 );
			unitsToLocationTable_2->setItem( i, 4, item5 );
			unitsToLocationTable_2->setItem( i, 5, item6 );
	// 		unitsToLocationTable_2->setItem( i, 4, item4 );
	// 		unitsToLocationTable_2->setItem( i, 5, item5 );
		}
		unitsToLocationTable_2->sortByColumn(0, Qt::AscendingOrder);
	}
}

void myQtApp::drawPolygons(const std::vector<BWTA::Polygon*> &polygons, QGraphicsScene* scene)
{
	for (const auto &pol : polygons) {
		drawPolygon(*pol, QColor(180, 180, 180), scene);
		for (const auto &h : pol->getHoles()) {
			drawPolygon(*h, QColor(255, 100, 255), scene);
		}
	}
}

void myQtApp::drawPolygon(const BWTA::Polygon &p, QColor qc, QGraphicsScene* scene, double scale)
{
	QVector<QPointF> qp;
	for(int i=0;i<(int)p.size();i++) {
		int j=(i+1)%p.size();
		qp.push_back(QPointF(p[i].x*scale,p[i].y*scale));
	}
	scene->addPolygon(QPolygonF(qp),QPen(QColor(0,0,0)),QBrush(qc));  
}

void myQtApp::drawCustomPolygon(const BWTA::Polygon &p, QGraphicsScene* scene, int regionId)
{
	double scale = 0.125;
	QVector<QPointF> qp;
	for (int i = 0; i < (int)p.size(); i++) {
		int j = (i + 1) % p.size();
		qp.push_back(QPointF(p[i].x*scale, p[i].y*scale));
	}

	CustomPolygonItem* item = new CustomPolygonItem;
	item->setPolygon(QPolygonF(qp));
// 	item->setBrush(QBrush(color));
	item->setPen(QPen(Qt::white));
	item->regionId = regionId;

	scene->addItem(item);
}

void myQtApp::changeDisplayBWTA(int state)
{
	//textEdit->append( "Display BWTA: " + QString::number(state) );
}

void myQtApp::changeBuildMap(int state)
{
	PRINT_BUILD_MAP = !PRINT_BUILD_MAP;
}

void myQtApp::changeGroundDPS(int state)
{
	PRINT_GROUND_DPS = !PRINT_GROUND_DPS;
}

void myQtApp::changeAirDPS(int state)
{
	PRINT_AIR_DPS = !PRINT_AIR_DPS;
}

void myQtApp::changeBuildOrder(int state)
{
	PRINT_BUILD_ORDER = !PRINT_BUILD_ORDER;
}

void myQtApp::changeRegionId(int state)
{
	PRINT_REGION_ID_MAP = !PRINT_REGION_ID_MAP;
}

void myQtApp::gameStateTabChanged(int tabIndex)
{
	switch (tabIndex) {
		case 1: // Units
			updateUnitMap();
			blueInfo->setText( "Friendly units" );
			redInfo->setText( "Enemy units" );
			break;
		case 2: // Effectiveness
			updateEffectivenessMap();
			blueInfo->setText( "Friendly region effectiveness" );
			redInfo->setText( "Enemy region effectiveness" );
			break;
		case 0: // Region ID
		default:
			changeBWTAdata();
			blueInfo->setText( "Center of the region" );
			redInfo->setText( "Center of the chokepoint" );
			break;
	}
}

void myQtApp::updateUnitMap()
{
// 	textEdit->append("myQtApp::updateUnitMap()");
	mapScene->clear();
	// draw regions
	drawPolygons(BWTA::getUnwalkablePolygons(),mapScene);
	// draw chokepoints borders
	double x0, y0, x1, y1;
	for (auto chokepoint : BWTA::getChokepoints()) {
		const auto positions = chokepoint->getSides();
		x0 = (double)positions.first.x / 8;
		y0 = (double)positions.first.y / 8;
		x1 = (double)positions.second.x / 8;
		y1 = (double)positions.second.y / 8;
		mapScene->addLine(QLineF(x0, y0, x1, y1));
	}

	// draw selected region
	for (auto region : BWTA::getRegions()) {
		drawCustomPolygon(region->getPolygon(), mapScene, informationManager->_regionID[region]);
	}

	// Print circles on center regions
	unitGroupVector friendlyUnits = informationManager->gameState._army.friendly;
	for( unsigned int i = 0; i < friendlyUnits.size(); ++i ) {
		BWAPI::Position groupPosition = getCenterRegionId(friendlyUnits[i]->regionId);
		if (groupPosition != BWAPI::Positions::None) {
			drawUnitEllipse((int)groupPosition.x / 8, (int)groupPosition.y / 8, 5, Qt::blue, friendlyUnits[i]->regionId, mapScene);
		}
	}
	unitGroupVector enemyUnits = informationManager->gameState._army.enemy;
	for( unsigned int i = 0; i < enemyUnits.size(); ++i ) {
		BWAPI::Position groupPosition = getCenterRegionId(enemyUnits[i]->regionId);
		if (groupPosition != BWAPI::Positions::None) {
			drawUnitEllipse((int)groupPosition.x / 8, (int)groupPosition.y / 8, -5, Qt::red, enemyUnits[i]->regionId, mapScene);
		}
	}

	// Render map
	mapView->setScene(mapScene);
	mapView->setRenderHint(QPainter::Antialiasing);
	mapView->show();

	updateUnitTable();
}

void myQtApp::updateUnitTable()
{
	int rows = 0;
	int printRow = 0;
	unitGroupVector friendlyUnits = informationManager->gameState._army.friendly;
	for (unsigned int i = 0; i < friendlyUnits.size(); ++i) {
		if (regionIdSelected == friendlyUnits[i]->regionId) {
			rows++;
		}
	}
	unitsToLocationTable_3->setRowCount(rows);
	if (rows > 0) {
		for (unsigned int i = 0; i < friendlyUnits.size(); ++i) {
			if (regionIdSelected == friendlyUnits[i]->regionId) {
				BWAPI::UnitType unitType = BWAPI::UnitType(friendlyUnits[i]->unitTypeId);
				std::string unitCommand = informationManager->gameState.getAbstractOrderName(friendlyUnits[i]->orderId);
				QTableWidgetItem *item0 = new QTableWidgetItem(tr("%1").arg(unitType.getName().c_str()));
				QTableWidgetItem *item1 = new QTableWidgetItem(tr("%1").arg(friendlyUnits[i]->numUnits));
				QTableWidgetItem *item3 = new QTableWidgetItem(tr("%1").arg(unitCommand.c_str()));
				int endFrame = friendlyUnits[i]->endFrame;
				if (endFrame > 0) endFrame -= BWAPI::Broodwar->getFrameCount();
				QTableWidgetItem *item6 = new QTableWidgetItem(tr("%1").arg(endFrame));
				unitsToLocationTable_3->setItem(printRow, 0, item0);
				unitsToLocationTable_3->setItem(printRow, 1, item1);
				unitsToLocationTable_3->setItem(printRow, 2, item3);
				unitsToLocationTable_3->setItem(printRow, 3, item6);
				printRow++;
			}
		}
		unitsToLocationTable_3->sortByColumn(0, Qt::AscendingOrder);
	}

	rows = 0;
	printRow = 0;
	unitGroupVector enemyUnits = informationManager->gameState._army.enemy;
	for (unsigned int i = 0; i < enemyUnits.size(); ++i) {
		if (regionIdSelected == enemyUnits[i]->regionId) {
			rows++;
		}
	}
	unitsToLocationTable_4->setRowCount(rows);
	if (rows > 0) {
		for (unsigned int i = 0; i < enemyUnits.size(); ++i) {
			if (regionIdSelected == enemyUnits[i]->regionId) {
				BWAPI::UnitType unitType = BWAPI::UnitType(enemyUnits[i]->unitTypeId);
				std::string unitCommand = informationManager->gameState.getAbstractOrderName(enemyUnits[i]->orderId);
				QTableWidgetItem *item0 = new QTableWidgetItem(tr("%1").arg(unitType.getName().c_str()));
				QTableWidgetItem *item1 = new QTableWidgetItem(tr("%1").arg(enemyUnits[i]->numUnits));
				QTableWidgetItem *item3 = new QTableWidgetItem(tr("%1").arg(unitCommand.c_str()));
				int endFrame = enemyUnits[i]->endFrame;
				if (endFrame > 0) endFrame -= BWAPI::Broodwar->getFrameCount();
				QTableWidgetItem *item6 = new QTableWidgetItem(tr("%1").arg(endFrame));
				unitsToLocationTable_4->setItem(printRow, 0, item0);
				unitsToLocationTable_4->setItem(printRow, 1, item1);
				unitsToLocationTable_4->setItem(printRow, 2, item3);
				unitsToLocationTable_4->setItem(printRow, 3, item6);
				printRow++;
			}
		}
		unitsToLocationTable_4->sortByColumn(0, Qt::AscendingOrder);
	}
}

void myQtApp::drawUnitEllipse(int x, int y, int xOffset, QColor color, int regionId, QGraphicsScene* scene)
{
	CustomEllipseItem* item = new CustomEllipseItem;
	item->setRect(x+xOffset, y, 10, 10);
	item->setBrush(QBrush(color));
	item->setPen(QPen(color));
	item->regionId = regionId;

	scene->addItem(item);
}

void myQtApp::drawCircleText(int x, int y, int xOffset, QColor color, int score, QGraphicsScene* scene)
{
	CustomEllipseItem* item = new CustomEllipseItem;
	item->setRect(x+xOffset, y, 20, 20);
	item->setBrush(QBrush(Qt::white));
	item->setPen(QPen(color));
	item->regionId = score;
	scene->addItem(item);

	QGraphicsTextItem * io = new QGraphicsTextItem;
	io->setDefaultTextColor(QColor(0,0,255));
	io->setPos(x+xOffset-1,y-1);
	io->setPlainText(QString::number(score));
	scene->addItem(io);
}

// TODO: duplicated from GameState!!!!
BWAPI::Position myQtApp::getCenterRegionId(int regionId)
{
	BWTA::Region* region = informationManager->_regionFromID[regionId];
	if (region != NULL) {
		return region->getCenter();
	} else {
		BWTA::Chokepoint* cp = informationManager->_chokePointFromID[regionId];
		if (cp != NULL) {
			return cp->getCenter();
		} else {
			return BWAPI::Positions::None;
		}
	}
}

void myQtApp::updateEffectivenessMap()
{
// 	textEdit->append("myQtApp::updateEffectivenessMap()");
	mapScene->clear();
	// draw regions
	drawPolygons(BWTA::getUnwalkablePolygons(),mapScene);
	// draw center of choke points/regions
	double x0, y0, x1, y1;
	QPen qp(QColor(0,0,0));
	qp.setWidth(2);
	const std::set<BWTA::Chokepoint*> chokePoints = BWTA::getChokepoints();
	for(std::set<BWTA::Chokepoint*>::const_iterator c=chokePoints.begin();c!=chokePoints.end();c++) {
		// draw choke point
		x0 = (double)((*c)->getCenter().x)/8;
		y0 = (double)((*c)->getCenter().y)/8;
		drawCircleText(x0, y0, 10, Qt::blue, 12, mapScene);
		drawCircleText(x0, y0, -10, Qt::red, 12, mapScene);

		const std::pair<BWTA::Region*,BWTA::Region*> regions = (*c)->getRegions();
		// draw region 1
		x1 = (double)regions.first->getCenter().x/8;
		y1 = (double)regions.first->getCenter().y/8;
		drawCircleText(x1, y1, 10, Qt::blue, 13, mapScene);
		drawCircleText(x1, y1, -10, Qt::red, 10, mapScene);

		// draw region 2
		x1 = (double)regions.second->getCenter().x/8;
		y1 = (double)regions.second->getCenter().y/8;
		drawCircleText(x1, y1, 10, Qt::blue, 10, mapScene);
		drawCircleText(x1, y1, -10, Qt::red, 15, mapScene);
	}

	// Render map
	mapView->setScene(mapScene);
	mapView->setRenderHint(QPainter::Antialiasing);
	mapView->show();
}

// Game Search
// *********************

void myQtApp::importState()
{
	// TODO import state from StarCraft
	testActions = ActionGenerator(&informationManager->gameState);
	//actionGeneration.cleanActions();
	searchLog->append(QString::fromStdString(testActions._gs->toString()));
	searchLog->append("By default generating actions for friendly");
	searchLog->append(QString::fromStdString(testActions.toString()));
}

void myQtApp::generateActions()
{
	if (informationManager->gameState.canExecuteAnyAction(true)) {
		searchLog->append("Generating actions for friendly");
		testActions = ActionGenerator(&informationManager->gameState, true);
	} else {
		searchLog->append("Generating actions for enemy");
		testActions = ActionGenerator(&informationManager->gameState, false);
	}
	searchLog->append(QString::fromStdString(testActions.toString()));
}

void myQtApp::expandNode()
{
	playerActions_t unitsAction = testActions.getNextAction();
	if (!unitsAction.empty()) {
		searchLog->append(QString::fromStdString(testActions.toString(unitsAction)));
	} else {
		searchLog->append("No more actions");
	}
}

void myQtApp::expandRandomNode()
{
// 	playerActions_t unitsAction = testActions.getRandomAction();
// 	playerActions_t unitsAction = testActions.getUniqueRandomAction();
	playerActions_t unitsAction = testActions.getBiasAction();
// 	playerActions_t unitsAction = testActions.getMostProbAction();
	
	if (!unitsAction.empty()) {
		searchLog->append(QString::fromStdString(testActions.toString(unitsAction)));
	} else {
		searchLog->append("No more actions");
	}
}


void myQtApp::executeAction()
{
	searchLog->append("Execute actions:");
	informationManager->gameState.execute(testActions._lastAction, testActions._player);
	searchLog->append(QString::fromStdString(informationManager->gameState.toString()));

	searchLog->append("Forward time:");
	informationManager->gameState.moveForward();
	searchLog->append(QString::fromStdString(informationManager->gameState.toString()));
}

void myQtApp::loadFile()
{
	searchLog->clear();
    std::string gameStateLog = "bwapi-data\\logs\\gameState.txt";
    searchLog->append( QString("Loading file: %1").arg(gameStateLog.c_str()) );
//     LOG( "Loading file: " << gameStateLog );

    // clear lists
	informationManager->gameState.cleanArmyData();
	informationManager->gameState._time = 0;

    std::ifstream infile(gameStateLog.c_str());
    std::string line;

	// move to the selected case to import
	std::getline(infile, line); // get first line
	std::istringstream stream(line);
	int numLine;
	std::string dummy;
	stream >> numLine >> dummy;
	searchLog->append(QString("Getting game state from line: %1").arg(numLine));
	for (int i = 2; i < numLine; ++i) std::getline(infile, line);

    int listID = 1; // 1=friendList 2=enemyList
    while (std::getline(infile, line)) {
        //searchLog->append(line.c_str());
        //LOG(line.c_str());
        std::istringstream iss(line);
        int unitTypeID, numUnits, regionID, targetRegionID, endFrame;
		float HP;
        std::string orderString, extraInfo;
		if (!(iss >> unitTypeID >> numUnits >> regionID >> orderString >> targetRegionID >> endFrame >> HP >> extraInfo)) {
            // Error processing line
			if (line.compare(0, 1, "(") == 0)  listID = 2;
			if (line.compare(0, 1, ";") == 0)  break; // end of game state
			if (line.compare(0, 1, "G") == 0) {
				// extract current time
				int time;
				std::istringstream iss(line);
				iss >> extraInfo >> extraInfo >> time >> extraInfo;
				informationManager->gameState._time = time;
			}
        } else {
// 			searchLog->append(QString("Extracted: %1 %2 %3 %4 %5 %6 %7").arg(unitTypeID).arg(numUnits).arg(regionID).arg(orderString.c_str()).arg(targetRegionID).arg(endFrame).arg(HP));
			// get abstract order
			abstractOrder::order order = abstractOrder::getOrder(orderString);
			endFrame += informationManager->gameState._time;
			informationManager->gameState.addGroup(unitTypeID, numUnits, regionID, listID, HP, order, targetRegionID, endFrame);
			if (order == abstractOrder::Attack) {
				// keep track of the regions with units in attack state
				informationManager->gameState._regionsInCombat.insert(regionID);
			}
        }
    }
	infile.close();

	searchLog->append("Game state imported:");
	// now that we have all the units in the game state, compute expected end frame
	//informationManager->gameState.calculateExpectedEndFrameForAllGroups();
	// and forward until next point decision
	//informationManager->gameState.moveForward(); // we can move Forward ...
	//informationManager->gameState.resetFriendlyActions(); // ...or set our orders to 0 to find the best immediate action
	searchLog->append(QString::fromStdString(informationManager->gameState.toString()));

	EvaluationFunctionBasic ef;
	double reward = ef.evaluate(informationManager->gameState);
	searchLog->append(QString("Reward %1").arg(reward));
}

void myQtApp::rolloutGame()
{
	// get ready for the action
	informationManager->gameState.moveForward();

	__int8 nextPlayerInSimultaneousNode = 1;
	int depth = 0;
	ActionGenerator moveGenerator;
	int nextPlayer = -1;
	while (!informationManager->gameState.gameover() && informationManager->gameState._time < 2880 && depth < 1000) {
		// look next player to move
		nextPlayer = -1;
		if (informationManager->gameState.canExecuteAnyAction(true)) {
			if (informationManager->gameState.canExecuteAnyAction(false)) {
				// if both can move: alternate
				nextPlayer = (int)nextPlayerInSimultaneousNode;
				nextPlayerInSimultaneousNode = 1 - nextPlayerInSimultaneousNode;
			} else {
				nextPlayer = 1;
			}
		} else {
			if (informationManager->gameState.canExecuteAnyAction(false)) nextPlayer = 0;
		}
		if (nextPlayer == 1 || nextPlayer == 0) {
			moveGenerator = ActionGenerator(&informationManager->gameState, nextPlayer);
		} else {
			printError(searchLog, QString("Both players can't move!!!"));
			break;
		}

		// chose random action
		playerActions_t unitsAction = moveGenerator.getRandomAction();
		searchLog->append(QString::fromStdString(moveGenerator.toString(unitsAction)));

		// execute action
		informationManager->gameState.execute(unitsAction, moveGenerator._player);
		searchLog->append("After execute");
		searchLog->append(QString::fromStdString(informationManager->gameState.toString()));
		informationManager->gameState.moveForward();
		searchLog->append("After move forward");
		searchLog->append(QString::fromStdString(informationManager->gameState.toString()));
		depth++;
	}

	searchLog->append(QString("Game state simulated after 2880 frames and depth %1:").arg(depth));
	informationManager->gameState.calculateExpectedEndFrameForAllGroups();
	searchLog->append(QString::fromStdString(informationManager->gameState.toString()));

	EvaluationFunctionBasic ef;
	double reward = ef.evaluate(informationManager->gameState);
	searchLog->append(QString("Reward %1").arg(reward));
}

void myQtApp::mctsSearch()
{
	// mark siege as researched
	informationManager->gameState.friendlySiegeTankResearched = true;
	informationManager->gameState.enemySiegeTankResearched = true;

	int depth = LoadConfigInt("MCTSCD", "depth", 1);
	int iterations = LoadConfigInt("MCTSCD", "iterations");
	int maxSimTime = LoadConfigInt("MCTSCD", "max_simulation_time");
	EvaluationFunctionBasic ef;
	MCTSCD searchAlg = MCTSCD(depth, &ef, iterations, maxSimTime);
	playerActions_t bestActions = searchAlg.start(informationManager->gameState);

	// Print best actions

	for (const auto& groupAction : bestActions) {
		std::string tmp;
#ifdef DEBUG_ORDERS
		tmp = BWAPI::UnitType(groupAction.unitTypeId).getName() + " at " + intToString(groupAction.regionId)
			+ " " + abstractOrder::name[groupAction.action.orderID] + " to " + intToString(groupAction.action.targetRegion);
#else
		unitGroup_t* unitGroup = informationManager->gameState._army.friendly[groupAction.pos];
		tmp = BWAPI::UnitType(unitGroup->unitTypeId).getName() + " at " + intToString(unitGroup->regionId)
			+ " " + abstractOrder::name[groupAction.action.orderID] + " to " + intToString(groupAction.action.targetRegion);
#endif
		searchLog->append(QString::fromStdString(tmp));
	}

}

void myQtApp::combatSimulator()
{
	// Experiment options
	const bool SKIP_TRANSPORTS			= checkSkipTransports->isChecked();
	const bool SKIP_PURE_HOMOGENEOUS	= checkSkipPureHomo->isChecked();
	const bool SKIP_HOMOGENEOUS			= checkSkipHomo->isChecked();
	const bool SKIP_PARTIAL_HOMOGENEOUS = checkSkipPartialHomo->isChecked();
	const bool SKIP_HETEROGENEOUS		= checkSkipHete->isChecked();
	const bool SKIP_SPARCRAFT_NOT_SUPPORTED = checkSkipSparcraft->isChecked();

	const bool SIMULATE_SPARCAFT = checkRunSparcraft->isChecked();
	const int SPARCRAFT_MOVE_LIMIT = moveLimitEdit->text().toInt();
	const bool IMPORT_FULL_HP_SPARCAFT = checkFullHpSparcraft->isChecked();

	const bool COMPARE_SIM = checkCompareSimulator->isChecked();
	int index1 = compareSim1->value();
	int index2 = compareSim2->value();
	const bool COMPARE_LTD = checkCompareLTD->isChecked();

	const int SIMULATIONS_EXPERIMENTS = 18;
#ifndef _DEBUG  // too many experiments on debug mode produce a stack overflow
	const int SPARCRAFT_EXPERIMENTS = 6;
#else
	const int SPARCRAFT_EXPERIMENTS = 1;
#endif
	const int NUMBER_OF_TESTS = SIMULATIONS_EXPERIMENTS + SPARCRAFT_EXPERIMENTS;

	// constructors needed for SparCraft
	SparCraft::init();
	SparCraft::PlayerPtr closest1(new SparCraft::Player_AttackClosest(SparCraft::Players::Player_One));
	SparCraft::PlayerPtr closest2(new SparCraft::Player_AttackClosest(SparCraft::Players::Player_Two));
#ifndef _DEBUG
	SparCraft::PlayerPtr dps1(new SparCraft::Player_AttackDPS(SparCraft::Players::Player_One));
	SparCraft::PlayerPtr dps2(new SparCraft::Player_AttackDPS(SparCraft::Players::Player_Two));
	SparCraft::PlayerPtr weakest1(new SparCraft::Player_AttackWeakest(SparCraft::Players::Player_One));
	SparCraft::PlayerPtr weakest2(new SparCraft::Player_AttackWeakest(SparCraft::Players::Player_Two));
	SparCraft::PlayerPtr nokdps1(new SparCraft::Player_NOKDPS(SparCraft::Players::Player_One));
	SparCraft::PlayerPtr nokdps2(new SparCraft::Player_NOKDPS(SparCraft::Players::Player_Two));
	SparCraft::PlayerPtr kiter1(new SparCraft::Player_Kiter(SparCraft::Players::Player_One));
	SparCraft::PlayerPtr kiter2(new SparCraft::Player_Kiter(SparCraft::Players::Player_Two));
	SparCraft::PlayerPtr kiterdps1(new SparCraft::Player_KiterDPS(SparCraft::Players::Player_One));
	SparCraft::PlayerPtr kiterdps2(new SparCraft::Player_KiterDPS(SparCraft::Players::Player_Two));
#endif // !_DEBUG

	Timer timers[NUMBER_OF_TESTS];
	Statistic stateSimilarity[NUMBER_OF_TESTS];
	Statistic winnerAccur[NUMBER_OF_TESTS];
	std::map<unsigned int, Statistic> groupWin[NUMBER_OF_TESTS];
	// build numbers of k for LTD
	const int NUMBER_OF_K = 20;
	std::vector<float> LTDk;
	float step = 2.0f / float(NUMBER_OF_K);
	for (float i(0.1f); i < 2.0f; i += step) LTDk.push_back(std::trunc(i * 100) / 100); // comparing float number is "tricky", I trunc the number for a better comparison 
	LTDk.push_back(2.0f);
	std::map<float, size_t> LTDkToId;
	size_t kIndex(0);
	for (const auto& val : LTDk) {
		LTDkToId[val] = kIndex;
		++kIndex;
	}
	// Define LTD tests
	Statistic winAccurLTDStatic[NUMBER_OF_K];
	std::map<unsigned int, Statistic> groupWinLTDStatic[NUMBER_OF_K];
	Statistic winAccurLTDLearn[NUMBER_OF_K];
	std::map<unsigned int, Statistic> groupWinLTDLearn[NUMBER_OF_K];

	Statistic statsCombatLength, statsCombatGroups, statsCombatUnits;

	// counters
	int combatSkippedMine = 0;
	int combatSkippedTransport = 0;
	int combatSkippedPerfectHomogeneous = 0;
	int combatSkippedHomogeneous = 0;
	int combatSkippedPartialHomogeneous = 0;
	int combatSkippedHeterogeneous = 0;
	int combatSkippedSparCraft = 0;
	int playerNotKilled = 0;
	int wasBetter = 0;
	int wasWorst = 0;
	
	bool hasMines = false;
	bool hasTransports = false;
	bool skipSparCraft = false;

	combatSimBar->setMaximum(learner.allCombats.size());
	combatSimBar->setValue(0);

	std::mt19937 g(123456); // fixed seed for experimentation purpose

	for (size_t k = 0; k < learner.allCombats.size(); ++k) {
		combatInfo combat(learner.allCombats[k]);

		combatSimBar->setValue(combatSimBar->value() + 1);
		std::vector<SparCraft::Game> sparCraftGames;
		GameState simulatorsSparCraft[SPARCRAFT_EXPERIMENTS];

		// **************************************************************
		// import initState
		// **************************************************************
		GameState initalState(new CombatSimSustained(&unitStatic->DPF));
		SparCraft::GameState sparCraftInitialState;

		// set flags to false
		hasMines = false;
		hasTransports = false;
		skipSparCraft = false;

		// Add units from data replay to game state
		addUnitsFromReplaytoGameState(combat.armyUnits1, initalState, sparCraftInitialState, 1,
			hasMines, hasTransports, skipSparCraft,
			SKIP_TRANSPORTS, SKIP_SPARCRAFT_NOT_SUPPORTED, IMPORT_FULL_HP_SPARCAFT);

		addUnitsFromReplaytoGameState(combat.armyUnits2, initalState, sparCraftInitialState, 2,
			hasMines, hasTransports, skipSparCraft,
			SKIP_TRANSPORTS, SKIP_SPARCRAFT_NOT_SUPPORTED, IMPORT_FULL_HP_SPARCAFT);
		
		// skip some combats
		if (hasMines) {
			combatSkippedMine++;
			continue;
		}
		if (SKIP_TRANSPORTS && hasTransports) {
			combatSkippedTransport++;
			continue;
		}
		if (SKIP_SPARCRAFT_NOT_SUPPORTED && skipSparCraft) {
			combatSkippedSparCraft++;
			continue;
		}
		
		std::string combatType = "1vN";
		if ((initalState._army.friendly.size() == 1 && initalState._army.enemy.size() == 1)) {
			combatType = "1v1";
			if (SKIP_PURE_HOMOGENEOUS && (initalState._army.friendly[0]->unitTypeId == initalState._army.enemy[0]->unitTypeId)) {
				combatSkippedPerfectHomogeneous++;
				continue;
			} else if (SKIP_HOMOGENEOUS && (initalState._army.friendly[0]->unitTypeId != initalState._army.enemy[0]->unitTypeId)) {
				combatSkippedHomogeneous++;
				continue;
			}
		}
		if (SKIP_PARTIAL_HOMOGENEOUS && 
			((initalState._army.friendly.size() == 1 && initalState._army.enemy.size() > 1) ||
			(initalState._army.friendly.size() > 1 && initalState._army.enemy.size() == 1))) {
			combatSkippedPartialHomogeneous++;
			continue;
		}
		if (initalState._army.friendly.size() > 1 && initalState._army.enemy.size() > 1) {
			combatType = "NvN";
			if (SKIP_HETEROGENEOUS) {
				combatSkippedHeterogeneous++;
				continue;
			}
		}

// 		LOG(initalState.toString());

		// to have a "random" target selection, we shuffle the vectors
		std::shuffle(initalState._army.friendly.begin(), initalState._army.friendly.end(), g);
		std::shuffle(initalState._army.enemy.begin(), initalState._army.enemy.end(), g);

		addCombatStats(statsCombatLength, statsCombatGroups, statsCombatUnits, combat, initalState);
		initalState.calculateExpectedEndFrameForAllGroups();

		// **************************************************************
		// import finalState
		// **************************************************************
		GameState finalState;
		for (auto army : combat.armyUnitsEnd1) {
			finalState.addUnit(army.second.typeID, 8, 1, abstractOrder::Attack, army.second.HP + army.second.shield);
		}
		for (auto army : combat.armyUnitsEnd2) {
			finalState.addUnit(army.second.typeID, 8, 2, abstractOrder::Attack, army.second.HP + army.second.shield);
		}

		// **************************************************************
		// create a game state for each simulator
		// **************************************************************
		GameState simulators[SIMULATIONS_EXPERIMENTS];
		for (auto& simulator : simulators) simulator = initalState; // set the initial state to all simulators
		simulators[12].changeCombatSimulator(new CombatSimLanchester(&unitStatic->DPF));
		simulators[13].changeCombatSimulator(new CombatSimLanchester(&learner.unitDPF));
		simulators[0].changeCombatSimulator(new CombatSimSustained(&unitStatic->DPF));
		simulators[9].changeCombatSimulator(new CombatSimSustained(&learner.unitDPF));
		simulators[1].changeCombatSimulator(new CombatSimDecreased(&unitStatic->typeDPF, &unitStatic->DPF));
		simulators[2].changeCombatSimulator(new CombatSimDecreased(&learner.unitTypeDPF, &unitStatic->DPF));
		// Sorted by Score
		comp_f targetPolicy = sortByScore;
		simulators[14].changeCombatSimulator(new CombatSimLanchester(&unitStatic->DPF, targetPolicy, targetPolicy));
		simulators[15].changeCombatSimulator(new CombatSimLanchester(&learner.unitDPF, targetPolicy, targetPolicy));
		simulators[3].changeCombatSimulator(new CombatSimSustained(&unitStatic->DPF, targetPolicy, targetPolicy));
		simulators[11].changeCombatSimulator(new CombatSimSustained(&learner.unitDPF, targetPolicy, targetPolicy));
		simulators[4].changeCombatSimulator(new CombatSimDecreased(&unitStatic->typeDPF, &unitStatic->DPF, targetPolicy, targetPolicy));
		simulators[5].changeCombatSimulator(new CombatSimDecreased(&learner.unitTypeDPF, &unitStatic->DPF, targetPolicy, targetPolicy));
		// Sorted by type priority learned
		std::vector<float> priority1;
		UnitInfoLearner::GroupDiversity groupDiversity1 = learner.getGroupDiversity(combat.armySize1);
		switch (groupDiversity1) {
		case UnitInfoLearner::AIR:	  priority1 = learner.typePriorityAir; break;
		case UnitInfoLearner::GROUND: priority1 = learner.typePriorityGround; break;
		case UnitInfoLearner::BOTH:	  priority1 = learner.typePriorityBoth; break;
		}
		sortByTypeClass sortByType1(&priority1);

		std::vector<float> priority2;
		UnitInfoLearner::GroupDiversity groupDiversity2 = learner.getGroupDiversity(combat.armySize2);
		switch (groupDiversity2) {
		case UnitInfoLearner::AIR:	  priority2 = learner.typePriorityAir; break;
		case UnitInfoLearner::GROUND: priority2 = learner.typePriorityGround; break;
		case UnitInfoLearner::BOTH:	  priority2 = learner.typePriorityBoth; break;
		}
		sortByTypeClass sortByType2(&priority2);
		
		simulators[16].changeCombatSimulator(new CombatSimLanchester(&unitStatic->DPF, sortByType2, sortByType1));
		simulators[17].changeCombatSimulator(new CombatSimLanchester(&learner.unitDPF, sortByType2, sortByType1));
		simulators[6].changeCombatSimulator(new CombatSimSustained(&unitStatic->DPF, sortByType2, sortByType1));
		simulators[10].changeCombatSimulator(new CombatSimSustained(&learner.unitDPF, sortByType2, sortByType1));
		simulators[7].changeCombatSimulator(new CombatSimDecreased(&unitStatic->typeDPF, &unitStatic->DPF, sortByType2, sortByType1));
		simulators[8].changeCombatSimulator(new CombatSimDecreased(&learner.unitTypeDPF, &unitStatic->DPF, sortByType2, sortByType1));

		// **************************************************************
		// simulate all states with different simulators
		// **************************************************************

		// to avoid cache issues with timers we run once for each data Type
		GameState simulatorCache(initalState);
		GameState simulatorCache2(initalState);
		simulatorCache2.changeCombatSimulator(new CombatSimDecreased(&unitStatic->typeDPF, &unitStatic->DPF));
		simulatorCache.moveForward();
		simulatorCache2.moveForward();

		for (int i = 0; i < SIMULATIONS_EXPERIMENTS; ++i) {
			timers[i].start();
			simulators[i].moveForward();
			timers[i].stop();
		}

		if (SIMULATE_SPARCAFT) {
			sparCraftGames.push_back(SparCraft::Game(sparCraftInitialState, closest1, closest2, SPARCRAFT_MOVE_LIMIT));
#ifndef _DEBUG
			sparCraftGames.push_back(SparCraft::Game(sparCraftInitialState, dps1, dps2, SPARCRAFT_MOVE_LIMIT));
			sparCraftGames.push_back(SparCraft::Game(sparCraftInitialState, weakest1, weakest2, SPARCRAFT_MOVE_LIMIT));
			sparCraftGames.push_back(SparCraft::Game(sparCraftInitialState, nokdps1, nokdps2, SPARCRAFT_MOVE_LIMIT));
			sparCraftGames.push_back(SparCraft::Game(sparCraftInitialState, kiter1, kiter2, SPARCRAFT_MOVE_LIMIT));
			sparCraftGames.push_back(SparCraft::Game(sparCraftInitialState, kiterdps1, kiterdps2, SPARCRAFT_MOVE_LIMIT));
#endif

			if (!skipSparCraft) {
				for (int i = 0; i < SPARCRAFT_EXPERIMENTS; ++i) {
					timers[SIMULATIONS_EXPERIMENTS + i].start();
					sparCraftGames[i].play();
					timers[SIMULATIONS_EXPERIMENTS + i].stop();

					if (sparCraftGames[i].getState().numUnits(SparCraft::Players::Player_One) > 0 &&
						sparCraftGames[i].getState().numUnits(SparCraft::Players::Player_Two) > 0) {
						playerNotKilled++; // both players still alive
					}
				}
			}
		}

		// **************************************************************
		// compare states against finalState
		// **************************************************************
		int groupSize = initalState._army.friendly.size() + initalState._army.enemy.size();
		// LTDk winner prediction
		for (size_t index(0); index < LTDk.size(); ++index) {
			// Static DPF
			float score = EvalCombat::LTD(unitStatic->DPF, combat, LTDk[index]);
			if ((score > 0 && finalState.winner() == 0) || (score < 0 && finalState.winner() == 1) ||
				(score == 0 && finalState.winner() == -1)) {
				winAccurLTDStatic[index].add(1);
				groupWinLTDStatic[index][groupSize].add(1);
			} else {
				winAccurLTDStatic[index].add(0);
				groupWinLTDStatic[index][groupSize].add(0);
			}
			// Learned DPF
			score = EvalCombat::LTD(learner.unitDPF, combat, LTDk[index]);
			if ((score > 0 && finalState.winner() == 0) || (score < 0 && finalState.winner() == 1) ||
				(score == 0 && finalState.winner() == -1)) {
				winAccurLTDLearn[index].add(1);
				groupWinLTDLearn[index][groupSize].add(1);
			} else {
				winAccurLTDLearn[index].add(0);
				groupWinLTDLearn[index][groupSize].add(0);
			}
		}

		if (COMPARE_LTD) {
			// testing LTD2 vs LTD1
			float scoreLTD2 = EvalCombat::LTD(unitStatic->DPF, combat, 1);
			float scoreLTD1 = EvalCombat::LTD(learner.unitDPF, combat, 1);
			float scoreLTD = EvalCombat::LTD(learner.unitDPF, combat, 0.5);
//			if ((scoreLTD1 < 0 && scoreLTD2 >= 0) || (scoreLTD1 > 0 && scoreLTD2 <= 0)) {
				combatLog2->append(QString("LTD1 learn: %1 LTD1 static: %2 (LTD0.5: %3) Final state winner: %4").arg(scoreLTD1).arg(scoreLTD2).arg(scoreLTD).arg(finalState.winner()));
				combatLog2->append(QString::fromStdString(learner.toString(combat)));
				combatLog2->append("\n\n");
//			}
			// print if LTD0.5 is better than LTD1
			if (((scoreLTD > 0 && finalState.winner() == 0) ||
				(scoreLTD < 0 && finalState.winner() == 1)) &&
				((scoreLTD1 < 0 && scoreLTD > 0) || (scoreLTD1 > 0 && scoreLTD < 0))) {
				combatLog2->append(QString("LTD1: %1 LTD2: %2 (LTD0.5: %3) LTD0.5 is better!!").arg(scoreLTD1).arg(scoreLTD2).arg(scoreLTD));
				combatLog2->append(QString::fromStdString(learner.toString(combat)));
				combatLog2->append("\n\n");
			}
		}

		
		// jaccard index and winner prediction
		for (int i = 0; i < SIMULATIONS_EXPERIMENTS; ++i) {
			stateSimilarity[i].add(finalState.getPredictionAccuracy(initalState, simulators[i]));
			winnerAccur[i].add(finalState.winner() == simulators[i].winner());
			groupWin[i][groupSize].add(finalState.winner() == simulators[i].winner());
		}
		// logs to calculate the p-value
// 		LOG(finalState.getPredictionAccuracy(initalState, simulators[0]) << "," <<
// 			finalState.getPredictionAccuracy(initalState, simulators[1]) << "," <<
// 			finalState.getPredictionAccuracy(initalState, simulators[2]) << "," <<
// 			finalState.getPredictionAccuracy(initalState, simulators[3]) << "," <<
// 			finalState.getPredictionAccuracy(initalState, simulators[4]) << "," <<
// 			finalState.getPredictionAccuracy(initalState, simulators[5]) << "," <<
// 			finalState.getPredictionAccuracy(initalState, simulators[6]) << "," <<
// 			finalState.getPredictionAccuracy(initalState, simulators[7]) << "," <<
// 			finalState.getPredictionAccuracy(initalState, simulators[8]) << "," <<
// 			finalState.getPredictionAccuracy(initalState, simulators[9]) << "," <<
// 			finalState.getPredictionAccuracy(initalState, simulators[10]) << "," <<
// 			finalState.getPredictionAccuracy(initalState, simulators[11]) << "," <<
// 			finalState.getPredictionAccuracy(initalState, simulators[12]) << "," <<
// 			finalState.getPredictionAccuracy(initalState, simulators[13]) << "," <<
// 			finalState.getPredictionAccuracy(initalState, simulators[14]) << "," <<
// 			finalState.getPredictionAccuracy(initalState, simulators[15]) << "," <<
// 			finalState.getPredictionAccuracy(initalState, simulators[16]) << "," <<
// 			finalState.getPredictionAccuracy(initalState, simulators[17]));
		// raw data for plots
// 		LOG(finalState.getPredictionAccuracy(initalState, simulators[12]) << ",Lanchester,Static,Random," << combatType);
// 		LOG(finalState.getPredictionAccuracy(initalState, simulators[13]) << ",Lanchester,Learn,Random," << combatType);
// 		LOG(finalState.getPredictionAccuracy(initalState, simulators[0]) << ",Sustained,Static,Random," << combatType);
// 		LOG(finalState.getPredictionAccuracy(initalState, simulators[9]) << ",Sustained,Learn,Random," << combatType);
// 		LOG(finalState.getPredictionAccuracy(initalState, simulators[1]) << ",Decreasing,Static,Random," << combatType);
// 		LOG(finalState.getPredictionAccuracy(initalState, simulators[2]) << ",Decreasing,Learn,Random," << combatType);
// 		LOG(finalState.getPredictionAccuracy(initalState, simulators[14]) << ",Lanchester,Static,Score," << combatType);
// 		LOG(finalState.getPredictionAccuracy(initalState, simulators[15]) << ",Lanchester,Learn,Score," << combatType);
// 		LOG(finalState.getPredictionAccuracy(initalState, simulators[3]) << ",Sustained,Static,Score," << combatType);
// 		LOG(finalState.getPredictionAccuracy(initalState, simulators[11]) << ",Sustained,Learn,Score," << combatType);
// 		LOG(finalState.getPredictionAccuracy(initalState, simulators[4]) << ",Decreasing,Static,Score," << combatType);
// 		LOG(finalState.getPredictionAccuracy(initalState, simulators[5]) << ",Decreasing,Learn,Score," << combatType);
// 		LOG(finalState.getPredictionAccuracy(initalState, simulators[16]) << ",Lanchester,Static,BC," << combatType);
// 		LOG(finalState.getPredictionAccuracy(initalState, simulators[17]) << ",Lanchester,Learn,BC," << combatType);
// 		LOG(finalState.getPredictionAccuracy(initalState, simulators[6]) << ",Sustained,Static,BC," << combatType);
// 		LOG(finalState.getPredictionAccuracy(initalState, simulators[10]) << ",Sustained,Learn,BC," << combatType);
// 		LOG(finalState.getPredictionAccuracy(initalState, simulators[7]) << ",Decreasing,Static,BC," << combatType);
// 		LOG(finalState.getPredictionAccuracy(initalState, simulators[8]) << ",Decreasing,Learn,BC," << combatType);

		if (SIMULATE_SPARCAFT) {
			if (!skipSparCraft) {
				for (int i = 0; i < SPARCRAFT_EXPERIMENTS; ++i) {
					simulatorsSparCraft[i] = getGameState(sparCraftGames[i].getState());
					stateSimilarity[SIMULATIONS_EXPERIMENTS + i].add(finalState.getPredictionAccuracy(initalState, simulatorsSparCraft[i]));
					winnerAccur[SIMULATIONS_EXPERIMENTS + i].add(finalState.winner() == simulatorsSparCraft[i].winner());
					int groupSize = initalState._army.friendly.size() + initalState._army.enemy.size();
					groupWin[SIMULATIONS_EXPERIMENTS + i][groupSize].add(finalState.winner() == simulatorsSparCraft[i].winner());
				}
			} else {
				skipSparCraft = false;
				combatSkippedSparCraft++;
			}
		}

		// **************************************************************
		// compare 2 simulators
		// **************************************************************
		if (COMPARE_SIM) {
			float jaccard1 = finalState.getPredictionAccuracy(initalState, simulators[index1]);
			float jaccard2 = finalState.getPredictionAccuracy(initalState, simulators[index2]);
// 			if (simulators[index1].winner() != simulators[index2].winner()) {
			if (jaccard1 > jaccard2) {
				++wasBetter;
				combatLog2->append(QString("Combat index: %1").arg(k));
				combatLog2->append("Initial State");
				combatLog2->append(QString::fromStdString(initalState.toString()));
				combatLog2->append("Final State");
				combatLog2->append(QString::fromStdString((finalState.toString())));
				combatLog2->append(QString("Simulation %1 jaccard (%2)").arg(index1).arg(jaccard1));
				combatLog2->append(QString::fromStdString((simulators[index1].toString())));
				combatLog2->append(QString("Simulation %1 jaccard (%2)").arg(index2).arg(jaccard2));
				combatLog2->append(QString::fromStdString((simulators[index2].toString())));
				combatLog2->append("\n\n");
			} else if (jaccard1 < jaccard2) {
				++wasWorst;
			}
		}
	}

	int allCombatsSize = learner.allCombats.size();
	int analyzedCombats = allCombatsSize - combatSkippedMine - combatSkippedTransport 
		- combatSkippedPerfectHomogeneous - combatSkippedHomogeneous - combatSkippedPartialHomogeneous - combatSkippedHeterogeneous;
	if (SKIP_SPARCRAFT_NOT_SUPPORTED) analyzedCombats -= combatSkippedSparCraft;
	combatLog->append(QString("Analyzed %1 of %2 combats").arg(analyzedCombats).arg(allCombatsSize));
	combatLog->append(QString("Reasons to skip:"));
	combatLog->append(QString("%1 combats with Vulture mines").arg(combatSkippedMine));
	if (SKIP_TRANSPORTS) {
		combatLog->append(QString("%1 combats with Transports").arg(combatSkippedTransport));
	}
	if (SKIP_PURE_HOMOGENEOUS) {
		combatLog->append(QString("%1 combats with Perfect Homogeneous armies (1vs1 same type)").arg(combatSkippedPerfectHomogeneous));
	}
	if (SKIP_HOMOGENEOUS) {
		combatLog->append(QString("%1 combats with Homogeneous armies (1vs1)").arg(combatSkippedHomogeneous));
	}
	if (SKIP_PARTIAL_HOMOGENEOUS) {
		combatLog->append(QString("%1 combats with Partial Homogeneous armies (1vsN)").arg(combatSkippedPartialHomogeneous));
	}
	if (SKIP_HETEROGENEOUS) {
		combatLog->append(QString("%1 combats with Heterogeneous armies (NvsN)").arg(combatSkippedHeterogeneous));
	}
	if (SKIP_SPARCRAFT_NOT_SUPPORTED) {
		combatLog->append(QString("%1 combats not Compatible with SparCraft").arg(combatSkippedSparCraft));
	}
	if (SIMULATE_SPARCAFT) {
		if (!SKIP_SPARCRAFT_NOT_SUPPORTED) {
			combatLog->append(QString("%1 SparCraft simulations").arg(analyzedCombats - combatSkippedSparCraft));
		}
		if (playerNotKilled > 0) {
			printError(combatLog, QString("%1 SparCraft games where both players are still alive").arg(playerNotKilled));
		}
	}
	combatLog->append("\n");

	// combat stats
	combatLog->append(QString("Avg combat length %1, max %2, min %3").arg(statsCombatLength.getMean()).arg(statsCombatLength.getMax()).arg(statsCombatLength.getMin()));
	combatLog->append(QString("Avg combat units %1, max %2, min %3").arg(statsCombatUnits.getMean()).arg(statsCombatUnits.getMax()).arg(statsCombatUnits.getMin()));
	combatLog->append(QString("Avg combat groups(types) %1, max %2, min %3").arg(statsCombatGroups.getMean()).arg(statsCombatGroups.getMax()).arg(statsCombatGroups.getMin()));
	combatLog->append("\n");

	if (COMPARE_SIM) {
		// index1 was better than index2
		combatLog->append(QString("%1 times simulator %2 was better than %3").arg(wasBetter).arg(index1).arg(index2));
		combatLog->append(QString("%1 times simulator %2 was worst than %3").arg(wasWorst).arg(index1).arg(index2));
		combatLog->append("\n\n");
	}

	QString htmlString = "<table><tr><td>Simulator</td><td>Final state pred</td><td>Win-pred accuracy</td><td>Total time</td></tr>";
	htmlString += QString("<tr><td>LTD Static</td><td>-</td><td>%1&plusmn;%2</td><td>-</td></tr>")
		.arg(winAccurLTDStatic[LTDkToId[1]].getMean()).arg(winAccurLTDStatic[LTDkToId[1]].getStdErr());
	htmlString += QString("<tr><td>LTD Learn</td><td>-</td><td>%1&plusmn;%2</td><td>-</td></tr>")
		.arg(winAccurLTDLearn[LTDkToId[1]].getMean()).arg(winAccurLTDLearn[LTDkToId[1]].getStdErr());
	htmlString += QString("<tr><td>LTD2 Static</td><td>-</td><td>%1&plusmn;%2</td><td>-</td></tr>")
		.arg(winAccurLTDStatic[LTDkToId[2]].getMean()).arg(winAccurLTDStatic[LTDkToId[2]].getStdErr());
	htmlString += QString("<tr><td>LTD2 Learn</td><td>-</td><td>%1&plusmn;%2</td><td>-</td></tr>")
		.arg(winAccurLTDLearn[LTDkToId[2]].getMean()).arg(winAccurLTDLearn[LTDkToId[2]].getStdErr());

	std::vector<std::string> testName = { "Sustained Static", "Decreased Static", "Decreased Learn",
		"Sustained Static", "Decreased Static", "Decreased Learn", "Sustained Static", "Decreased Static",
		"Decreased Learn", "Sustained Learn", "Sustained Learn", "Sustained Learn", "Lanchester Static",
		"Lanchester Learn", "Lanchester Static", "Lanchester Learn", "Lanchester Static", "Lanchester Learn" };

	std::vector<int> randomTargetIndex = { 12, 13, 0, 9, 1, 2 };
	for (auto i : randomTargetIndex) {
		htmlString += QString("<tr><td>[%6]%7</td><td>%1&plusmn;%4</td><td>%2&plusmn;%5</td><td>%3</td></tr>")
			.arg(stateSimilarity[i].getMean()).arg(winnerAccur[i].getMean()).arg(timers[i].getElapsedTime())
			.arg(stateSimilarity[i].getStdErr()).arg(winnerAccur[i].getStdErr())
			.arg(i).arg(testName[i].c_str());
	}
	htmlString += QString("<tr><td colspan=\"4\"><b>Sorted by score</b></td></tr>");
	std::vector<int> scoreTargetIndex = { 14, 15, 3, 11, 4, 5 };
	for (auto i : scoreTargetIndex) {
		htmlString += QString("<tr><td>[%6]%7</td><td>%1&plusmn;%4</td><td>%2&plusmn;%5</td><td>%3</td></tr>")
			.arg(stateSimilarity[i].getMean()).arg(winnerAccur[i].getMean()).arg(timers[i].getElapsedTime())
			.arg(stateSimilarity[i].getStdErr()).arg(winnerAccur[i].getStdErr())
			.arg(i).arg(testName[i].c_str());
	}
	htmlString += QString("<tr><td colspan=\"4\"><b>Sorted by Borda Count</b></td></tr>");
	std::vector<int> bordaTargetIndex = { 16, 17, 6, 10, 7, 8 };
	for (auto i : bordaTargetIndex) {
		htmlString += QString("<tr><td>[%6]%7</td><td>%1&plusmn;%4</td><td>%2&plusmn;%5</td><td>%3</td></tr>")
			.arg(stateSimilarity[i].getMean()).arg(winnerAccur[i].getMean()).arg(timers[i].getElapsedTime())
			.arg(stateSimilarity[i].getStdErr()).arg(winnerAccur[i].getStdErr())
			.arg(i).arg(testName[i].c_str());
	}

	if (SIMULATE_SPARCAFT) {
		int offset = SIMULATIONS_EXPERIMENTS;
		htmlString += "<tr><td colspan=\"5\"><b>SparCraft</b></td></tr>";
		htmlString += QString("<tr><td>closest</td><td>%1&plusmn;%3</td><td>%2&plusmn;%4</td><td>%5</td></tr>")
			.arg(stateSimilarity[offset].getMean()).arg(winnerAccur[offset].getMean())
			.arg(stateSimilarity[offset].getStdErr()).arg(winnerAccur[offset].getStdErr())
			.arg(timers[offset].getElapsedTime());
#ifndef _DEBUG
		htmlString += QString("<tr><td>weakest</td><td>%1&plusmn;%3</td><td>%2&plusmn;%4</td><td>%5</td></tr>")
			.arg(stateSimilarity[offset + 2].getMean()).arg(winnerAccur[offset + 2].getMean())
			.arg(stateSimilarity[offset + 2].getStdErr()).arg(winnerAccur[offset + 2].getStdErr())
			.arg(timers[offset + 2].getElapsedTime());
		htmlString += QString("<tr><td>DPS</td><td>%1&plusmn;%3</td><td>%2&plusmn;%4</td><td>%5</td></tr>")
			.arg(stateSimilarity[offset + 1].getMean()).arg(winnerAccur[offset + 1].getMean())
			.arg(stateSimilarity[offset + 1].getStdErr()).arg(winnerAccur[offset + 1].getStdErr())
			.arg(timers[offset + 1].getElapsedTime());
		htmlString += QString("<tr><td>NOKDPS</td><td>%1&plusmn;%3</td><td>%2&plusmn;%4</td><td>%5</td></tr>")
			.arg(stateSimilarity[offset + 3].getMean()).arg(winnerAccur[offset + 3].getMean())
			.arg(stateSimilarity[offset + 3].getStdErr()).arg(winnerAccur[offset + 3].getStdErr())
			.arg(timers[offset + 3].getElapsedTime());
		htmlString += QString("<tr><td>kiter</td><td>%1&plusmn;%3</td><td>%2&plusmn;%4</td><td>%5</td></tr>")
			.arg(stateSimilarity[offset + 4].getMean()).arg(winnerAccur[offset + 4].getMean())
			.arg(stateSimilarity[offset + 4].getStdErr()).arg(winnerAccur[offset + 4].getStdErr())
			.arg(timers[offset + 4].getElapsedTime());
		htmlString += QString("<tr><td>kiter DPS</td><td>%1&plusmn;%3</td><td>%2&plusmn;%4</td><td>%5</td></tr>")
			.arg(stateSimilarity[offset + 5].getMean()).arg(winnerAccur[offset + 5].getMean())
			.arg(stateSimilarity[offset + 5].getStdErr()).arg(winnerAccur[offset + 5].getStdErr())
			.arg(timers[offset + 5].getElapsedTime());
#endif
	}
	htmlString += "</table><br>";

	htmlString += "<b>Avg. Win-prediction accuracy by LTD-k Static: </b>";
	for (const auto& k : LTDk) htmlString += QString("%1,").arg(k);
	htmlString += "<br>";
	for (const auto& winAcc : winAccurLTDStatic) htmlString += QString("%1,").arg(winAcc.getMean());
	htmlString += "<br>";

	// for winner accuracy it doesn't matter the target selection policy
// 	htmlString += "<b>Avg. Win-prediction accuracy by groups: </b>";
// 	auto groupWinIt = groupWin[0].begin();
// 	for (size_t i = 0; i < groupWin[0].size(); ++i) {
// 		htmlString += QString("%1,").arg(groupWinIt->first);
// 		groupWinIt++;
// 	}
// 
// 	htmlString += "<br>LTD2 Static: ";
// 	for (const auto& winAcc : groupWinLTDStatic[LTDkToId[2]]) htmlString += QString("%1,").arg(winAcc.second.getMean());
// 	htmlString += "<br>[0]Sustained Static: ";
// 	for (const auto& winAcc : groupWin[0]) htmlString += QString("%1,").arg(winAcc.second.getMean());
// 	htmlString += "<br>[9]Sustained Learn: ";
// 	for (const auto& winAcc : groupWin[9]) htmlString += QString("%1,").arg(winAcc.second.getMean());
// 	htmlString += "<br>[1]Decreased Static: ";
// 	for (const auto& winAcc : groupWin[1]) htmlString += QString("%1,").arg(winAcc.second.getMean());
// 	htmlString += "<br>[2]Decreased Learn: ";
// 	for (const auto& winAcc : groupWin[2]) htmlString += QString("%1,").arg(winAcc.second.getMean());
// 	htmlString += "<br>[12]Lanchester Static: ";
// 	for (const auto& winAcc : groupWin[12]) htmlString += QString("%1,").arg(winAcc.second.getMean());
// 	htmlString += "<br>[13]Lanchester Learn: ";
// 	for (const auto& winAcc : groupWin[13]) htmlString += QString("%1,").arg(winAcc.second.getMean());
// 
// 	if (SIMULATE_SPARCAFT) {
// 		int offset = SIMULATIONS_EXPERIMENTS;
// 		htmlString += "<br>closest: ";
// 		for (const auto& winAcc : groupWin[offset]) htmlString += QString("%1,").arg(winAcc.second.getMean());
// #ifndef _DEBUG
// 		htmlString += "<br>weakest: ";
// 		for (const auto& winAcc : groupWin[offset+2]) htmlString += QString("%1,").arg(winAcc.second.getMean());
// 		htmlString += "<br>DPS: ";
// 		for (const auto& winAcc : groupWin[offset+1]) htmlString += QString("%1,").arg(winAcc.second.getMean());
// 		htmlString += "<br>NOKDPS: ";
// 		for (const auto& winAcc : groupWin[offset+3]) htmlString += QString("%1,").arg(winAcc.second.getMean());
// 		htmlString += "<br>kiter: ";
// 		for (const auto& winAcc : groupWin[offset+4]) htmlString += QString("%1,").arg(winAcc.second.getMean());
// 		htmlString += "<br>kiter DPS: ";
// 		for (const auto& winAcc : groupWin[offset+5]) htmlString += QString("%1,").arg(winAcc.second.getMean());
// #endif
// 	}

	combatLog->insertHtml(htmlString);
}

void myQtApp::addUnitsFromReplaytoGameState(std::map<int, unitInfo> armyUnits, GameState &myGameState, SparCraft::GameState &sparCraftState, 
	int playerID, bool &hasMines, bool &hasTransports, bool &skipSparCraft,
	bool SKIP_TRANSPORTS, bool ONLY_SPARCRAFT_SUPPORTED, bool IMPORT_FULL_HP_SPARCAFT)
{
	SparCraft::IDType sparCraftPlayerID = SparCraft::Players::Player_One;
	if (playerID == 2) sparCraftPlayerID = SparCraft::Players::Player_Two;

	for (auto army : armyUnits) {
		if (army.second.typeID == BWAPI::UnitTypes::Enum::Terran_Vulture_Spider_Mine ||
			army.second.typeID == BWAPI::UnitTypes::Enum::Zerg_Scourge) {
			hasMines = true;
			break;
		}
		if (SKIP_TRANSPORTS) {
			BWAPI::UnitType uType(army.second.typeID);
			if (uType.spaceProvided() > 0 && army.second.typeID != BWAPI::UnitTypes::Enum::Terran_Bunker) {
				hasTransports = true;
				break;
			}
		}
		// the region (8) is arbitrary, but should be the same
		myGameState.addUnit(army.second.typeID, 8, playerID, abstractOrder::Attack, army.second.HP + army.second.shield);
		//SparCraft doesn't support all units
		if (skipSparCraft || !SparCraft::System::isSupportedUnitType(army.second.typeID)
			|| army.second.typeID == BWAPI::UnitTypes::Enum::Terran_Dropship
			|| army.second.typeID == BWAPI::UnitTypes::Enum::Terran_Missile_Turret
			|| army.second.typeID == BWAPI::UnitTypes::Enum::Zerg_Guardian) {
// 			LOG("SparCraft skipped because unit " << BWAPI::UnitType(army.second.typeID));
			skipSparCraft = true;
			if (ONLY_SPARCRAFT_SUPPORTED) break;
		} else {
			if (IMPORT_FULL_HP_SPARCAFT)
				sparCraftState.addUnit(army.second.typeID, sparCraftPlayerID, SparCraft::Position(army.second.x, army.second.y));
			else {
				// we creat the SparCraft unit with the current HP
				SparCraft::Unit unit(army.second.typeID, SparCraft::Position(army.second.x, army.second.y), 0, sparCraftPlayerID,
					army.second.HP+army.second.shield, 50, 0, 0);
				sparCraftState.addUnit(unit);
			}
		}
	}
}

void myQtApp::addCombatStats(Statistic &statsCombatLength, Statistic &statsCombatGroups, Statistic &statsCombatUnits, const combatInfo  &combat, const GameState &gameState)
{	
	int combatLength = combat.endFrame - combat.startFrame;
// 	if (combatLength < 5) { // debug short combats (they should be "insta-kills")
// 		LOG("Combat Length of " << combatLength);
// 		LOG("Army A start");
// 		for (auto army : combat.armyUnits1) LOG("Unit: " << army.second.typeName << " HP: " << army.second.HP);
// 		LOG("Army B start");
// 		for (auto army : combat.armyUnits2) LOG("Unit: " << army.second.typeName << " HP: " << army.second.HP);
// 		LOG("Army A end");
// 		for (auto army : combat.armyUnitsEnd1) LOG("Unit: " << army.second.typeName << " HP: " << army.second.HP);
// 		LOG("Army B end");
// 		for (auto army : combat.armyUnitsEnd2) LOG("Unit: " << army.second.typeName << " HP: " << army.second.HP);
// 	}
	statsCombatLength.add(combatLength);
	statsCombatUnits.add(combat.armyUnits1.size() + combat.armyUnits2.size());
	statsCombatGroups.add(gameState._army.friendly.size() + gameState._army.enemy.size());
}

// translate SparCraft gameState to ours in order to compare states
GameState myQtApp::getGameState(SparCraft::GameState sparCraftGameState)
{
	GameState myGameState(new CombatSimSustained(&unitStatic->DPF));
	for (SparCraft::UnitCountType u(0); u < sparCraftGameState.numUnits(SparCraft::Players::Player_One); ++u) {
		const SparCraft::Unit & unit(sparCraftGameState.getUnit(SparCraft::Players::Player_One, u));
		myGameState.addUnit(unit.typeID(), 8, 1, abstractOrder::Attack, unit.currentHP());
	}
	for (SparCraft::UnitCountType u(0); u < sparCraftGameState.numUnits(SparCraft::Players::Player_Two); ++u) {
		const SparCraft::Unit & unit(sparCraftGameState.getUnit(SparCraft::Players::Player_Two, u));
		myGameState.addUnit(unit.typeID(), 8, 2, abstractOrder::Attack, unit.currentHP());
	}
	return myGameState;
}

void myQtApp::printError(QTextEdit * textEdit, const QString & text)
{
	// save    
	int fw = textEdit->fontWeight();
	QColor tc = textEdit->textColor();
	// append
	textEdit->setFontWeight(QFont::DemiBold);
	textEdit->setTextColor(QColor("red"));
	textEdit->append(text);
	// restore
	textEdit->setFontWeight(fw);
	textEdit->setTextColor(tc);
}

void myQtApp::parseCombats()
{
	std::string datasetPath = combatDatasetLine->text().toStdString();
	combatLog->append(QString("Analyzing folder %1").arg(datasetPath.c_str()));
	combatLog->repaint();

	// Get the list of replays
	std::vector<std::string> replaysPath;
	unsigned found = datasetPath.find_last_of("/\\");
	std::string path = datasetPath.substr(0, found);
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile((_bstr_t)datasetPath.c_str(), &FindFileData);
	while (hFind != INVALID_HANDLE_VALUE) {
		replaysPath.push_back(path + FindFileData.cFileName);
		if (!FindNextFile(hFind, &FindFileData)) {
			FindClose(hFind);
			hFind = INVALID_HANDLE_VALUE;
		}
	}

	// start parsing files
	combatSimBar->setMaximum(replaysPath.size());
	combatSimBar->setValue(0);
	for (const auto& filePath : replaysPath) {
		combatSimBar->setValue(combatSimBar->value() + 1);
		learner.parseReplayFile(filePath);
	}
	learner.combatsSanityCheck();

	// show results stats
	int combatsFound = learner.armyDestroyed + learner.armyReinforcement + learner.armyPeace + 
		learner.armyGameEnd - learner.armyKilledWithoutKills - learner.moreThanTwoArmies - learner.passiveArmy - learner.corruptedCombats;
	combatLog->append(QString("%1 combats found (with no errors)").arg(combatsFound));
	combatLog->append(QString("%1 combats ended by army destroyed").arg(learner.armyDestroyed));
	combatLog->append(QString("%1 combats ended by reinforcement").arg(learner.armyReinforcement));
	combatLog->append(QString("%1 combats ended by peace").arg(learner.armyPeace));
	combatLog->append(QString("%1 combats ended by game end").arg(learner.armyGameEnd));
	combatLog->append(QString("%1 combats valid to learn (army destroyed with no errors)").arg(learner.allCombats.size()));
	if (learner.armyKilledWithoutKills > 0)
		printError(combatLog, QString("%1 combats ended by army destroyed without kills").arg(learner.armyKilledWithoutKills));
	if (learner.moreThanTwoArmies > 0)
		printError(combatLog, QString("%1 combats with more than 2 armies").arg(learner.moreThanTwoArmies));
	if (learner.passiveArmy > 0)
		printError(combatLog, QString("%1 combats with a passive army").arg(learner.passiveArmy));
	if (learner.corruptedCombats > 0)
		printError(combatLog, QString("%1 combats were corrupted").arg(learner.corruptedCombats));

	combatsParsedLabel->setText(QString("%1").arg(learner.allCombats.size()));
	learnButton->setEnabled(true);
	combatClearButton->setEnabled(true);
	crossValidationButton->setEnabled(true);
}

void myQtApp::learnCombat()
{
	// be sure the learn stuff is clear
	learner.clear();

	// Learn DPF from combat replays data
	learner.calculateDPF(DPF_noTransport->isChecked(), DPF_1type->isChecked());
	if (checkBWAPIupperBound->isChecked()) {
		learner.damageUpperBound();
	} else {
		// cleaning stats
		learner.DPFbounded.clear();
		for (int matchupInt = UnitInfoLearner::Matchups::TVT; matchupInt != UnitInfoLearner::Matchups::NONE; matchupInt++) {
			learner.DPFbounded.insert(std::pair<UnitInfoLearner::Matchups, int>(static_cast<UnitInfoLearner::Matchups>(matchupInt), 0));
		}
	}
	combatLog->append(QString("DPS calculated from %1 combats").arg(learner.combatsProcessed));
	
	for (int matchupInt = UnitInfoLearner::Matchups::TVT; matchupInt != UnitInfoLearner::Matchups::NONE; matchupInt++) {
		UnitInfoLearner::Matchups matchId = static_cast<UnitInfoLearner::Matchups>(matchupInt);
		if (learner.DPFcases[matchId].noCases > 0) {
			printError(combatLog, QString("[%1] Detected %2 DPF cases (%3 missing, %4 bounded, %5 learned)")
				.arg(learner.getMatchupName(matchId).c_str())
				.arg(learner.DPFcases[matchId].size - learner.DPFcases[matchId].noCases)
				.arg(learner.DPFcases[matchId].noCases)
				.arg(learner.DPFbounded[matchId])
				.arg(learner.DPFcases[matchId].size - learner.DPFcases[matchId].noCases - learner.DPFbounded[matchId])
				);
		}
	}
	
	// Learn target selection from combat replays data
	learner.learnTargetBordaCount();

	getDPFbutton->setEnabled(true);
	combatSimulatorButton->setEnabled(true);
}

void myQtApp::clearCombatsParsed()
{
	learner.allCombats.clear();
	learner.clear();
	learnButton->setEnabled(false);
	combatClearButton->setEnabled(false);

	getDPFbutton->setEnabled(false);
	combatSimulatorButton->setEnabled(false);
	crossValidationButton->setEnabled(false);

	combatsParsedLabel->setText("0");
}

void myQtApp::getDPF()
{
	int unitType1 = (comboDPF1->itemData(comboDPF1->currentIndex())).toInt();
	int unitType2 = (comboDPF2->itemData(comboDPF2->currentIndex())).toInt();

	combatLog->append(QString("Getting DPF of %1(%2) vs %3(%4)").arg(comboDPF1->currentText()).arg(unitType1).arg(comboDPF2->currentText()).arg(unitType2));
	
	DPF_learned->setText(QString("%1").arg(learner.unitTypeDPF[unitType1][unitType2]));
	DPF_BWAPI->setText(QString("%1").arg(unitStatic->typeDPF[unitType1][unitType2]));

	DPF_learned_2->setText(QString("%1").arg(learner.unitTypeDPF[unitType2][unitType1]));
	DPF_BWAPI_2->setText(QString("%1").arg(unitStatic->typeDPF[unitType2][unitType1]));

	BWAPI::UnitType bwapiType(unitType1);
	HP_label1->setText(QString("%1 (%2)").arg(bwapiType.maxHitPoints()).arg(bwapiType.maxShields()));
	BWAPI::UnitType bwapiType2(unitType2);
	HP_label2->setText(QString("%1 (%2)").arg(bwapiType2.maxHitPoints()).arg(bwapiType2.maxShields()));
}

// **** Utilities for cross validation
void getFoldIndices(const size_t fold, const std::vector<size_t> foldIndices, std::vector<size_t>& testIndices, std::vector<size_t>& trainingIndices)
{
	testIndices.clear();
	trainingIndices.clear();
	for (size_t i = 0; i < foldIndices.size(); ++i) {
		if (foldIndices[i] == fold) {
			testIndices.emplace_back(i);
		} else {
			trainingIndices.emplace_back(i);
		}
	}
}

// Calculate on what fold each index belongs
std::vector<size_t> createFoldIndices(const size_t& folds, const size_t& maxSize) {
	assert(folds <= maxSize);
	// create initial fold indices
	std::vector<size_t> foldIndices(maxSize);
	for (size_t i = 0; i < maxSize; ++i) {
		foldIndices[i] = i%folds;
	}

	// shuffle fold indices
	std::random_device rd;
	std::mt19937 gen(rd());
	std::shuffle(foldIndices.begin(), foldIndices.end(), gen);

	return foldIndices;
}

// after parsing the combats we can perform a 10-fold cross validation
void myQtApp::crossValidation()
{
	const int NUMBER_OF_TESTS = 12;
	bool hasMines = false;
	bool hasTransports = false;
	bool skipSparCraft = false;
	const bool SKIP_TRANSPORTS = checkSkipTransports->isChecked();
	const bool SKIP_PURE_HOMOGENEOUS = checkSkipPureHomo->isChecked();
	const bool SKIP_HOMOGENEOUS = checkSkipHomo->isChecked();
	const bool SKIP_PARTIAL_HOMOGENEOUS = checkSkipPartialHomo->isChecked();
	const bool SKIP_HETEROGENEOUS = checkSkipHete->isChecked();
	const bool SKIP_SPARCRAFT_NOT_SUPPORTED = checkSkipSparcraft->isChecked();
	std::mt19937 g(123456); // fixed seed for experimentation purpose
	std::vector<int> LTDk = { 1, 2 };

	const size_t folds = foldsEdit->text().toInt();;
	std::vector<size_t> foldIndices(createFoldIndices(folds, learner.allCombats.size()));
	Statistic avgWinAccurLTDLearn[2];
	Statistic avgStateSimilarity[NUMBER_OF_TESTS];
	Statistic avgWinnerAccur[NUMBER_OF_TESTS];

	std::vector<std::string> testNames = {"Lanchester Learn", "Sustained Learn", "Decreased Learn",
		"Lanchester Learn", "Sustained Learn", "Decreased Learn",
		"Lanchester Static", "Lanchester Learn", "Sustained Static", "Sustained Learn", "Decreased Static", "Decreased Learn" };

	combatSimBar->setMaximum(folds);
	combatSimBar->setValue(0);

	std::string configVals;
	if (SKIP_SPARCRAFT_NOT_SUPPORTED) configVals += "only SparCraft dataset";
	else configVals += "full dataset";
	combatLog->append(QString("%1-Cross Validation %2").arg(folds).arg(configVals.c_str()));
	
	// stats for total combat simulations
	Statistic totalWinAccurLTDLearn[2];
	Statistic totalStateSimilarity[NUMBER_OF_TESTS];
	Statistic totalWinnerAccur[NUMBER_OF_TESTS];

	// For each fold compute the accuracy
	for (size_t kFold = 0; kFold != folds; ++kFold) {
		combatSimBar->setValue(kFold+1);
		std::vector<size_t> testIndices, trainingIndices;
		getFoldIndices(kFold, foldIndices, testIndices, trainingIndices);

		// TRAINING: learn DPF and Borda Count
		learner.learnTrainingSet(trainingIndices);

		// TESTING: simulate combats 
		// **************************************************************
		Statistic winAccurLTDLearn[2];
		Statistic stateSimilarity[NUMBER_OF_TESTS];
		Statistic winnerAccur[NUMBER_OF_TESTS];



		for (const auto& n : trainingIndices) {
			const combatInfo& combat = learner.allCombats[n];

			// **************************************************************
			// import initState
			// **************************************************************
			GameState initalState(new CombatSimLanchester(&learner.unitDPF));
			SparCraft::GameState sparCraftInitialState; // not used

			// set flags to false
			hasMines = false;
			hasTransports = false;
			skipSparCraft = false;
			// Add units from data replay to game state
			addUnitsFromReplaytoGameState(combat.armyUnits1, initalState, sparCraftInitialState, 1,
				hasMines, hasTransports, skipSparCraft,
				SKIP_TRANSPORTS, SKIP_SPARCRAFT_NOT_SUPPORTED, false);

			addUnitsFromReplaytoGameState(combat.armyUnits2, initalState, sparCraftInitialState, 2,
				hasMines, hasTransports, skipSparCraft,
				SKIP_TRANSPORTS, SKIP_SPARCRAFT_NOT_SUPPORTED, false);

			// skip some combats
			if (hasMines) continue;
			if (SKIP_TRANSPORTS && hasTransports) continue;
			if (SKIP_SPARCRAFT_NOT_SUPPORTED && skipSparCraft) continue;

			if ((initalState._army.friendly.size() == 1 && initalState._army.enemy.size() == 1)) {
				if (SKIP_PURE_HOMOGENEOUS && (initalState._army.friendly[0]->unitTypeId == initalState._army.enemy[0]->unitTypeId)) {
					continue;
				} else if (SKIP_HOMOGENEOUS && (initalState._army.friendly[0]->unitTypeId != initalState._army.enemy[0]->unitTypeId)) {
					continue;
				}
			}
			if (SKIP_PARTIAL_HOMOGENEOUS &&
				((initalState._army.friendly.size() == 1 && initalState._army.enemy.size() > 1) ||
				(initalState._army.friendly.size() > 1 && initalState._army.enemy.size() == 1))) {
				continue;
			}
			if (SKIP_HETEROGENEOUS && (initalState._army.friendly.size() > 1 && initalState._army.enemy.size() > 1)) {
				continue;
			}

			// to have a "random" target selection, we shuffle the vectors
			std::shuffle(initalState._army.friendly.begin(), initalState._army.friendly.end(), g);
			std::shuffle(initalState._army.enemy.begin(), initalState._army.enemy.end(), g);
			initalState.calculateExpectedEndFrameForAllGroups();


			// **************************************************************
			// import finalState
			// **************************************************************
			GameState finalState;
			for (auto army : combat.armyUnitsEnd1) {
				finalState.addUnit(army.second.typeID, 8, 1, abstractOrder::Attack, army.second.HP + army.second.shield);
			}
			for (auto army : combat.armyUnitsEnd2) {
				finalState.addUnit(army.second.typeID, 8, 2, abstractOrder::Attack, army.second.HP + army.second.shield);
			}

			// **************************************************************
			// create a game state for each simulator
			// **************************************************************
			GameState simulators[NUMBER_OF_TESTS];
			for (auto& simulator : simulators) simulator = initalState; // set the initial state to all simulators
			// sorted by RANDOM
			simulators[0].changeCombatSimulator(new CombatSimLanchester(&learner.unitDPF));
			simulators[1].changeCombatSimulator(new CombatSimSustained(&learner.unitDPF));
			simulators[2].changeCombatSimulator(new CombatSimDecreased(&learner.unitTypeDPF, &unitStatic->DPF));
			// Sorted by Score
			comp_f targetPolicy = sortByScore;
			simulators[3].changeCombatSimulator(new CombatSimLanchester(&learner.unitDPF, targetPolicy, targetPolicy));
			simulators[4].changeCombatSimulator(new CombatSimSustained(&learner.unitDPF, targetPolicy, targetPolicy));
			simulators[5].changeCombatSimulator(new CombatSimDecreased(&learner.unitTypeDPF, &unitStatic->DPF, targetPolicy, targetPolicy));
			// Sorted by type priority learned
			std::vector<float> priority1;
			UnitInfoLearner::GroupDiversity groupDiversity1 = learner.getGroupDiversity(combat.armySize1);
			switch (groupDiversity1) {
			case UnitInfoLearner::AIR:	  priority1 = learner.typePriorityAir; break;
			case UnitInfoLearner::GROUND: priority1 = learner.typePriorityGround; break;
			case UnitInfoLearner::BOTH:	  priority1 = learner.typePriorityBoth; break;
			}
			sortByTypeClass sortByType1(&priority1);

			std::vector<float> priority2;
			UnitInfoLearner::GroupDiversity groupDiversity2 = learner.getGroupDiversity(combat.armySize2);
			switch (groupDiversity2) {
			case UnitInfoLearner::AIR:	  priority2 = learner.typePriorityAir; break;
			case UnitInfoLearner::GROUND: priority2 = learner.typePriorityGround; break;
			case UnitInfoLearner::BOTH:	  priority2 = learner.typePriorityBoth; break;
			}
			sortByTypeClass sortByType2(&priority2);

			simulators[6].changeCombatSimulator(new CombatSimLanchester(&unitStatic->DPF, sortByType2, sortByType1));
			simulators[7].changeCombatSimulator(new CombatSimLanchester(&learner.unitDPF, sortByType2, sortByType1));
			simulators[8].changeCombatSimulator(new CombatSimSustained(&unitStatic->DPF, sortByType2, sortByType1));
			simulators[9].changeCombatSimulator(new CombatSimSustained(&learner.unitDPF, sortByType2, sortByType1));
			simulators[10].changeCombatSimulator(new CombatSimDecreased(&unitStatic->typeDPF, &unitStatic->DPF, sortByType2, sortByType1));
			simulators[11].changeCombatSimulator(new CombatSimDecreased(&learner.unitTypeDPF, &unitStatic->DPF, sortByType2, sortByType1));



			// **************************************************************
			// simulate all states with different simulators
			// **************************************************************

			for (int i = 0; i < NUMBER_OF_TESTS; ++i) {
				simulators[i].moveForward();
			}

			// **************************************************************
			// compare states against finalState
			// **************************************************************
			// LTDk winner prediction
			for (size_t index(0); index < LTDk.size(); ++index) {
				// Learned DPF
				float score = EvalCombat::LTD(learner.unitDPF, combat, LTDk[index]);
				if ((score > 0 && finalState.winner() == 0) || (score < 0 && finalState.winner() == 1) ||
					(score == 0 && finalState.winner() == -1)) {
					winAccurLTDLearn[index].add(1);
					totalWinAccurLTDLearn[index].add(1);
				} else {
					winAccurLTDLearn[index].add(0);
					totalWinAccurLTDLearn[index].add(0);
				}
			}
			// Combat model winner prediction and state similarity
			for (int i = 0; i < NUMBER_OF_TESTS; ++i) {
				stateSimilarity[i].add(finalState.getPredictionAccuracy(initalState, simulators[i]));
				winnerAccur[i].add(finalState.winner() == simulators[i].winner());

				totalStateSimilarity[i].add(finalState.getPredictionAccuracy(initalState, simulators[i]));
				totalWinnerAccur[i].add(finalState.winner() == simulators[i].winner());
			}

			// logs to calculate the p-value
// 			LOG(finalState.getPredictionAccuracy(initalState, simulators[0]) << "," <<
// 				finalState.getPredictionAccuracy(initalState, simulators[1]) << "," <<
// 				finalState.getPredictionAccuracy(initalState, simulators[2]) << "," <<
// 				finalState.getPredictionAccuracy(initalState, simulators[3]) << "," <<
// 				finalState.getPredictionAccuracy(initalState, simulators[4]) << "," <<
// 				finalState.getPredictionAccuracy(initalState, simulators[5]) << "," <<
// 				finalState.getPredictionAccuracy(initalState, simulators[6]) << "," <<
// 				finalState.getPredictionAccuracy(initalState, simulators[7]) << "," <<
// 				finalState.getPredictionAccuracy(initalState, simulators[8]) << "," <<
// 				finalState.getPredictionAccuracy(initalState, simulators[9]) << "," <<
// 				finalState.getPredictionAccuracy(initalState, simulators[10]) << "," <<
// 				finalState.getPredictionAccuracy(initalState, simulators[11]));
		}

		// all combats simulated, save results of this fold, save avg between folds
		for (size_t index(0); index < LTDk.size(); ++index) {
			avgWinAccurLTDLearn[index].add(winAccurLTDLearn[index].getMean());
		}
		for (int i = 0; i < NUMBER_OF_TESTS; ++i) {
			avgStateSimilarity[i].add(stateSimilarity[i].getMean());
			avgWinnerAccur[i].add(winnerAccur[i].getMean());
		}
	}

	// print results

	combatLog->append("Final Result:");
	QString htmlString = "<table><tr><td>Simulator</td><td>Final state pred</td><td>Win-pred accuracy</td></tr>";
	htmlString += QString("<tr><td>LTD Learn</td><td>-</td><td>%1&plusmn;%2</td></tr>")
		.arg(avgWinAccurLTDLearn[1].getMean()).arg(totalWinAccurLTDLearn[1].getStdErr());
	htmlString += QString("<tr><td>LTD2 Learn</td><td>-</td><td>%1&plusmn;%2</td></tr>")
		.arg(avgWinAccurLTDLearn[2].getMean()).arg(totalWinAccurLTDLearn[2].getStdErr());

	for (size_t i = 0; i < testNames.size(); ++i) {
		if (i == 3) htmlString += QString("<tr><td colspan=\"3\"><b>Sorted by score</b></td></tr>");
		if (i == 6) htmlString += QString("<tr><td colspan=\"3\"><b>Sorted by Borda Count</b></td></tr>");

		htmlString += QString("<tr><td>%5</td><td>%1&plusmn;%3</td><td>%2&plusmn;%4</td></tr>")
			.arg(avgStateSimilarity[i].getMean()).arg(avgWinnerAccur[i].getMean())
			.arg(totalStateSimilarity[i].getStdErr()).arg(totalWinnerAccur[i].getStdErr())
			.arg(testNames[i].c_str());
	}

	htmlString += "</table><br>";
	combatLog->insertHtml(htmlString);

}

void myQtApp::testUnitSelected(int index)
{
	int unitTypeId = (comboTestUnits->itemData(index)).toInt();
	int maxHP = unitStatic->HP[unitTypeId].any;
	testUnitHP->setText(QString::number(maxHP));
}

void myQtApp::addToCombat(QTableWidget* table)
{
	int unitTypeId = (comboTestUnits->itemData(comboTestUnits->currentIndex())).toInt();
	int numberUnits = testUnitSize->text().toInt();
	int avgHP = testUnitHP->text().toInt();
// 	textEdit->append("Adding " + QString::number(numberUnits) + " unit id " + QString::number(unitTypeId) + " at group " + QString::number(groupId));

	// add to table
	int row = table->rowCount();
	table->insertRow(row);
	table->setItem(row, 0, new QTableWidgetItem(tr("%1").arg(unitTypeId)));
	table->setItem(row, 1, new QTableWidgetItem(tr("%1").arg(BWAPI::UnitType(unitTypeId).getName().c_str())));
	table->setItem(row, 2, new QTableWidgetItem(tr("%1").arg(numberUnits)));
	table->setItem(row, 3, new QTableWidgetItem(tr("%1").arg(avgHP)));
	table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void myQtApp::clearGroup1()
{
	groupTestTable1->setRowCount(0);
}

void myQtApp::clearGroup2()
{
	groupTestTable2->setRowCount(0);
}

void myQtApp::testCombat()
{
	// select DPF
	std::vector<DPF_t>* maxDPF;
	std::vector<std::vector<double> >* unitTypeDPF;
	switch (comboDpfType->currentIndex()) {
	case 0: // static DPF
		maxDPF = &unitStatic->DPF;
		unitTypeDPF = &unitStatic->typeDPF;
		break;
	default: textEdit->append("Combat Model configuration not implemented yet"); return;
	}

	// select target selection
	comp_f comparator;
	switch (comboTargetSelection->currentIndex()) {
	case 0: comparator = nullptr; break;
	case 1: comparator = sortByScore; break;
	case 2: comparator = sortByDps; break;
	default: textEdit->append("Combat Model configuration not implemented yet"); return;
	}

	// define combat simulator
	CombatSimulator* combatSim;
	switch (comboSimType->currentIndex()) {
	case 0: combatSim = new CombatSimLanchester(maxDPF, comparator, comparator); break;
	case 1: combatSim = new CombatSimSustained(maxDPF, comparator, comparator); break;
	case 2: combatSim = new CombatSimDecreased(unitTypeDPF, maxDPF, comparator, comparator); break;
	default: textEdit->append("Combat Model configuration not implemented yet"); return;
	}

	// import state 
	GameState state(combatSim);
	for (int row = 0; row < groupTestTable1->rowCount(); ++row) {
		int typeID = groupTestTable1->item(row, 0)->text().toInt();
		int size = groupTestTable1->item(row, 2)->text().toInt();
		int avgHP = groupTestTable1->item(row, 3)->text().toInt();
		state.addGroup(typeID, size, 8, 1, avgHP, abstractOrder::Attack, 0, 0);
	}
	for (int row = 0; row < groupTestTable2->rowCount(); ++row) {
		int typeID = groupTestTable2->item(row, 0)->text().toInt();
		int size = groupTestTable2->item(row, 2)->text().toInt();
		int avgHP = groupTestTable2->item(row, 3)->text().toInt();
		state.addGroup(typeID, size, 8, 2, avgHP, abstractOrder::Attack, 0, 0);
	}
	state.calculateExpectedEndFrameForAllGroups();

	// mark siege as researched
	state.friendlySiegeTankResearched = true;
	state.enemySiegeTankResearched = true;

	// simulate
	state.moveForward();

	// print final state
	groupTestTableRes1->setRowCount(0);
	groupTestTableRes2->setRowCount(0);

	QTableWidget* table = groupTestTableRes1;
	for (const auto& unitGroup : state._army.friendly) {
		int row = table->rowCount();
		table->insertRow(row);
		table->setItem(row, 0, new QTableWidgetItem(tr("%1").arg(unitGroup->unitTypeId)));
		table->setItem(row, 1, new QTableWidgetItem(tr("%1").arg(BWAPI::UnitType(unitGroup->unitTypeId).getName().c_str())));
		table->setItem(row, 2, new QTableWidgetItem(tr("%1").arg(unitGroup->numUnits)));
		table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	}
	table = groupTestTableRes2;
	for (const auto& unitGroup : state._army.enemy) {
		int row = table->rowCount();
		table->insertRow(row);
		table->setItem(row, 0, new QTableWidgetItem(tr("%1").arg(unitGroup->unitTypeId)));
		table->setItem(row, 1, new QTableWidgetItem(tr("%1").arg(BWAPI::UnitType(unitGroup->unitTypeId).getName().c_str())));
		table->setItem(row, 2, new QTableWidgetItem(tr("%1").arg(unitGroup->numUnits)));
		table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	}
}