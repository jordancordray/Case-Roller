#include "case.h"
#include <qforeach.h>

Case ::Case (){
    // populate items in the constructor for portability
    items.append({"Stone That Looks Like a Potato",
                  "You're 90% sure it's a rock... 90%.",
                  "Common"});

    items.append({"Bent Paperclip of Mystery",
                  "Always slightly bent, no matter how many times you fix it.",
                  "Common"});

    items.append({"Feather That Smells Like Rain",
                  "Soft, damp, and nostalgic.",
                  "Common"});

    items.append({"Expired Juice Box",
                  "The straw refuses to go in. The juice refuses to die.",
                  "Common"});

    items.append({"Pocket Hole",
                  "A small portable void. Handy for losing keys faster.",
                  "Common"});

    items.append({"Single Sock of Unknown Origin",
                  "It doesn't match anything — maybe that's the point.",
                  "Common"});

    items.append({"Magnet That Only Attracts Trouble",
                  "Stick it anywhere, and chaos follows.",
                  "Rare"});

    items.append({"Puzzle Piece That Fits Nowhere",
                  "It's from something, but no one knows what.",
                  "Rare"});

    items.append({"Lightbulb That Glows When You Lie",
                  "Keep it away from politicians and toddlers.",
                  "Rare"});

    items.append({"Sunglasses for the Moon",
                  "Designed for celestial style. Only slightly cursed.",
                  "Rare"});

    items.append({"Box That Contains a Smaller Box",
                  "No matter how deep you go, there's always one more.",
                  "Epic"});

    items.append({"Scroll of Forgotten Passwords",
                  "The answers to everything... but none of them work anymore.",
                  "Epic"});

    items.append({"Cookie That Regenerates",
                  "Take a bite, blink, and it's whole again. Calories not included.",
                  "Epic"});

    items.append({"Hourglass of Lost Time",
                  "Turn it, and you'll forget what you were doing — but feel productive anyway.",
                  "Legendary"});

    items.append({"Mind Mirror",
                  "When you look into it, it looks back. And sometimes, it's disappointed.",
                  "Legendary"});

    items.append({"Time-Snail Shell",
                  "Carry it in your pocket. Sometimes time slows down around you… sometimes it speeds up.",
                  "Mythic"});




    weights["Common"] = 60;
    weights["Rare"] = 25;
    weights["Epic"] = 10;
    weights["Legendary"] = 4;
    weights["Mythic"] = 1;

    colors["Common"] = "#326fa8";
    colors["Rare"] = "#7e19e3";
    colors["Epic"] = "#eb66ed";
    colors["Legendary"] = "#b31010";
    colors["Mythic"] = "#b09405";

    Case::addColorByRarity(colors, items);

}

void Case::addColorByRarity(const QMap<QString, QString> &colors, QList<caseItem> &items)
{
    for (caseItem &item : items)
    {
        QString color = colors[item.rarity];
        item.color = color;
    }
}

QString chooseRarityByWeight(const QMap<QString,int> &weights) {

    int total = 0;
    for (auto w : weights) total += w;
    if (total <= 0) return QString();

    int r = QRandomGenerator::global()->bounded(total);

    int cumulative = 0;
    for (auto it = weights.constBegin(); it != weights.constEnd(); ++it) {
        cumulative += it.value();
        if (r < cumulative) return it.key();
    }

    return weights.constBegin().key();
}

int chooseItemByRarity(QString rarity, QList<caseItem> &items){
    QList<int> pool;
    for (int i = 0; i < items.size(); i++){
        if (items[i].rarity == rarity){
            pool.append(i);
        }
    }
    int choice = pool.at(QRandomGenerator::global()->bounded(pool.size()));
    return choice;
}

caseItem Case::rollCase(){
    QString rarity = chooseRarityByWeight(weights);
    int index = chooseItemByRarity(rarity, items);
    return items[index];
}
