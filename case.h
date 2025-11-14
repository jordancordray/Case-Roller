#ifndef CASE_H
#define CASE_H

#include <QString>
#include <QList>
#include <QRandomGenerator>
#include <QMap>

struct caseItem{
    QString name;
    QString description;
    QString rarity;
    QString color;
};

class Case {public:
    Case();

    caseItem rollCase();

private:
    void addColorByRarity(const QMap<QString, QString> &colors, QList<caseItem> &items);

    QList<caseItem> items;
    QMap<QString,int> weights;
    QMap<QString, QString> colors;
};

#endif // CASE_H
