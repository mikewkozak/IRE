#ifndef NOVALAUNCHER_H
#define NOVALAUNCHER_H

#define NOMINMAX

#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <QtWidgets/QMainWindow>
#include <QTimer>
#include <QDateTime>

#include "ui_novalauncher.h"


class NovaLauncher : public QMainWindow, private Ui::NovaLauncherClass
{
	Q_OBJECT

public:
	NovaLauncher(QWidget *parent = 0, Qt::WindowFlags flags = 0);
	~NovaLauncher();

public slots:
 	void startCheckCrash();
	void stopExperiments();
	void checkCrash();
	void showTime();
	void checkFilePaths();

private:
	QTimer* iTimerCrash;
	QTimer* iTimerDisplay;
	QTime timeCounter;
	QTime experimentsTime;

	// experiments configurations ***********************
	// general
	std::vector<int> refreshIntervals;
	int refreshIntervalsIndex;
	std::vector<std::string> spacePartition;
	int spacePartitionIndex;
	std::vector<std::string> buildings;
	int buildingsIndex;
	std::vector<std::string> searchAlgorithm;
	int searchAlgorithmIndex;
	// ABCD
	std::vector<int> ABCD_depth;
	int ABCD_depthIndex;
	std::vector<int> ABCD_downsampling;
	int ABCD_downsamplingIndex;
	std::vector<int> ABCD_timeLimit;
	int ABCD_timeLimitIndex;
	// MCTSCD
	std::vector<int> MCTSCD_depth;
	int MCTSCD_depthIndex;
	std::vector<int> MCTSCD_iterations;
	int MCTSCD_iterationsIndex;
	std::vector<int> MCTSCD_simulationTime;
	int MCTSCD_simulationTimeIndex;

	int totalConfigurations;
	int actualConfiguration;
	// **************************************************

    void printError(const QString & text);
	static BOOL CALLBACK searchCloseButton(HWND hwnd, LPARAM lParam);
	bool launchStarcraft();
	void stopStarcraft();
	void checkGamesCompleted();
	void setExperimentConfigurations();
	void setINIconfig();
	bool nextConfiguration();

	unsigned int FileRead( std::istream & is, std::vector <char> & buff );
	unsigned int CountLines( const std::vector <char> & buff, int sz );
	bool newLogLines();

};

#endif // NOVALAUNCHER_H
