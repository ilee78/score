// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <QString>

#include "CurvePointInspectorFactory.hpp"
#include "CurvePointInspectorWidget.hpp"
#include <Curve/Point/CurvePointModel.hpp>
namespace Automation
{
QWidget* PointInspectorFactory::makeWidget(
    const QList<const QObject*>& sourceElements,
    const iscore::DocumentContext& doc,
    QWidget* parent) const
{
  return new PointInspectorWidget{
      safe_cast<const Curve::PointModel&>(*sourceElements.first()), doc,
      parent};
}

bool PointInspectorFactory::matches(const QList<const QObject*>& objects) const
{
  return dynamic_cast<const Curve::PointModel*>(objects.first());
}
}
