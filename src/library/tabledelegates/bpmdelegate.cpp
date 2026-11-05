#include "library/tabledelegates/bpmdelegate.h"

#include <QDoubleSpinBox>
#include <QItemEditorCreatorBase>
#include <QItemEditorFactory>

#include "moc_bpmdelegate.cpp"

// We override the typical QDoubleSpinBox editor by registering this class with
// a QItemEditorFactory for the BPMDelegate.
class BpmEditorCreator : public QItemEditorCreatorBase {
  public:
    BpmEditorCreator() {}
    ~BpmEditorCreator() override {
    }

    QWidget* createWidget(QWidget* parent) const override {
        QDoubleSpinBox* pBpmSpinbox = new QDoubleSpinBox(parent);
        pBpmSpinbox->setFrame(false);
        pBpmSpinbox->setMinimum(0);
        pBpmSpinbox->setMaximum(9999);
        pBpmSpinbox->setSingleStep(1e-3);
        pBpmSpinbox->setDecimals(8);
        pBpmSpinbox->setObjectName("LibraryBPMSpinBox");
        return pBpmSpinbox;
    }

    QByteArray valuePropertyName() const override {
        return QByteArray("value");
    }
};

BPMDelegate::BPMDelegate(QTableView* pTableView)
        : CheckboxDelegate(pTableView, QStringLiteral("LibraryBPMButton")) {
    // Register a custom QItemEditorFactory to override the default
    // QDoubleSpinBox editor.
    m_pFactory = new QItemEditorFactory();
    m_pFactory->registerEditor(QMetaType::Double, new BpmEditorCreator());
    setItemEditorFactory(m_pFactory);
}
