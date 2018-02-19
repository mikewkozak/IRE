#include "CustomItems.h"

void CustomEllipseItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
// 	globalTextEdit->append( "Ellipse pressed!!" );
	novaGUI->regionIdSelected = regionId;
	novaGUI->updateUnitTable();
	novaGUI->mapScene->update(); // refresh map image
}

void CustomPolygonItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
// 	globalTextEdit->append( "Polygon pressed!!" );
	novaGUI->regionIdSelected = regionId;
	novaGUI->updateUnitTable();
	novaGUI->mapScene->update(); // refresh map image
}

void CustomPolygonItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	if (regionId == novaGUI->regionIdSelected) {
		painter->setBrush(QBrush(QColor(255, 100, 255)));
	} else {
		painter->setBrush(QBrush(Qt::white));
	}
	painter->drawPolygon(polygon());
}
