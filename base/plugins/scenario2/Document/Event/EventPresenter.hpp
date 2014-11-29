#pragma once
#include <QNamedObject>

class EventModel;
class EventView;
class EventPresenter : public QNamedObject
{
	Q_OBJECT

	public:
		EventPresenter(EventModel* model, EventView* view, QObject* parent);
		virtual ~EventPresenter();

		int id() const;

	private:
		EventModel* m_model{};
		EventView* m_view{};
};

