#pragma once
#include <QObject>

class Qsignal : public QObject
{
	Q_OBJECT

public:
	Qsignal()  {};
	void emitMapInfoChanged() { emit mapInfoChanged();};
	void emitGameStateChanged() { emit gameStateChanged();};

signals:
	void mapInfoChanged();
	void gameStateChanged();
};

