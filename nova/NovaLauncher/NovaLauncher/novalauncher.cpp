#include "novalauncher.h"
#include "iniConfig.h"
#include <tchar.h>
#include <direct.h>
#include "Shlwapi.h" // for PathFileExists()
#include "comutil.h" // for _bstr_t (string conversions)
#include <psapi.h> // for GetProcessMemoryInfo

_bstr_t		currentPath;
_bstr_t		chaosLauncherPath;
_bstr_t		logFileName;
_bstr_t		resultFilePath;

int lastLogFileLines = 0;

// get size in bytes, return a human readable size
std::string readableSize(size_t size) {
	std::stringstream buf;
// 	char buf[10];
	int i = 0;
	const char* units[] = { "B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB" };
	while (size > 1024) {
		size /= 1024;
		i++;
	}
// 	sprintf(buf, "%.*f %s", i, size, units[i]);
	buf << size << " " << units[i];
	return buf.str();
}

NovaLauncher::NovaLauncher(QWidget *parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags)
{
	setupUi(this);

	iTimerCrash = new QTimer(this);
	iTimerDisplay = new QTimer(this);

	refreshIntervalsIndex = 0;
	spacePartitionIndex = 0;
	buildingsIndex = 0;
	searchAlgorithmIndex = 0;
	ABCD_depthIndex = 0;
	ABCD_downsamplingIndex = 0;
	ABCD_timeLimitIndex = 0;
	MCTSCD_depthIndex = 0;
	MCTSCD_iterationsIndex = 0;
	MCTSCD_simulationTimeIndex = 0;

	// signals/slots mechanism in action
	connect(checkCrashButton, SIGNAL(clicked()), this, SLOT(startCheckCrash()));
	connect(stopExperimentsButton, SIGNAL(clicked()), this, SLOT(stopExperiments()));
	connect(checkPathsButton, SIGNAL(clicked()), this, SLOT(checkFilePaths()));

	connect(iTimerCrash, SIGNAL(timeout()), this, SLOT(checkCrash()));
	connect(iTimerDisplay, SIGNAL(timeout()), this, SLOT(showTime()));

	// saving current working directory
	TCHAR pathBuffer[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, pathBuffer);
	currentPath = pathBuffer;

	iniFileLabel->setText(configPath.c_str());

	// checking if needed files exist
	checkFilePaths();
}

NovaLauncher::~NovaLauncher()
{
	delete iTimerCrash;
}

void NovaLauncher::checkFilePaths()
{
	bool ok = true;
	
	QByteArray text1 = chaosPathUI->text().toLocal8Bit();
	chaosLauncherPath = text1.data();
	if (!PathFileExists(chaosLauncherPath)) {
		printError(QString("Path doesn't exist: %1").arg((char*)chaosLauncherPath));
		ok = false;
	}

	if (!PathFileExists((_bstr_t)configPath.c_str())) {
		printError(QString("Path doesn't exist: %1").arg(configPath.c_str()));
		ok = false;
	}

	QByteArray text2 = logPathUI->text().toLocal8Bit();
	logFileName = text2.data();
	if (!PathFileExists(logFileName)) {
		printError(QString("Path doesn't exist: %1").arg((char*)logFileName));
		ok = false;
	}

	QByteArray text3 = resultsPathUI->text().toLocal8Bit();
	resultFilePath = text3.data();
	if (!PathFileExists(resultFilePath)) {
		printError(QString("Path doesn't exist: %1").arg((char*)resultFilePath));
		ok = false;
	}



	if (ok) textEdit->append(QString("All config paths exist"));
}

void NovaLauncher::stopExperiments()
{
    textEdit->append("Stopping Experiments");
	iTimerCrash->stop();
	// stop timer display
	timeLabel->setText("00:00");
	iTimerDisplay->stop();
	stopStarcraft();
	timeToCompleteLabel->setText("00:00:00");
	// resent indices
	refreshIntervalsIndex = 0;
	spacePartitionIndex = 0;
	buildingsIndex = 0;
	searchAlgorithmIndex = 0;
	ABCD_depthIndex = 0;
	ABCD_downsamplingIndex = 0;
	ABCD_timeLimitIndex = 0;
	MCTSCD_depthIndex = 0;
	MCTSCD_iterationsIndex = 0;
	MCTSCD_simulationTimeIndex = 0;
}

void NovaLauncher::startCheckCrash()
{
	setExperimentConfigurations();

	if (totalConfigurations == 0) {
		printError("No configurations to run (possibly error in settings)");
	} else {
		setINIconfig();
		int msecInterval = intervalUI->text().toInt() * 1000;
		textEdit->append(QString("Starting experiments."));
		textEdit->append(QString("Checking StarCraft crush with intervals of %1 seconds.").arg(intervalUI->text()));
		checkCrash();
		iTimerCrash->start(msecInterval);
		// start timer display
		QTime time(0, 0, 0);
		timeCounter = time.addSecs(intervalUI->text().toInt());
		timeLabel->setText(timeCounter.toString("mm:ss"));
		iTimerDisplay->start(1000);

		experimentsTime.start();
	}
}

void NovaLauncher::setExperimentConfigurations()
{
	totalConfigurations = 0;
	actualConfiguration = 0;
	QStringList tmpList;
	// Get config variables
	tmpList = refreshUI->text().split(",", QString::SkipEmptyParts);
	if (tmpList.isEmpty()) return;
	refreshIntervals.clear();
	for (auto it = tmpList.constBegin(); it != tmpList.constEnd(); ++it) {
		refreshIntervals.push_back((*it).toInt());
	}
	tmpList.clear();
	// 	std::vector<std::string> spacePartition;
	spacePartition.clear();
	if (spaceR_UI->isChecked()) spacePartition.push_back("REGIONS");
	if (spaceRC_UI->isChecked()) spacePartition.push_back("REGIONS_AND_CHOKEPOINTS");
	// 	std::vector<std::string> buildings;
	buildings.clear();
	if (buildingsRD_UI->isChecked()) buildings.push_back("RESOURCE_DEPOT");
	if (buildingsALL_UI->isChecked()) buildings.push_back("ALL");
	// 	std::vector<std::string> searchAlgorithm;
	searchAlgorithm.clear();
	if (runABCD_UI->isChecked()) searchAlgorithm.push_back("ABCD");
	if (runMCTSCD_UI->isChecked()) searchAlgorithm.push_back("MCTSCD");
	if (runRundom_UI->isChecked()) searchAlgorithm.push_back("RANDOM");
	// 	// ABCD
	// 	std::vector<int> ABCD_depth;
	tmpList = ABCD_depth_UI->text().split(",", QString::SkipEmptyParts);
	if (tmpList.isEmpty()) return;
	ABCD_depth.clear();
	for (auto it = tmpList.constBegin(); it != tmpList.constEnd(); ++it) {
		ABCD_depth.push_back((*it).toInt());
	}
	tmpList.clear();
	// 	std::vector<int> ABCD_downsampling;
	tmpList = ABCD_downsampling_UI->text().split(",", QString::SkipEmptyParts);
	if (tmpList.isEmpty()) return;
	ABCD_downsampling.clear();
	for (auto it = tmpList.constBegin(); it != tmpList.constEnd(); ++it) {
		ABCD_downsampling.push_back((*it).toInt());
	}
	tmpList.clear();
	// 	std::vector<int> ABCD_timeLimit;
	tmpList = ABCD_timeLimit_UI->text().split(",", QString::SkipEmptyParts);
	if (tmpList.isEmpty()) return;
	ABCD_timeLimit.clear();
	for (auto it = tmpList.constBegin(); it != tmpList.constEnd(); ++it) {
		ABCD_timeLimit.push_back((*it).toInt());
	}
	tmpList.clear();
	// 	// MCTSCD
	// 	std::vector<int> MCTSCD_depth;
	tmpList = MCTSCD_depth_UI->text().split(",", QString::SkipEmptyParts);
	if (tmpList.isEmpty()) return;
	MCTSCD_depth.clear();
	for (auto it = tmpList.constBegin(); it != tmpList.constEnd(); ++it) {
		MCTSCD_depth.push_back((*it).toInt());
	}
	tmpList.clear();
	// 	std::vector<int> MCTSCD_iterations;
	tmpList = MCTSCD_iterations_UI->text().split(",", QString::SkipEmptyParts);
	if (tmpList.isEmpty()) return;
	MCTSCD_iterations.clear();
	for (auto it = tmpList.constBegin(); it != tmpList.constEnd(); ++it) {
		MCTSCD_iterations.push_back((*it).toInt());
	}
	tmpList.clear();
	// 	std::vector<int> MCTSCD_simulationTime;
	tmpList = MCTSCD_simulationTime_UI->text().split(",", QString::SkipEmptyParts);
	if (tmpList.isEmpty()) return;
	MCTSCD_simulationTime.clear();
	for (auto it = tmpList.constBegin(); it != tmpList.constEnd(); ++it) {
		MCTSCD_simulationTime.push_back((*it).toInt());
	}
	tmpList.clear();

	// calculating total configurations
	int basicConfigurations = buildings.size() * spacePartition.size() * refreshIntervals.size();
	if (runABCD_UI->isChecked()) {
		totalConfigurations += basicConfigurations * ABCD_depth.size() * ABCD_downsampling.size() * ABCD_timeLimit.size();
	}
	if (runMCTSCD_UI->isChecked()) {
		totalConfigurations += basicConfigurations * MCTSCD_depth.size() * MCTSCD_iterations.size() * MCTSCD_simulationTime.size();
	}
	if (runRundom_UI->isChecked()) {
		totalConfigurations += basicConfigurations;
	}
}

void NovaLauncher::setINIconfig()
{
	QString configText = QString("%1 => Buildings: %2, Map: %3, Intervals: %4")
		.arg(searchAlgorithm[searchAlgorithmIndex].c_str())
		.arg(buildings[buildingsIndex].c_str())
		.arg(spacePartition[spacePartitionIndex].c_str())
		.arg(refreshIntervals[refreshIntervalsIndex]);
	WriteConfigString("high_level_search", "algorithm", searchAlgorithm[searchAlgorithmIndex].c_str());
	WriteConfigString("high_level_search", "space_partition", spacePartition[spacePartitionIndex].c_str());
	WriteConfigString("high_level_search", "buildings", buildings[buildingsIndex].c_str());
	WriteConfigString("high_level_search", "refresh", intToString(refreshIntervals[refreshIntervalsIndex]).c_str());

	if (searchAlgorithm[searchAlgorithmIndex].compare("ABCD") == 0) {
		configText += QString(" depth: %1, downsampling: %2, timeLimit: %3")
			.arg(ABCD_depth[ABCD_depthIndex])
			.arg(ABCD_downsampling[ABCD_downsamplingIndex])
			.arg(ABCD_timeLimit[ABCD_timeLimitIndex]);
		WriteConfigString("ABCD", "depth", intToString(ABCD_depth[ABCD_depthIndex]).c_str());
		WriteConfigString("ABCD", "downsampling", intToString(ABCD_downsampling[ABCD_downsamplingIndex]).c_str());
		WriteConfigString("ABCD", "time_limit", intToString(ABCD_timeLimit[ABCD_timeLimitIndex]).c_str());
	}
	if (searchAlgorithm[searchAlgorithmIndex].compare("MCTSCD") == 0) {
		configText += QString(" depth: %1, iterations: %2, simulationTime: %3")
			.arg(MCTSCD_depth[MCTSCD_depthIndex])
			.arg(MCTSCD_iterations[MCTSCD_iterationsIndex])
			.arg(MCTSCD_simulationTime[MCTSCD_simulationTimeIndex]);
		WriteConfigString("MCTSCD", "depth", intToString(MCTSCD_depth[MCTSCD_depthIndex]).c_str());
		WriteConfigString("MCTSCD", "iterations", intToString(MCTSCD_iterations[MCTSCD_iterationsIndex]).c_str());
		WriteConfigString("MCTSCD", "max_simulation_time", intToString(MCTSCD_simulationTime[MCTSCD_simulationTimeIndex]).c_str());
	}
	textEdit->append(configText);

	actualConfiguration++;
	numConfigsLabel->setText(QString("%1 of %2").arg(actualConfiguration).arg(totalConfigurations));
}

void NovaLauncher::showTime()
{
	timeCounter = timeCounter.addSecs(-1);
	timeLabel->setText(timeCounter.toString("mm:ss"));
}

void NovaLauncher::checkCrash()
{
	// Reset timer display
	QTime time(0, 0, 0);
	timeCounter = time.addSecs(intervalUI->text().toInt());

	// look if StarCraft crashed
	HWND starcraftErrorWindow = FindWindow(NULL, TEXT("StarCraft"));
	if (starcraftErrorWindow) {
		printError("StarCraft crashed");
		HWND childWindow = FindWindowEx(starcraftErrorWindow, 0, TEXT("DirectUIHWND"), NULL);
		if (childWindow) {
			// find the child window with the button
			EnumChildWindows(childWindow, searchCloseButton, reinterpret_cast<LPARAM>(textEdit));
		}
		Sleep(1000); // let time to close the crashed window
		// sometimes we have a second crash window, try to close it too
		starcraftErrorWindow = FindWindow(NULL, TEXT("StarCraft"));
		if (starcraftErrorWindow) {
			HWND childWindow = FindWindowEx(starcraftErrorWindow, 0, TEXT("DirectUIHWND"), NULL);
			if (childWindow) {
				// find the child window with the button
				EnumChildWindows(childWindow, searchCloseButton, reinterpret_cast<LPARAM>(textEdit));
			}
			Sleep(1000); // let time to close the crashed window
			
			// start StarCraft again
			launchStarcraft();
		}
	} else { // No StarCraft crash window
		HWND starcraftWindow = FindWindow(NULL, TEXT("Brood War"));
		if (starcraftWindow) { // StarCraft still running, we are good

			// get StarCraft memory usage
// 			DWORD processID;
// 			PROCESS_MEMORY_COUNTERS pmc;
// 			GetWindowThreadProcessId(starcraftWindow, &processID);
// 			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
// 			if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
// 				textEdit->append(QString("WorkingSetSize: %1").arg(readableSize(pmc.WorkingSetSize).c_str())); // in bytes
// 			}
// 			CloseHandle(hProcess);

			if (newLogLines()) {
				checkGamesCompleted();
			} else {
				// StarCraft is hang out. Restart it
				stopStarcraft();
				launchStarcraft();
			}
		} else {
			// no crash window, neither StarCraft. So, start StarCraft again
			launchStarcraft();
		}
	}
}

void NovaLauncher::printError(const QString & text)
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

unsigned int NovaLauncher::FileRead( std::istream & is, std::vector <char> & buff ) {
	is.read( &buff[0], buff.size() );
	return is.gcount();
}

unsigned int NovaLauncher::CountLines( const std::vector <char> & buff, int sz ) {
	int newlines = 0;
	const char * p = &buff[0];
	for ( int i = 0; i < sz; i++ ) {
		if ( p[i] == '\n' ) {
			newlines++;
		}
	}
	return newlines;
}

bool NovaLauncher::newLogLines()
{
	const int SZ = 1024 * 1024;
	std::vector <char> buff( SZ );
	std::ifstream ifs((char*)logFileName);
	int n = 0;
	while( int cc = FileRead( ifs, buff ) ) {
		n += CountLines( buff, cc );
	}
	logFileLinesLabel->setText(QString("%1").arg(n));

	if (lastLogFileLines != n) {
		lastLogFileLines = n;
		return true;
	} else {
		printError(QString("Same logFile lines: %1").arg(lastLogFileLines));
		return false;
	}
}

void NovaLauncher::checkGamesCompleted()
{
	// Check how many games completed
	std::ifstream logFile((char*)logFileName);
	int gamesCompleted = 0;
	if(logFile) {
		std::string fileLine;
		while(std::getline(logFile, fileLine)) {
			// look for text EvaluationCurrentState
			std::size_t found = fileLine.find("Frames:");
			if (found!=std::string::npos) {
				gamesCompleted++;
			}
		}
		logFile.close();
		numExperimentsLabel->setText(QString("%1 of %2").arg(gamesCompleted).arg(numGames->text()));

		// update time left
		if (gamesCompleted > 0) {
			int timeTaken = experimentsTime.elapsed(); // in milliseconds
			float percentDone = (float)gamesCompleted / numGames->text().toInt();
			int timeLeft = int((float)timeTaken * (1 / percentDone - 1));

			int hours = timeLeft / (1000 * 60 * 60);
			int	minutes = (timeLeft % (1000 * 60 * 60)) / (1000 * 60);
			int	seconds = ((timeLeft % (1000 * 60 * 60)) % (1000 * 60)) / 1000;

			timeToCompleteLabel->setText(QString("%1:%2:%3")
				.arg(hours, 2, 10, QChar('0'))
				.arg(minutes, 2, 10, QChar('0'))
				.arg(seconds, 2, 10, QChar('0')));
		}
		

		if (gamesCompleted >= numGames->text().toInt()) {
			// stop StarCraft
			stopStarcraft();

			// save log
			QString resultFileName = QString("%1_%2_%3_%4")
				.arg(searchAlgorithm[searchAlgorithmIndex].c_str())
				.arg(buildings[buildingsIndex].c_str())
				.arg(spacePartition[spacePartitionIndex].c_str())
				.arg(refreshIntervals[refreshIntervalsIndex]);

			if (searchAlgorithm[searchAlgorithmIndex].compare("ABCD") == 0) {
				resultFileName += QString("_%1_%2_%3")
					.arg(ABCD_depth[ABCD_depthIndex])
					.arg(ABCD_downsampling[ABCD_downsamplingIndex])
					.arg(ABCD_timeLimit[ABCD_timeLimitIndex]);
			}
			if (searchAlgorithm[searchAlgorithmIndex].compare("MCTSCD") == 0) {
				resultFileName += QString("_%1_%2_%3")
					.arg(MCTSCD_depth[MCTSCD_depthIndex])
					.arg(MCTSCD_iterations[MCTSCD_iterationsIndex])
					.arg(MCTSCD_simulationTime[MCTSCD_simulationTimeIndex]);
			}
			resultFileName += ".txt";
			//textEdit->append(resultFileName.c_str());
			CopyFileA(logFileName, resultFilePath + resultFileName.toStdString().c_str() , FALSE);

			// empty log
			std::ofstream resetFile((char*)logFileName);

			// update INI parameters and relaunch StarCraft
			if (nextConfiguration()) {
				experimentsTime.start();
				setINIconfig();
				launchStarcraft();
			} else {
				stopExperiments();
			}
		}
	}
}

BOOL CALLBACK NovaLauncher::searchCloseButton(HWND hwnd, LPARAM lParam)
{
	QTextEdit* textEdit = reinterpret_cast<QTextEdit*>(lParam);
	// should have a child with a button
	HWND ButtonHandle = FindWindowEx(hwnd, 0, TEXT("Button"), TEXT("Close the program"));
	if (ButtonHandle) {
		//textEdit->append("Clicking close crash window 3 times...");
		SendMessage(ButtonHandle, BM_CLICK, 0 , 0);
		SendMessage(ButtonHandle, BM_CLICK, 0 , 0);
		SendMessage(ButtonHandle, BM_CLICK, 0 , 0);
		//textEdit->append("StarCraft error window closed");
		return FALSE; // stop looking children
	}
	return TRUE; // must return TRUE; If return is FALSE it stops the recursion
}

bool NovaLauncher::launchStarcraft()
{
	// Start StarCraft through Chaosluncher 
	HWND chaosWindows = FindWindow(TEXT("TChaoslauncherForm"), NULL);
	if (!chaosWindows) {
		printError("Chaoslauncher not open (trying to start it)");
		// starting Chaoslauncher
		STARTUPINFOA si;
		PROCESS_INFORMATION pi;

		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);
		ZeroMemory( &pi, sizeof(pi) );
		// change current working directory to Chaoslauncher path
		chdir(chaosLauncherPath);
		// create the process
		if( !CreateProcessA( "Chaoslauncher.exe",NULL, 
			NULL, NULL, NULL, NULL, NULL, NULL, &si, &pi ) ) 
		{
			printError("Impossible to start Chaoslauncher");
			return FALSE;
		}
		// restore previous working directory
		chdir(currentPath);
		Sleep(500); // give time to lunch Chaos Launcher
		chaosWindows = FindWindow(TEXT("TChaoslauncherForm"), NULL);
	} 
	// Chaoslauncher is running
	HWND chaosPanel = FindWindowEx(chaosWindows, 0, TEXT("TPanel"), NULL);
	if (!chaosPanel) {
		printError("Chaoslauncher Panel doesn't exist");
		return FALSE;
	} else {
		HWND ButtonHandle = FindWindowEx(chaosPanel, 0, TEXT("TButton"), TEXT("Start"));
		if (!ButtonHandle) {
			printError("Chaoslauncher Start button doesn't exist");
			return FALSE;
		} else {
			SendMessage(ButtonHandle, BM_CLICK, 0 , 0);
			QDateTime dateTime = QDateTime::currentDateTime();
			QString dateTimeString = dateTime.toString();
			textEdit->append("StarCraft relaunched (" + dateTimeString + ")");
		}
	}

	return TRUE;
}

void NovaLauncher::stopStarcraft()
{
	HWND starcraftWindows = FindWindow(NULL, TEXT("Brood War"));
	if (starcraftWindows) {
		DWORD starcraftProcessId = 0;
		GetWindowThreadProcessId(starcraftWindows, &starcraftProcessId);
		if (!starcraftProcessId) {
			printError("Error finding StarCraft process ID");
		} else {
			HANDLE starcraftProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, starcraftProcessId);
			if (!starcraftProcess) {
				printError("Error opening StarCraft process");
			} else {
				TerminateProcess(starcraftProcess, 0);
				CloseHandle(starcraftProcess);
				textEdit->append("StarCraft process closed");
				Sleep(500); // give time to close StarCraft
			}
		}
	}
}

bool NovaLauncher::nextConfiguration()
{
	if (refreshIntervalsIndex < refreshIntervals.size()-1) {
		refreshIntervalsIndex++;
		return true;
	} else {
		refreshIntervalsIndex = 0;
	}
	if (spacePartitionIndex < spacePartition.size() - 1) {
		spacePartitionIndex++;
		return true;
	} else {
		spacePartitionIndex = 0;
	}
	if (buildingsIndex < buildings.size() - 1) {
		buildingsIndex++;
		return true;
	} else {
		buildingsIndex = 0;
	}

	if (searchAlgorithm[searchAlgorithmIndex].compare("ABCD") == 0) {
		if (ABCD_depthIndex < ABCD_depth.size() - 1) {
			ABCD_depthIndex++;
			return true;
		} else {
			ABCD_depthIndex = 0;
		}
		if (ABCD_downsamplingIndex < ABCD_downsampling.size() - 1) {
			ABCD_downsamplingIndex++;
			return true;
		} else {
			ABCD_downsamplingIndex = 0;
		}
		if (ABCD_timeLimitIndex < ABCD_timeLimit.size() - 1) {
			ABCD_timeLimitIndex++;
			return true;
		} else {
			ABCD_timeLimitIndex = 0;
		}
	}

	if (searchAlgorithm[searchAlgorithmIndex].compare("MCTSCD") == 0) {
		if (MCTSCD_depthIndex < MCTSCD_depth.size() - 1) {
			MCTSCD_depthIndex++;
			return true;
		} else {
			MCTSCD_depthIndex = 0;
		}
		if (MCTSCD_iterationsIndex < MCTSCD_iterations.size() - 1) {
			MCTSCD_iterationsIndex++;
			return true;
		} else {
			MCTSCD_iterationsIndex = 0;
		}
		if (MCTSCD_simulationTimeIndex < MCTSCD_simulationTime.size() - 1) {
			MCTSCD_simulationTimeIndex++;
			return true;
		} else {
			MCTSCD_simulationTimeIndex = 0;
		}
	}

	if (searchAlgorithmIndex < searchAlgorithm.size() - 1) {
		searchAlgorithmIndex++;
		return true;
	} else {
		searchAlgorithmIndex = 0;
	}

	return false;
}