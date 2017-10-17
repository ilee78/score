// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <Scenario/Document/Interval/IntervalModel.hpp>
#include <Scenario/Document/Event/EventModel.hpp>
#include <Scenario/Document/TimeSync/TimeSyncModel.hpp>
#include <algorithm>
#include <score/tools/std/Optional.hpp>
#include <qnamespace.h>
#include <tuple>

#include "Loop/LoopProcessMetadata.hpp"
#include "LoopProcessModel.hpp"
#include <ossia/detail/algorithms.hpp>
#include <Process/Process.hpp>
#include <Process/Style/ScenarioStyle.hpp>
#include <Scenario/Document/BaseScenario/BaseScenarioContainer.hpp>
#include <Scenario/Document/Interval/IntervalDurations.hpp>
#include <Scenario/Document/State/StateModel.hpp>
#include <score/document/DocumentInterface.hpp>
#include <score/model/ModelMetadata.hpp>
#include <score/model/Skin.hpp>
#include <score/serialization/VisitorCommon.hpp>
#include <score/model/Identifier.hpp>

namespace Loop
{

std::vector<Process::Port*> ProcessModel::inlets() const
{
  return {m_ports[0], m_ports[2], m_ports[4]};
}

std::vector<Process::Port*> ProcessModel::outlets() const
{
  return {m_ports[1], m_ports[3], m_ports[5]};
}


ProcessModel::ProcessModel(
    const TimeVal& duration,
    const Id<Process::ProcessModel>& id,
    QObject* parent)
    : Process::ProcessModel{duration, id,
                            Metadata<ObjectKey_k, ProcessModel>::get(), parent}
    , Scenario::BaseScenarioContainer{this}
{
  Scenario::IntervalDurations::Algorithms::changeAllDurations(
      interval(), duration);
  endEvent().setDate(duration);
  endTimeSync().setDate(duration);

  const double height = 0.5;
  interval().setHeightPercentage(height);
  interval().metadata().setName("pattern");
  BaseScenarioContainer::startState().setHeightPercentage(height);
  BaseScenarioContainer::endState().setHeightPercentage(height);
  BaseScenarioContainer::startEvent().setExtent({height, 0.2});
  BaseScenarioContainer::endEvent().setExtent({height, 0.2});
  BaseScenarioContainer::startTimeSync().setExtent({height, 1});
  BaseScenarioContainer::endTimeSync().setExtent({height, 1});

  metadata().setInstanceName(*this);

  { auto p = new Process::Port{Id<Process::Port>{0}, this};
    p->type = Process::PortType::Audio; p->propagate = true; p->outlet = false; m_ports.push_back(p); }
  { auto p = new Process::Port{Id<Process::Port>{1}, this};
    p->type = Process::PortType::Audio; p->propagate = true; p->outlet = true; m_ports.push_back(p); }
  { auto p = new Process::Port{Id<Process::Port>{2}, this};
    p->type = Process::PortType::Message; p->propagate = true; p->outlet = false; m_ports.push_back(p); }
  { auto p = new Process::Port{Id<Process::Port>{3}, this};
    p->type = Process::PortType::Message; p->propagate = true; p->outlet = true; m_ports.push_back(p); }
  { auto p = new Process::Port{Id<Process::Port>{4}, this};
    p->type = Process::PortType::Midi; p->propagate = true; p->outlet = false; m_ports.push_back(p); }
  { auto p = new Process::Port{Id<Process::Port>{5}, this};
    p->type = Process::PortType::Midi; p->propagate = true; p->outlet = true; m_ports.push_back(p); }
}

ProcessModel::ProcessModel(
    const Loop::ProcessModel& source,
    const Id<Process::ProcessModel>& id,
    QObject* parent)
    : Process::ProcessModel{source, id,
                            Metadata<ObjectKey_k, ProcessModel>::get(), parent}
    , BaseScenarioContainer{source, this}
{
  metadata().setInstanceName(*this);
  for(const auto& port : source.m_ports)
  {
    m_ports.push_back(port->clone(this));
  }
}

ProcessModel::~ProcessModel()
{
  emit identified_object_destroying(this);
}

void ProcessModel::startExecution()
{
  interval().startExecution();
}

void ProcessModel::stopExecution()
{
  interval().stopExecution();
}

void ProcessModel::reset()
{
  interval().reset();
  startEvent().setStatus(Scenario::ExecutionStatus::Editing, *this);
  endEvent().setStatus(Scenario::ExecutionStatus::Editing, *this);
}

Selection ProcessModel::selectableChildren() const
{
  Selection s;

  ossia::for_each_in_tuple(elements(), [&](auto elt) { s.append(elt); });

  return s;
}

Selection ProcessModel::selectedChildren() const
{
  Selection s;

  ossia::for_each_in_tuple(elements(), [&](auto elt) {
    if (elt->selection.get())
      s.append(elt);
  });

  return s;
}

void ProcessModel::setSelection(const Selection& s) const
{
  ossia::for_each_in_tuple(elements(), [&](auto elt) {
    elt->selection.set(s.contains(elt)); // OPTIMIZEME
  });
}

const QVector<Id<Scenario::IntervalModel>> intervalsBeforeTimeSync(
    const ProcessModel& scen, const Id<Scenario::TimeSyncModel>& timeSyncId)
{
  if (timeSyncId == scen.endTimeSync().id())
  {
    return {scen.interval().id()};
  }
  return {};
}
}
