#ifndef _SPECIAL3V3_H
#define _SPECIAL3V3_H

#include "package.h"
#include "card.h"
#include "skill.h"

class HongyuanCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE HongyuanCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class HuanshiCard: public SkillCard {
    Q_OBJECT

public:
    Q_INVOKABLE HuanshiCard();

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class Special3v3Package: public Package {
    Q_OBJECT

public:
    Special3v3Package();
};

class New3v3CardPackage: public Package {
    Q_OBJECT

public:
    New3v3CardPackage();
};

#endif
// FORMATTED