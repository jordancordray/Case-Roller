#include "case.h"
#include "caseroller.h"
#include "ui_caseroller.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QScrollBar>
#include <QPropertyAnimation>
#include <QTimer>

caseRoller::caseRoller(caseItem *targetItem, QWidget *parent)
    : QWidget(parent), targetItem(targetItem)
    , ui(new Ui::caseRoller)
{
    ui->setupUi(this);
    scrollArea = new QScrollArea(this);
    scrollArea->viewport()->installEventFilter(this);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameStyle(QFrame::NoFrame);

    QWidget *stripWidget = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(stripWidget);
    layout->setSpacing(10);
    layout->setContentsMargins(0,0,0,0);
    scrollArea->setWidget(stripWidget);
    scrollArea->setWidgetResizable(true);


    populateRoll(*targetItem, layout);

    centerLine = new QFrame(scrollArea->viewport());      // parent = viewport -> stays fixed in view
    centerLine->setFrameShape(QFrame::VLine);
    centerLine->setFrameShadow(QFrame::Plain);
    centerLine->setLineWidth(0);                          // we'll use background color + fixed width
    centerLine->setFixedWidth(3);                         // visual width of the indicator
    centerLine->setStyleSheet("background-color: red;");  // visible color — change as you like
    centerLine->setGeometry(195, 0, 10, 120);
    centerLine->raise();
    centerLine->show();

}

void caseRoller::populateRoll(caseItem newItem, QHBoxLayout* layout)
{
    Case *newCase = new Case();

    for (int i = 0; i < 40; ++i) {
        if (i == 32)
        {
            QLabel *item = new QLabel(QString("%1").arg(newItem.name));
            item->setFixedSize(120, 120);
            item->setWordWrap(true);
            item->setAlignment(Qt::AlignCenter);
            item->setStyleSheet(QString("background-color: %1; border-radius:8px;").arg(newItem.color));
            layout->addWidget(item);
            targetLabel = item;
        }
        else
        {
            caseItem *thing = new caseItem(newCase->rollCase());

            QLabel *item = new QLabel(QString("%1").arg(thing->name));
            item->setFixedSize(120, 120);
            item->setWordWrap(true);
            item->setAlignment(Qt::AlignCenter);
            item->setStyleSheet(QString("background-color: %1; border-radius:8px;").arg(thing->color));
            layout->addWidget(item);

            delete thing;
        }
    }


    delete newCase;
}

void caseRoller::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    int itemCenter = targetLabel->x() + targetLabel->width()/2;
    int viewportCenter = scrollArea->viewport()->width()/2;
    int targetScroll = itemCenter - viewportCenter + 100;
    targetScroll = qBound(scrollArea->horizontalScrollBar()->minimum(),
                          targetScroll,
                          scrollArea->horizontalScrollBar()->maximum());

    // Animate the scrollbar
    QPropertyAnimation *anim = new QPropertyAnimation(scrollArea->horizontalScrollBar(), "value");
    anim->setStartValue(scrollArea->horizontalScrollBar()->value());
    anim->setEndValue(targetScroll);
    anim->setDuration(2000);
    anim->setEasingCurve(QEasingCurve::OutCubic);

    connect(anim, &QPropertyAnimation::finished, this, [this]() {
        qDebug()<< "Waiting 2 seconds";
        QTimer::singleShot(1300, this, [this]() {
            qDebug()<< "2 seconds waited";
            emit caserollDone();
        });
    });

    qDebug()<< "Animation started";
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

bool caseRoller::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == scrollArea->viewport()) {
        if (event->type() == QEvent::Wheel ||
            event->type() == QEvent::Gesture ||
            event->type() == QEvent::TouchBegin) {
            return true; // block user scrolling
        }
    }
    return QWidget::eventFilter(obj, event); // let other events through
}

caseRoller::~caseRoller()
{
    delete ui;
}
