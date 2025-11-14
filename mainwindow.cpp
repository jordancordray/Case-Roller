#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "caseroller.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->centralwidget->setStyleSheet("background-color:#c7c7c7; color: #2b2929");
    ui->stackedWidget->setCurrentIndex(0);


    newCase = new Case();

    connect( this, &MainWindow::coursesFetched, this, [this](const QList<Course> &items){

        SetItems(items);
        ui->todolistWidget->clear();
        for (const Course item : this->items){
            if (item.m_assignments.size() != 0)
            {
                for (const Assignment &a : item.m_assignments)
                {
                    newTodoListItem(a);
                }

            }
        }
    });

    connect(this, &MainWindow::addNewItem, this, [this](caseItem newItem){
        newInventoryItem(newItem);
    });

    GetCoursesFromCanvas();
}

MainWindow::~MainWindow()
{
    delete newCase;
    delete ui;
}

void MainWindow::GetCoursesFromCanvas(){
    CanvasClient *client = new CanvasClient(QUrl("https://byui.instructure.com"), QStringLiteral("")); // Add canvas api bearer token here

    connect(client, &CanvasClient::coursesReady, [=](const QList<Course> &courses) {
        qDebug() << "Got" << courses.size() << "courses";
        for (const Course &c : courses) {
            qDebug() << "Fetching assignments for course" << c.id;
            client->fetchAssignmentsFromCourse(c);
        }
    });

    connect(client, &CanvasClient::assignmentsReady, [=](const QList<Course> &courses){
        emit coursesFetched(courses);
    });

    client->fetchCourses();

}

void MainWindow::newTodoListItem(const Assignment &item)
{
    QWidget *row = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(row);
    layout->setContentsMargins(5, 0, 5, 0);

    QLabel *nameLabel = new QLabel(item.name);

    QPushButton *statusButton = new QPushButton;
    statusButton->setFixedWidth(100);

    if (item.submitted)
    {
        statusButton->setText("Completed");
        statusButton->setStyleSheet(
            "background-color: lightgreen; color: black; font-weight: bold;"
            );


        connect(statusButton, &QPushButton::clicked, this, [=]() {
            qDebug() << "Clicked completed assignment:" << item.name;
            caseItem *newItem = new caseItem(newCase->rollCase());
            caseRoller *roller = new caseRoller(newItem);


            connect(roller, &caseRoller::caserollDone, this, [=](){
                qDebug() << "Case roll finished";
                statusButton->setText("Collected");
                statusButton->setStyleSheet(
                    "background-color: lightgrey; color: black; font-weight: bold;"
                    );
                statusButton->setEnabled(false);
                emit addNewItem(roller->getTarget());
                roller->deleteLater();
                delete newItem;
            });

            roller->show();


        });
    }
    else
    {
        statusButton->setText("Todo");
        statusButton->setEnabled(false);
        statusButton->setStyleSheet(
            "background-color: orange; color: black; font-weight: bold;"
            );
    }


    layout->addWidget(nameLabel);
    layout->addStretch();
    layout->addWidget(statusButton);


    QListWidgetItem *listItem = new QListWidgetItem(ui->todolistWidget);
    listItem->setSizeHint(row->sizeHint());
    ui->todolistWidget->addItem(listItem);
    ui->todolistWidget->setItemWidget(listItem, row);
}


void MainWindow::viewItem(const caseItem &item)
{
    ui->name_2->setText(item.name);
    ui->name_2->setStyleSheet(QString("color: %1").arg(item.color));
    ui->rarity_2->setText(item.rarity);
    ui->rarity_2->setStyleSheet(QString("color: %1").arg(item.color));
    ui->description->setText(item.description);
    ui->containerCard->setStyleSheet(QString("#containerCard{border:2px solid %1; border-radius: 10px;}").arg(item.color));
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::newInventoryItem(caseItem &item)
{
    QString color = item.color;

    QWidget *row = new QWidget;
    row->setStyleSheet("background-color: #c7c7c7; color: black;");
    QHBoxLayout *layout = new QHBoxLayout(row);
    layout->setContentsMargins(5, 0, 5, 0);


    QLabel *nameLabel = new QLabel(item.name);

    QPushButton *viewButton = new QPushButton;
    viewButton->setFixedWidth(100);
    viewButton->setText("View item");
    viewButton->setStyleSheet(QString("background-color: %1; border: 1px solid black; border-radius: 5px; margin: 3px;").arg(item.color));

    connect(viewButton, &QPushButton::clicked, this, [=]() {
        qDebug() << "Clicked view item:" << item.name;
        viewItem(item);
    });

    layout->addWidget(nameLabel);
    layout->addStretch();
    layout->addWidget(viewButton);

    QListWidgetItem *listItem = new QListWidgetItem(ui->inventoryWidget);
    listItem->setSizeHint(row->sizeHint());
    ui->inventoryWidget->addItem(listItem);
    ui->inventoryWidget->setItemWidget(listItem, row);
}

void MainWindow::on_backButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

