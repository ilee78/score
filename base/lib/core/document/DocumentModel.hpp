#pragma once
#include <tools/NamedObject.hpp>
#include <core/interface/selection/Selection.hpp>
#include <set>

namespace iscore
{
    class DocumentDelegateModelInterface;
    class PanelModelInterface;
    /**
     * @brief The DocumentDelegateModelInterface class
     *
     * Drawbridge between the application and a model given by a plugin.
     *
     * TODO this is now eligible to be an identified object.
     */
    class DocumentModel : public NamedObject
    {
            Q_OBJECT
        public:
            DocumentModel(QObject* parent);

            void setModelDelegate(DocumentDelegateModelInterface* m);
            DocumentDelegateModelInterface* modelDelegate() const
            {
                return m_model;
            }

            void addPanel(PanelModelInterface* m)
            {
                m_panelModels.append(m);
            }

            const QList<PanelModelInterface*>& panels() const
            {
                return m_panelModels;
            }

            // Returns a Panel by name.
            PanelModelInterface* panel(QString name) const;

        public slots:
            void setNewSelection(Selection);

        private:
            DocumentDelegateModelInterface* m_model {};
            QList<PanelModelInterface*> m_panelModels;
    };
}
