#pragma once
#if defined(HAS_VST2)
#include <Dataflow/PortItem.hpp>
#include <Media/Effect/VST/VSTEffectModel.hpp>

#include <verdigris>

namespace Process
{
struct Context;
}
namespace Media::VST
{

class VSTControlInlet final : public Process::Inlet
{
  W_OBJECT(VSTControlInlet)
  SCORE_SERIALIZE_FRIENDS
public:
  MODEL_METADATA_IMPL(VSTControlInlet)
  VSTControlInlet(Id<Process::Port> c, QObject* parent) : Inlet{std::move(c), parent} { }

  VSTControlInlet(DataStream::Deserializer& vis, QObject* parent) : Inlet{vis, parent}
  {
    vis.writeTo(*this);
  }
  VSTControlInlet(JSONObject::Deserializer& vis, QObject* parent) : Inlet{vis, parent}
  {
    vis.writeTo(*this);
  }
  VSTControlInlet(DataStream::Deserializer&& vis, QObject* parent) : Inlet{vis, parent}
  {
    vis.writeTo(*this);
  }
  VSTControlInlet(JSONObject::Deserializer&& vis, QObject* parent) : Inlet{vis, parent}
  {
    vis.writeTo(*this);
  }

  VIRTUAL_CONSTEXPR Process::PortType type() const noexcept override
  {
    return Process::PortType::Message;
  }

  int fxNum{};

  float value() const { return m_value; }
  void setValue(float v)
  {
    if (v != m_value)
    {
      m_value = v;
      valueChanged(v);
    }
  }

public:
  void valueChanged(float arg_1) W_SIGNAL(valueChanged, arg_1);

private:
  float m_value{};
};

struct VSTControlPortItem final : public Dataflow::AutomatablePortItem
{
public:
  using Dataflow::AutomatablePortItem::AutomatablePortItem;

  void setupMenu(QMenu& menu, const score::DocumentContext& ctx) override;
  bool on_createAutomation(
      Scenario::IntervalModel& cst,
      std::function<void(score::Command*)> macro,
      const score::DocumentContext& ctx) override;
};

class VSTControlPortFactory final : public Process::PortFactory
{
public:
  ~VSTControlPortFactory() override;

  UuidKey<Process::Port> concreteKey() const noexcept override;

  Process::Port* load(const VisitorVariant& vis, QObject* parent) override;

  Dataflow::PortItem* makeItem(
      Process::Inlet& port,
      const Process::Context& ctx,
      QGraphicsItem* parent,
      QObject* context) override;

  Dataflow::PortItem* makeItem(
      Process::Outlet& port,
      const Process::Context& ctx,
      QGraphicsItem* parent,
      QObject* context) override;

  void setupInletInspector(
      const Process::Inlet& port,
      const score::DocumentContext& ctx,
      QWidget* parent,
      Inspector::Layout& lay,
      QObject* context) override;
};
}
#endif
