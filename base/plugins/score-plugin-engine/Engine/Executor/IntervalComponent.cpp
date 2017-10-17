// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <ossia/editor/state/state_element.hpp>
#include <ossia/editor/scenario/time_interval.hpp>
#include <Automation/AutomationModel.hpp>
#include <Engine/OSSIA2score.hpp>
#include <Engine/score2OSSIA.hpp>
#include <Scenario/Document/Interval/IntervalModel.hpp>
#include <Scenario/Process/ScenarioModel.hpp>
#include <utility>

#include "IntervalComponent.hpp"
#include "Loop/LoopProcessModel.hpp"
#include "ScenarioComponent.hpp"
#include <ossia/editor/scenario/time_value.hpp>
#include <Engine/Executor/DocumentPlugin.hpp>
#include <Engine/Executor/ExecutorContext.hpp>
#include <Engine/Executor/ProcessComponent.hpp>
#include <Process/Process.hpp>
#include <Process/TimeValue.hpp>
#include <Scenario/Document/Interval/IntervalDurations.hpp>
#include <score/document/DocumentContext.hpp>
#include <score/model/Identifier.hpp>
#include <score/tools/IdentifierGeneration.hpp>

namespace Engine
{
namespace Execution
{
IntervalComponentBase::IntervalComponentBase(
    Scenario::IntervalModel& score_cst,
    const Context& ctx,
    const Id<score::Component>& id,
    QObject* parent)
    : Scenario::GenericIntervalComponent<const Context>{score_cst, ctx, id, "Executor::Interval", nullptr}
{
  con(interval().duration,
      &Scenario::IntervalDurations::executionSpeedChanged, this,
      [&](double sp) { m_ossia_interval->set_speed(sp); });

  con(interval().duration,
      &Scenario::IntervalDurations::defaultDurationChanged, this,
      [&](TimeVal sp) {
    system().executionQueue.enqueue([t=ctx.time(sp),cst = m_ossia_interval]
      { cst->set_nominal_duration(t); });
  });

  con(interval().duration,
      &Scenario::IntervalDurations::minDurationChanged, this,
      [&](TimeVal sp) {
    system().executionQueue.enqueue([t=ctx.time(sp),cst = m_ossia_interval]
      { cst->set_min_duration(t); });
  });

  con(interval().duration,
      &Scenario::IntervalDurations::maxDurationChanged, this,
      [&](TimeVal sp) {
    system().executionQueue.enqueue([t=ctx.time(sp),cst = m_ossia_interval]
      { cst->set_max_duration(t); });
  });
}

IntervalComponent::~IntervalComponent()
{
  if(m_ossia_interval)
    m_ossia_interval->set_callback(ossia::time_interval::exec_callback{});

  for(auto& proc : m_processes)
    proc.second->cleanup();
  executionStopped();
}

void IntervalComponent::init()
{
    init_hierarchy();
}

void IntervalComponent::cleanup()
{
  if(m_ossia_interval)
    m_ossia_interval->set_callback(ossia::time_interval::exec_callback{});
  for(auto& proc : m_processes)
    proc.second->cleanup();

  system().plugin.execGraph->remove_node(m_ossia_interval->node);
  clear();
  m_processes.clear();
  m_ossia_interval.reset();
}

interval_duration_data IntervalComponentBase::makeDurations() const
{
  return {
        context().time(interval().duration.defaultDuration()),
        context().time(interval().duration.minDuration()),
        context().time(interval().duration.maxDuration()),
        interval().duration.executionSpeed()
  };
}

void IntervalComponent::onSetup(
    std::shared_ptr<ossia::time_interval> ossia_cst,
    interval_duration_data dur,
    bool parent_is_base_scenario)
{
  m_ossia_interval = ossia_cst;

  m_ossia_interval->set_min_duration(dur.minDuration);
  m_ossia_interval->set_max_duration(dur.maxDuration);
  m_ossia_interval->set_speed(dur.speed);

  // BaseScenario needs a special callback. It is given in DefaultClockManager.
  if (!parent_is_base_scenario)
  {
    m_ossia_interval->set_stateless_callback(
        [&](double position,
            ossia::time_value date,
            const ossia::state_element& state) {
      auto currentTime = this->context().reverseTime(date);

      auto& cstdur = interval().duration;
      const auto& maxdur = cstdur.maxDuration();

      if (!maxdur.isInfinite())
        cstdur.setPlayPercentage(currentTime / cstdur.maxDuration());
      else
        cstdur.setPlayPercentage(currentTime / cstdur.defaultDuration());
    });
  }

  // set-up the interval ports
  system().plugin.inlets.insert({interval().ports()[0], std::make_pair(ossia_cst->node, ossia_cst->node->inputs()[0])});
  system().plugin.outlets.insert({interval().ports()[1], std::make_pair(ossia_cst->node, ossia_cst->node->outputs()[0])});
  system().plugin.inlets.insert({interval().ports()[2], std::make_pair(ossia_cst->node, ossia_cst->node->inputs()[1])});
  system().plugin.outlets.insert({interval().ports()[3], std::make_pair(ossia_cst->node, ossia_cst->node->outputs()[1])});
  system().plugin.inlets.insert({interval().ports()[4], std::make_pair(ossia_cst->node, ossia_cst->node->inputs()[2])});
  system().plugin.outlets.insert({interval().ports()[5], std::make_pair(ossia_cst->node, ossia_cst->node->outputs()[2])});

  system().plugin.execGraph->add_node(ossia_cst->node);

  init();
}

std::shared_ptr<ossia::time_interval>
IntervalComponentBase::OSSIAInterval() const
{
  return m_ossia_interval;
}

Scenario::IntervalModel& IntervalComponentBase::scoreInterval() const
{
  return interval();
}

void IntervalComponentBase::pause()
{
  m_ossia_interval->pause();
}

void IntervalComponentBase::resume()
{
  m_ossia_interval->resume();
}

void IntervalComponentBase::stop()
{
  m_ossia_interval->stop();
  auto st = m_ossia_interval->get_end_event().get_state();
  st.launch();

  for (auto& process : m_processes)
  {
    process.second->stop();
  }
  interval().reset();

  executionStopped();
  interval().duration.setPlayPercentage(0);
}

void IntervalComponentBase::executionStarted()
{
  interval().duration.setPlayPercentage(0);
  interval().executionStarted();
  for (Process::ProcessModel& proc : interval().processes)
  {
    proc.startExecution();
  }
}

void IntervalComponentBase::executionStopped()
{
  interval().executionStopped();
  for (Process::ProcessModel& proc : interval().processes)
  {
    proc.stopExecution();
  }
}

ProcessComponent* IntervalComponentBase::make(
    const Id<score::Component> & id,
    ProcessComponentFactory& fac,
    Process::ProcessModel &proc)
{
  try
  {
    auto& self = static_cast<Engine::Execution::IntervalComponent&>(*this);
    const Engine::Execution::Context& ctx = system();
    auto plug = fac.make(self, proc, ctx, id, nullptr);
    if (plug)
    {
      m_processes.emplace(proc.id(), plug);

      const auto& outlets = proc.outlets();
      std::vector<int> propagated_outlets;
      for(std::size_t i = 0; i < outlets.size(); i++)
      {
        if(outlets[i]->propagate && outlets[i]->cables().empty() && outlets[i]->address() == State::AddressAccessor{})
          propagated_outlets.push_back(i);
      }

      system().executionQueue.enqueue(
            [=,cst=m_ossia_interval,oproc=plug->OSSIAProcessPtr()] {
        if(oproc)
        {
          cst->add_time_process(oproc);
          if(oproc->node)
          {
            ossia::graph_node& n = *oproc->node;
            for(int propagated : propagated_outlets)
            {
              const auto& outlet = n.outputs()[propagated]->data;
              ossia::inlet_ptr cst_inlet;
              if(outlet.target<ossia::audio_port>()) cst_inlet = cst->node->inputs()[0];
              else if(outlet.target<ossia::value_port>()) cst_inlet = cst->node->inputs()[1];
              else if(outlet.target<ossia::midi_port>()) cst_inlet = cst->node->inputs()[2];
              if(cst_inlet)
              {
                auto cable = ossia::make_edge(
                      ossia::immediate_glutton_connection{}
                      , n.outputs()[propagated]
                      , cst_inlet
                      , oproc->node
                      , cst->node);
                plug->system().plugin.execGraph->connect(cable);
              }
            }
          }
        }

      });
    }
    return plug.get();
  }
  catch (const std::exception& e)
  {
    qDebug() << "Error while creating a process: " << e.what();
  }
  catch (...)
  {
    qDebug() << "Error while creating a process";
  }
  return nullptr;
}

std::function<void ()> IntervalComponentBase::removing(
    const Process::ProcessModel& e,
    ProcessComponent& c)
{
  auto it = m_processes.find(e.id());
  if(it != m_processes.end())
  {
    system().executionQueue.enqueue([cstr=m_ossia_interval,proc=c.OSSIAProcessPtr()] {
      cstr->remove_time_process(proc.get());
    });

    c.cleanup();

    return [=] { m_processes.erase(it); };
  }
  return {};
}

}
}
