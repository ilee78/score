#include "InspectorPanelModel.hpp"

#include <core/document/DocumentModel.hpp>
#include <QDebug>

InspectorPanelModel::InspectorPanelModel(iscore::DocumentModel* parent) :
    iscore::PanelModelInterface {"InspectorPanelModel", nullptr}
// NOTE : here we declare parent after because else for some weird reason,
// "newItemInspected" is not found...
{
    this->setParent(parent);
}
/*
void InspectorPanelModel::newItemInspected(QObject* item)
{
    //TODO save the path to the item.
    emit setNewItem(item);
    m_lastInspectedObjects.push_back(item);
}

void InspectorPanelModel::lastItemInspected()
{
    if(m_lastInspectedObjects.size() > 1)
    {
        m_lastInspectedObjects.removeLast();
        emit setNewItem(m_lastInspectedObjects.back());
    }
}
*/
