#include "fancheng-scenario.h"
#include "scenario.h"
#include "skill.h"
#include "clientplayer.h"
#include "client.h"
#include "engine.h"
#include "standard.h"

class Guagu: public TriggerSkill {
public:
    Guagu(): TriggerSkill("guagu") {
        events << Damage;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, Room *, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.to->isLord() ){
            int x = damage.damage;
            Room *room = player->getRoom();

            RecoverStruct recover;
            recover.card = damage.card;
            recover.who = damage.from;
            recover.recover = x * 2;
            room->recover(damage.to, recover);
            player->drawCards(x);
        }

        return false;
    }
};

DujiangCard::DujiangCard() {
    target_fixed = true;
}

void DujiangCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
    LogMessage log;
    log.type = "#InvokeSkill";
    log.from = source;
    log.arg = "dujiang";
    room->sendLog(log);

    room->changeHero(source, "shenlvmeng", false);
    room->setTag("Dujiang", true);
}

class DujiangViewAsSkill: public ViewAsSkill {
public:
    DujiangViewAsSkill(): ViewAsSkill("dujiang") {
        frequency = Limited;
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@dujiang-card";
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        return selected.length() < 2 && to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() != 2)
            return false;

        DujiangCard *card = new DujiangCard;
        card->addSubcards(cards);

        return card;
    }
};

class Dujiang: public PhaseChangeSkill {
public:
    Dujiang(): PhaseChangeSkill("dujiang") {
        view_as_skill = new DujiangViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target) && target->getGeneralName() != "shenlvmeng";
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() == Player::Start) {
            if (target->getEquips().length() < 2)
                return false;

            Room *room = target->getRoom();
            room->askForUseCard(target, "@dujiang-card", "@@dujiang", -1, Card::MethodDiscard);
        }

        return false;
    }
};

FloodCard::FloodCard() {
    target_fixed = true;
}

void FloodCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
    room->setTag("Flood", true);

    room->setPlayerFlag(source, "flood");

    QList<ServerPlayer *> players = room->getOtherPlayers(source);
    foreach (ServerPlayer *player, players) {
        if (player->getRoleEnum() == Player::Rebel)
            room->cardEffect(this, source, player);
    }
}

void FloodCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->throwAllEquips();

    Room *room = effect.to->getRoom();
    if (!room->askForDiscard(effect.to, "flood", 2, 2, true)) {
        DamageStruct damage;
        damage.from = effect.from;
        damage.to = effect.to;

        room->damage(damage);
    }
}

class Flood: public ViewAsSkill {
public:
    Flood(): ViewAsSkill("flood") {
        frequency = Limited;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasFlag("flood");
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        return selected.length() < 3 && !to_select->isEquipped() && to_select->isBlack();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() != 3)
            return NULL;

        FloodCard *card = new FloodCard;
        card->addSubcards(cards);

        return card;
    }
};

TaichenFightCard::TaichenFightCard() {
    target_fixed = true;
}

void TaichenFightCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    room->loseHp(source);

    if (source->isAlive()) {
        Duel *duel = new Duel(Card::NoSuitNoColor, 0);
        duel->setSkillName("taichenfight");
        duel->setCancelable(false);

        room->setPlayerMark(source, "WushuangTarget", 1);
        CardUseStruct use;
        use.from = source;
        use.to << room->getLord();
        use.card = duel;
        room->useCard(use);
        room->setPlayerMark(source, "WushuangTarget", 0);
    }
}

class TaichenFight: public ZeroCardViewAsSkill {
public:
    TaichenFight(): ZeroCardViewAsSkill("taichenfight") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        if (player->hasUsed("TaichenFightCard"))
            return false;
        Duel *duel = new Duel(Card::NoSuitNoColor, 0);
        duel->deleteLater();
        if (player->isCardLimited(duel, Card::MethodUse)) {
            return false;
        }
        return true;
    }

    virtual const Card *viewAs() const{
        return new TaichenFightCard;
    }

    virtual int getEffectIndex(const ServerPlayer *player, const Card *card) const{
        if (card->isKindOf("Duel"))
            return -2;
        else
            return -1;
    }
};

class Xiansheng: public PhaseChangeSkill {
public:
    Xiansheng(): PhaseChangeSkill("xiansheng") {
        frequency = Limited;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target) && target->getGeneralName() == "guanyu" && target->getHp() <= 2;
    }

    virtual bool onPhaseChange(ServerPlayer *guanyu) const{
        if (guanyu->getPhase() == Player::Start) {
            Room *room = guanyu->getRoom();

            if (guanyu->askForSkillInvoke("xiansheng")) {
                guanyu->throwAllHandCardsAndEquips();
                room->changeHero(guanyu, "shenguanyu", true);
                room->drawCards(guanyu, 3);
            }
        }
        return false;
    }
};

ZhiyuanCard::ZhiyuanCard() {
    will_throw = false;
    handling_method = Card::MethodNone;
}

bool ZhiyuanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self && to_select->getRoleEnum() == Player::Rebel;
}

void ZhiyuanCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    targets.first()->obtainCard(this);
    room->setPlayerMark(source, "zhiyuan", source->getMark("zhiyuan") - 1);
}

class ZhiyuanViewAsSkill: public OneCardViewAsSkill {
public:
    ZhiyuanViewAsSkill(): OneCardViewAsSkill("zhiyuan") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("zhiyuan") > 0;
    }

    virtual bool viewFilter(const Card *to_select) const{
        return to_select->isKindOf("BasicCard");
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        ZhiyuanCard *zhiyuanCard = new ZhiyuanCard;
        zhiyuanCard->addSubcard(originalCard);
        return zhiyuanCard;
    }
};

class Zhiyuan: public PhaseChangeSkill {
public:
    Zhiyuan(): PhaseChangeSkill("zhiyuan") {
        view_as_skill = new ZhiyuanViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if (target->getPhase() == Player::Start) {
            Room *room = target->getRoom();
            room->setPlayerMark(target, "zhiyuan", 2);
        }

        return false;
    }
};

class FanchengRule: public ScenarioRule {
public:
    FanchengRule(Scenario *scenario)
        : ScenarioRule(scenario)
    {
        events << GameStart << BuryVictim;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        switch (event) {
        case GameStart: {
                player = room->getLord();
                room->installEquip(player, "chitu");
                room->installEquip(player, "blade");
                room->acquireSkill(player, "flood");
                room->acquireSkill(player, "xiansheng");

                ServerPlayer *sp_pangde = room->findPlayer("sp_pangde");
                room->acquireSkill(sp_pangde, "taichen_fight");

                ServerPlayer *huatuo = room->findPlayer("huatuo");
                room->installEquip(huatuo, "hualiu");
                room->acquireSkill(huatuo, "guagu");

                ServerPlayer *lvmeng = room->findPlayer("lvmeng");
                room->acquireSkill(lvmeng, "dujiang");

                ServerPlayer *caoren = room->findPlayer("caoren");
                room->installEquip(caoren, "renwang_shield");
                room->acquireSkill(caoren, "zhiyuan");


                break;
            }
        case BuryVictim: {
                DeathStruct death = data.value<DeathStruct>();
                if (player->getGeneralName() == "sp_pangde"
                    && death.damage && death.damage->from && death.damage->from->isLord()) {
                    death.damage = NULL;
                    data = QVariant::fromValue(death);
                }
                break;
            }
        default:
            break;
        }

        return false;
    }
};

FanchengScenario::FanchengScenario()
    : Scenario("fancheng")
{
    lord = "guanyu";
    loyalists << "huatuo";
    rebels << "caoren" << "sp_pangde" << "xuhuang";
    renegades << "lvmeng";

    rule = new FanchengRule(this);

    skills << new Guagu
           << new Dujiang
           << new Flood
           << new TaichenFight
           << new Xiansheng
           << new Zhiyuan;

    addMetaObject<DujiangCard>();
    addMetaObject<FloodCard>();
    addMetaObject<TaichenFightCard>();
    addMetaObject<ZhiyuanCard>();
}

void FanchengScenario::onTagSet(Room *room, const QString &key) const{
    if (key == "Flood") {
        ServerPlayer *xuhuang = room->findPlayer("xuhuang");
        if (xuhuang) {
            ServerPlayer *lord = room->getLord();
            room->setFixedDistance(xuhuang, lord, 1);
        }

        ServerPlayer *caoren = room->findPlayer("caoren");
        if (caoren)
            room->setPlayerProperty(caoren, "xueyi", -1);
    } else if (key == "Dujiang") {
        ServerPlayer *caoren = room->findPlayer("caoren");
        if (caoren)
            room->setPlayerProperty(caoren, "xueyi", 0);
    }
}
// FORMATTED