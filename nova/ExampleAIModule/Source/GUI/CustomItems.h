#pragma once

#include <QtWidgets/QGraphicsEllipseItem>
#include "GUI/QtWindow.h"

class CustomEllipseItem : public QGraphicsEllipseItem
{
public:
	int regionId;
protected:
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
};

class CustomPolygonItem : public QGraphicsPolygonItem
{
public:
	int regionId;

	// overriding paint()
	void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget);
protected:
	// overriding mousePressEvent()
	void mousePressEvent(QGraphicsSceneMouseEvent *event);
};