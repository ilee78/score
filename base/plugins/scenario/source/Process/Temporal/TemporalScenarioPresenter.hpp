#pragma once
#include "ProcessInterface/ProcessPresenterInterface.hpp"
#include <tools/SettableIdentifier.hpp>
#include <Document/Event/EventData.hpp>

namespace iscore
{
    class SerializableCommand;
}
class ProcessViewModelInterface;
class ProcessViewInterface;

class AbstractConstraintViewModel;
class TemporalConstraintViewModel;
class TemporalConstraintPresenter;
class EventPresenter;
class TemporalScenarioViewModel;
class TemporalScenarioView;
class EventModel;
class TimeNodeModel;
class TimeNodePresenter;
class ConstraintModel;
class ScenarioCommandManager;
class ScenarioViewInterface;
struct EventData;
struct ConstraintData;

class TemporalScenarioPresenter : public ProcessPresenterInterface
{
        Q_OBJECT

        friend class ScenarioCommandManager;
        friend class ScenarioViewInterface;

    public:
        TemporalScenarioPresenter(ProcessViewModelInterface* model,
                                  ProcessViewInterface* view,
                                  QObject* parent);
        virtual ~TemporalScenarioPresenter();


        virtual id_type<ProcessViewModelInterface> viewModelId() const override;
        virtual id_type<ProcessSharedModelInterface> modelId() const override;

        virtual void setWidth(int width) override;
        virtual void setHeight(int height) override;
        virtual void putToFront() override;
        virtual void putBehind() override;

        virtual void parentGeometryChanged() override;

        virtual void on_horizontalZoomChanged(int val) override;

        long millisecPerPixel() const;

    signals:
        void linesExtremityScaled(int, int);

    public slots:
        // Model -> view
        void on_eventCreated(id_type<EventModel> eventId);
        void on_eventDeleted(id_type<EventModel> eventId);

        void on_timeNodeCreated(id_type<TimeNodeModel> timeNodeId);
        void on_timeNodeDeleted(id_type<TimeNodeModel> timeNodeId);

        void on_constraintCreated(id_type<AbstractConstraintViewModel> constraintId);
        void on_constraintViewModelRemoved(id_type<AbstractConstraintViewModel> constraintId);

        // View -> Presenter
        void on_scenarioPressed();

        void on_askUpdate();

    private slots:
        void addTimeNodeToEvent(id_type<EventModel> eventId, id_type<TimeNodeModel> timeNodeId);

    protected:
        // TODO faire passer l'abstract et utiliser des free functions de cast
        std::vector<TemporalConstraintPresenter*> m_constraints;
        std::vector<EventPresenter*> m_events;
        std::vector<TimeNodePresenter*> m_timeNodes;

        // Necessary for the real-time creation / moving of elements
        bool ongoingCommand();

        int m_horizontalZoomSliderVal {};
        double m_millisecPerPixel {1};

        TemporalScenarioViewModel* m_viewModel;
        TemporalScenarioView* m_view;

        EventData m_lastData {};

    private:
        void on_eventCreated_impl(EventModel* event_model);
        void on_constraintCreated_impl(TemporalConstraintViewModel* constraint_view_model);
        void on_timeNodeCreated_impl(TimeNodeModel* timeNode_model);


        ScenarioCommandManager* m_cmdManager;
        ScenarioViewInterface* m_viewInterface;

};
