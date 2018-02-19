#include "novalauncher.h"
#include <QtWidgets/QApplication>

std::string configPath("c:/StarCraft/bwapi-data/AI/Nova.ini");

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	NovaLauncher w;
	w.show();
	a.setWindowIcon(QIcon(":/NovaLauncher/Resources/Nova.ico"));
	return a.exec();
}
