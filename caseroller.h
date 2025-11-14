#ifndef CASEROLLER_H
#define CASEROLLER_H

#include "case.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QWidget>

namespace Ui {
class caseRoller;
}

class caseRoller : public QWidget
{
    Q_OBJECT

public:
    explicit caseRoller(caseItem *targetItem, QWidget *parent = nullptr);
    ~caseRoller();

    caseItem getTarget(){return *targetItem; } ;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

signals:
    void caserollDone();

private:
    void showEvent(QShowEvent *event) override;
    void populateRoll(caseItem newItem, QHBoxLayout* layout);

    Ui::caseRoller *ui;
    QScrollArea *scrollArea;
    QFrame *centerLine;
    caseItem *targetItem;
    QLabel *targetLabel;
};


#endif // CASEROLLER_H
