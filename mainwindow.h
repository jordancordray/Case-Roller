#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "canvasclient.h"
#include "case.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void GetCoursesFromCanvas();
    void SetItems(QList<Course> items){this->items = items;};


signals:
    void coursesFetched(const QList<Course>& items);
    void addNewItem(caseItem newItem);

private slots:
    void on_backButton_clicked();

private:
    void newTodoListItem(const Assignment &item);
    void viewItem(const caseItem &item);
    void newInventoryItem(caseItem &item);

    Ui::MainWindow *ui;
    QList<Course> items;
    Case *newCase = nullptr;
};
#endif // MAINWINDOW_H
