#include "bgm.h"
#include "skill.h"
#include "standard.h"
#include "clientplayer.h"
#include "engine.h"
#include "settings.h"

class ChongZhen: public TriggerSkill {
public:
    ChongZhen(): TriggerSkill("chongzhen") {
        events << CardResponded << TargetConfirmed;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        if (event == CardResponded) {
            CardResponseStruct resp = data.value<CardResponseStruct>();
            if (resp.m_card->getSkillName() == "longdan"
                && resp.m_who != NULL && !resp.m_who->isKongcheng()) {
                QVariant data = QVariant::fromValue((PlayerStar)resp.m_who);
                if (player->askForSkillInvoke(objectName(), data)) {
                    room->broadcastSkillInvoke("chongzhen", 1);
                    int card_id = room->askForCardChosen(player, resp.m_who, "h", objectName());
                    CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, player->objectName());
                    room->obtainCard(player, Sanguosha->getCard(card_id), reason, false);
                }
            }
        } else {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.from == player && use.card->getSkillName() == "longdan") {
                foreach (ServerPlayer *p, use.to) {
                    if (p->isKongcheng()) continue;
                    QVariant data = QVariant::fromValue((PlayerStar)p);
                    if (player->askForSkillInvoke(objectName(), data)) {
                        room->broadcastSkillInvoke("chongzhen", 2);
                        int card_id = room->askForCardChosen(player, p, "h", objectName());
                        CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, player->objectName());
                        room->obtainCard(player, Sanguosha->getCard(card_id), reason, false);
                    }
                }
            }
        }
        return false;
    }
};

LihunCard::LihunCard() {
    mute = true;
}

bool LihunCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->isMale() && to_select != Self;
}

void LihunCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    effect.from->turnOver();
    room->broadcastSkillInvoke("lihun", 1);

    DummyCard *dummy_card = new DummyCard;
    foreach (const Card *cd, effect.to->getHandcards())
        dummy_card->addSubcard(cd);
    if (!effect.to->isKongcheng()) {
        CardMoveReason reason(CardMoveReason::S_REASON_TRANSFER, effect.from->objectName(),
                              effect.to->objectName(), "lihun", QString());
        room->moveCardTo(dummy_card, effect.to, effect.from, Player::PlaceHand, reason, false);
    }
    effect.to->setFlags("LihunTarget");
    delete dummy_card;
}

class LihunSelect: public OneCardViewAsSkill {
public:
    LihunSelect(): OneCardViewAsSkill("lihun") {
    }

    virtual bool viewFilter(const Card *) const{
        return true;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("LihunCard");
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Card *card = new LihunCard;
        card->addSubcard(originalCard);
        return card;
    }
};

class Lihun: public TriggerSkill {
public:
    Lihun(): TriggerSkill("lihun") {
        events << EventPhaseEnd;
        view_as_skill = new LihunSelect;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->hasUsed("LihunCard");
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *diaochan, QVariant &) const{
        if (diaochan->getPhase() == Player::Play) {
            ServerPlayer *target = NULL;
            foreach (ServerPlayer *other, room->getOtherPlayers(diaochan)) {
                if (other->hasFlag("LihunTarget")) {
                    other->setFlags("-LihunTarget");
                    target = other;
                    break;
                }
            }

            if (!target || target->getHp() < 1 || diaochan->isNude())
                return false;

            room->broadcastSkillInvoke(objectName(), 2);
            DummyCard *to_goback;
            if (diaochan->getCardCount(true) <= target->getHp()) {
                to_goback = diaochan->isKongcheng() ? new DummyCard : diaochan->wholeHandCards();
                for (int i = 0; i < 4; i++)
                    if (diaochan->getEquip(i))
                        to_goback->addSubcard(diaochan->getEquip(i)->getEffectiveId());
            } else
                to_goback = (DummyCard *)room->askForExchange(diaochan, objectName(), target->getHp(), true, "LihunGoBack");

            CardMoveReason reason(CardMoveReason::S_REASON_GIVE, diaochan->objectName(),
                                  target->objectName(), objectName(), QString());
            reason.m_playerId = target->objectName();
            room->moveCardTo(to_goback, diaochan, target, Player::PlaceHand, reason);
            delete to_goback;
        }

        return false;
    }
};

class Kuiwei: public TriggerSkill {
public:
    Kuiwei(): TriggerSkill("kuiwei") {
        events << EventPhaseStart;
    }

    static int getWeaponCount(ServerPlayer *caoren) {
        int n = 0;
        foreach (ServerPlayer *p, caoren->getRoom()->getAlivePlayers())
            if (p->getWeapon())
                n++;

        return n;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target && target->isAlive()
               && (target->hasSkill(objectName()) || target->getMark("@kuiwei") > 0);
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *caoren, QVariant &) const{
        if (caoren->getPhase() == Player::Finish) {
            if (!caoren->hasSkill(objectName()))
                return false;
            if (!caoren->askForSkillInvoke(objectName()))
                return false;

            room->broadcastSkillInvoke(objectName());
            int n = getWeaponCount(caoren);
            caoren->drawCards(n + 2);
            caoren->turnOver();

            if (caoren->getMark("@kuiwei") == 0)
                caoren->gainMark("@kuiwei");
        } else if (caoren->getPhase() == Player::Draw) {
            if (caoren->getMark("@kuiwei") == 0)
                return false;

            int n = getWeaponCount(caoren);
            if (n > 0) {
                LogMessage log;
                log.type = "#KuiweiDiscard";
                log.from = caoren;
                log.arg = QString::number(n);
                log.arg2 = objectName();
                room->sendLog(log);

                room->askForDiscard(caoren, objectName(), n, n, false, true);
            }
            caoren->loseMark("@kuiwei");
        }
        return false;
    }
};

class Yanzheng: public OneCardViewAsSkill {
public:
    Yanzheng(): OneCardViewAsSkill("yanzheng") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "nullification" && player->getHandcardNum() > player->getHp();
    }

    virtual bool viewFilter(const Card *to_select) const{
        return to_select->isEquipped();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Card *ncard = new Nullification(originalCard->getSuit(), originalCard->getNumber());
        ncard->addSubcard(originalCard);
        ncard->setSkillName(objectName());
        return ncard;
    }

    virtual bool isEnabledAtNullification(const ServerPlayer *player) const{
        foreach(const Card *card, player->getHandcards()){
            if(card->objectName() == "nullification")
                return true;
        }

        return player->getHandcardNum() > player->getHp() && !player->getEquips().isEmpty();
    }
};

class Manjuan: public TriggerSkill {
public:
    Manjuan(): TriggerSkill("manjuan") {
        events << CardsMoving << CardDrawing;
        frequency = Frequent;
    }

    void doManjuan(ServerPlayer *sp_pangtong, int card_id) const{
        Room *room = sp_pangtong->getRoom();
        room->setPlayerFlag(sp_pangtong, "ManjuanInvoke");
        QList<int> DiscardPile = room->getDiscardPile(), toGainList;
        const Card *card = Sanguosha->getCard(card_id);
        foreach (int id, DiscardPile) {
            const Card *cd = Sanguosha->getCard(id);
            if (cd->getNumber() == card->getNumber())
                toGainList << id;
        }

        room->fillAG(toGainList, sp_pangtong);
        int id = room->askForAG(sp_pangtong, toGainList, false, objectName());
        if (id != -1)
            room->moveCardTo(Sanguosha->getCard(id), sp_pangtong, Player::PlaceHand, true);

        sp_pangtong->invoke("clearAG");
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *sp_pangtong, QVariant &data) const{
        if (sp_pangtong->hasFlag("ManjuanInvoke")) {
            room->setPlayerFlag(sp_pangtong, "-ManjuanInvoke");
            return false;
        }

        int card_id = -1;
        CardMoveReason reason(CardMoveReason::S_REASON_PUT, sp_pangtong->objectName(), "manjuan", QString());
        if (event == CardsMoving) {
            if (sp_pangtong->hasFlag("NoManjuan"))
                return false;
            CardsMoveOneTimeStar move = data.value<CardsMoveOneTimeStar>();
            if (move->to != sp_pangtong || move->to_place != Player::PlaceHand)
                return false;
            room->broadcastSkillInvoke(objectName());
            foreach (int card_id, move->card_ids){
                const Card *card = Sanguosha->getCard(card_id);
                room->moveCardTo(card, NULL, NULL, Player::DiscardPile, reason);
            }
        } else if (event == CardDrawing) {
            if (room->getTag("FirstRound").toBool())
                return false;
            if (sp_pangtong->hasFlag("NoManjuan"))
                return false;
            if (sp_pangtong->getMark("has_drawn_card") == 0)
                room->broadcastSkillInvoke(objectName());
            card_id = data.toInt();
            const Card *card = Sanguosha->getCard(card_id);
            room->moveCardTo(card, NULL, NULL, Player::DiscardPile, reason);
        }

        LogMessage log;
        log.type = "$ManjuanGot";
        log.from = sp_pangtong;
        if (event == CardsMoving) {
            CardsMoveOneTimeStar move = data.value<CardsMoveOneTimeStar>();
            log.card_str = Card::IdsToStrings(move->card_ids).join("+");
        }
        else
            log.card_str = Sanguosha->getCard(card_id)->toString();
        room->sendLog(log);

        if (sp_pangtong->getPhase() == Player::NotActive || !sp_pangtong->askForSkillInvoke(objectName(), data))
            return event != CardsMoving;

        if (event == CardsMoving) {
            CardsMoveOneTimeStar move = data.value<CardsMoveOneTimeStar>();
            foreach (int _card_id, move->card_ids)
                doManjuan(sp_pangtong, _card_id);
        } else {
            doManjuan(sp_pangtong, card_id);
        }
        return event != CardsMoving;
    }
};

class Zuixiang: public TriggerSkill {
public:
    Zuixiang(): TriggerSkill("zuixiang") {
        events << EventPhaseStart << CardEffected;
        frequency = Limited;

        type[Card::TypeBasic] = "BasicCard";
        type[Card::TypeTrick] = "TrickCard";
        type[Card::TypeEquip] = "EquipCard";
    }

    void doZuixiang(ServerPlayer *player) const{
        Room *room = player->getRoom();
        room->broadcastSkillInvoke("zuixiang");
        if (player->getPile("dream").isEmpty()) {
            room->broadcastInvoke("animate", "lightbox:$ZuixiangAnimate:3000");
            room->getThread()->delay(3000);
        }

        QList<Card::CardType> type_list;
        foreach (int card_id, player->getPile("dream")) {
            const Card *c = Sanguosha->getCard(card_id);
            type_list << c->getTypeId();
        }

        QList<int> ids = room->getNCards(3, false);
        CardsMoveStruct move;
        move.card_ids = ids;
        move.to = player;
        move.to_place = Player::PlaceTable;
        move.reason = CardMoveReason(CardMoveReason::S_REASON_TURNOVER, player->objectName(), "zuixiang", QString());
        room->moveCardsAtomic(move, true);

        room->getThread()->delay();
        room->getThread()->delay();

        player->addToPile("dream", ids, true);
        foreach (int id, ids) {
            const Card *cd = Sanguosha->getCard(id);
            if (type[cd->getTypeId()] == "EquipCard") {
                if (player->getMark("Equips_Nullified_to_Yourself") == 0)
                    room->setPlayerMark(player, "Equips_Nullified_to_Yourself", 1);
                if (player->getMark("Equips_of_Others_Nullified_to_You") == 0)
                    room->setPlayerMark(player, "Equips_of_Others_Nullified_to_You", 1);
            }
            if (!type_list.contains(cd->getTypeId())) {
                type_list << cd->getTypeId();
                room->setPlayerCardLock(player, type[cd->getTypeId()]);
            }
        }

        QList<int> zuixiang = player->getPile("dream");
        QSet<int> numbers;
        bool zuixiangDone = false;
        foreach (int id, zuixiang) {
            const Card *card = Sanguosha->getCard(id);
            if (numbers.contains(card->getNumber())) {
                zuixiangDone = true;
                break;
            }            
            numbers.insert(card->getNumber());
        }
        if (zuixiangDone) {
            player->addMark("zuixiangHasTrigger");
            room->setPlayerMark(player, "Equips_Nullified_to_Yourself", 0);
            room->setPlayerMark(player, "Equips_of_Others_Nullified_to_You", 0);
            room->setPlayerCardLock(player, "-BasicCard");
            room->setPlayerCardLock(player, "-TrickCard");
            room->setPlayerCardLock(player, "-EquipCard");

            LogMessage log;
            log.type = "$ZuixiangGot";
            log.from = player;

            log.card_str = Card::IdsToStrings(zuixiang).join("+");
            room->sendLog(log);

            room->setPlayerFlag(player, "NoManjuan");
            CardMoveReason reason(CardMoveReason::S_REASON_PUT, player->objectName(), QString(), "zuixiang", "");
            CardsMoveStruct move(zuixiang, player, Player::PlaceHand, reason);
            room->moveCardsAtomic(move, true);
            room->setPlayerFlag(player, "-NoManjuan");
        }
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *sp_pangtong, QVariant &data) const{
        QList<int> zuixiang = sp_pangtong->getPile("dream");

        if (event == EventPhaseStart && sp_pangtong->getMark("zuixiangHasTrigger") == 0) {
            if (sp_pangtong->getPhase() == Player::Start) {
                if (sp_pangtong->getMark("@sleep") > 0) {
                    if (!sp_pangtong->askForSkillInvoke(objectName()))
                        return false;
                    sp_pangtong->loseMark("@sleep", 1);
                    doZuixiang(sp_pangtong);
                } else
                    doZuixiang(sp_pangtong);
            }
        } else if (event == CardEffected) {
            if (zuixiang.isEmpty())
                return false;

            CardEffectStruct effect = data.value<CardEffectStruct>();
            QList<Card::CardType> type_list;
            foreach (int card_id, zuixiang) {
                const Card *c = Sanguosha->getCard(card_id);
                type_list << c->getTypeId();
            }

            if (type_list.contains(effect.card->getTypeId())) {
                LogMessage log;
                log.type = effect.from ? "#ZuiXiang1" : "#ZuiXiang2";
                log.from = effect.to;
                if(effect.from)
                    log.to << effect.from;
                log.arg = effect.card->objectName();
                log.arg2 = objectName();

                room->sendLog(log);
                room->broadcastSkillInvoke(objectName());
                return true;
            }
        }
        return false;
    }

private:
    QMap<Card::CardType, QString> type;
};

class ZuixiangClear: public TriggerSkill {
public:
    ZuixiangClear(): TriggerSkill("#zuixiang-clear") {
        events << EventLoseSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target && !target->hasSkill("zuixiang") && target->getPile("dream").length() > 0;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const{
        player->clearOnePrivatePile("dream");
        room->setPlayerMark(player, "equip_null", 0);
        room->setPlayerCardLock(player, "-BasicCard");
        room->setPlayerCardLock(player, "-TrickCard");
        room->setPlayerCardLock(player, "-EquipCard");
        return false;
    }
};

class Jie: public TriggerSkill {
public:
    Jie(): TriggerSkill("jie") {
        events << ConfirmDamage;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if (!damage.card || !damage.card->isKindOf("Slash") || !damage.card->isRed())
            return false;

        LogMessage log;
        log.type = "#Jie";
        log.from = player;
        log.to << damage.to;
        log.arg = QString::number(damage.damage);
        log.arg2 = QString::number(++damage.damage);
        room->sendLog(log);
        data = QVariant::fromValue(damage);

        return false;
    }
};

DaheCard::DaheCard() {
    will_throw = false;
    mute = true;
    handling_method = Card::MethodPindian;
}

bool DaheCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void DaheCard::use(Room *room, ServerPlayer *zhangfei, QList<ServerPlayer *> &targets) const{
    if (targets.first()->getGeneralName().contains("lvbu"))
        room->broadcastSkillInvoke("dahe", 2);
    else
        room->broadcastSkillInvoke("dahe", 1);
    zhangfei->pindian(targets.first(), "dahe", this);
}

class DaheViewAsSkill: public OneCardViewAsSkill {
public:
    DaheViewAsSkill(): OneCardViewAsSkill("dahe") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("DaheCard") && !player->isKongcheng();
    }

    virtual bool viewFilter(const Card *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Card *daheCard = new DaheCard;
        daheCard->addSubcard(originalCard);
        return daheCard;
    }
};

class Dahe: public TriggerSkill {
public:
    Dahe() :TriggerSkill("dahe") {
        events << JinkEffect << EventPhaseChanging << Death;
        view_as_skill = new DaheViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        if (event == JinkEffect) {
            CardStar jink = data.value<CardStar>();
            ServerPlayer *bgm_zhangfei = room->findPlayerBySkillName(objectName());
            if (bgm_zhangfei && bgm_zhangfei->isAlive() && player->hasFlag(objectName()) && jink->getSuit() != Card::Heart) {
                LogMessage log;
                log.type = "#DaheEffect";
                log.from = bgm_zhangfei;
                log.to << player;
                log.arg = jink->getSuitString();
                log.arg2 = "dahe";
                room->sendLog(log);

                return true;
            }
            return false;
        }
        if (event == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to != Player::NotActive)
                return false;
        }
        if (event == Death) {
            DeathStruct death = data.value<DeathStruct>();
            if (death.who != player)
                return false;
        }
        foreach (ServerPlayer *other, room->getOtherPlayers(player))
            if (other->hasFlag(objectName()))
                room->setPlayerFlag(other, "-" + objectName());
        return false;
    }
};

class DahePindian: public TriggerSkill {
public:
    DahePindian(): TriggerSkill("#dahe") {
        events << Pindian;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *, QVariant &data) const{
        PindianStar pindian = data.value<PindianStar>();
        if (pindian->reason != "dahe" || !pindian->from->hasSkill(objectName()))
            return false;

        if (pindian->isSuccess()) {
            room->setPlayerFlag(pindian->to, "dahe");
            QList<ServerPlayer *> to_givelist = room->getAlivePlayers();
            foreach (ServerPlayer *p, to_givelist) {
                if (p->getHp() > pindian->from->getHp())
                    to_givelist.removeOne(p);
            }
            QString choice = room->askForChoice(pindian->from, "dahe", "yes+no");
            if (!to_givelist.isEmpty() && choice == "yes") {
                ServerPlayer *to_give = room->askForPlayerChosen(pindian->from, to_givelist, "dahe");
                CardMoveReason reason(CardMoveReason::S_REASON_GIVE, pindian->from->objectName());
                reason.m_playerId = to_give->objectName();
                to_give->obtainCard(pindian->to_card);
            }
        } else {
            if (!pindian->from->isKongcheng()) {
                room->showAllCards(pindian->from);
                room->askForDiscard(pindian->from, "dahe", 1, 1, false, false);
            }
        }
        return false;
    }
};

TanhuCard::TanhuCard() {
    will_throw = false;
    handling_method = Card::MethodPindian;
}

bool TanhuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void TanhuCard::use(Room *room, ServerPlayer *lvmeng, QList<ServerPlayer *> &targets) const{
    bool success = lvmeng->pindian(targets.first(), "tanhu", this);
    if (success) {
        room->broadcastSkillInvoke("tanhu", 2);
        lvmeng->tag["TanhuInvoke"] = QVariant::fromValue(targets.first());
        room->setPlayerFlag(targets.first(), "TanhuTarget");
        room->setFixedDistance(lvmeng, targets.first(), 1);
    } else {
        room->broadcastSkillInvoke("tanhu", 3);
    }
}

class TanhuViewAsSkill: public OneCardViewAsSkill {
public:
    TanhuViewAsSkill(): OneCardViewAsSkill("tanhu") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("TanhuCard") && !player->isKongcheng();
    }

    virtual bool viewFilter(const Card *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Card *newCard = new TanhuCard;
        newCard->addSubcard(originalCard);
        return newCard;
    }
};

class Tanhu: public TriggerSkill {
public:
    Tanhu(): TriggerSkill("tanhu") {
        events << EventPhaseChanging << Death << EventLoseSkill << TrickCardCanceling;
        view_as_skill = new TanhuViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        if (event == TrickCardCanceling) {
            CardEffectStruct effect = data.value<CardEffectStruct>();
            if (effect.from && effect.from->hasSkill(objectName()) && effect.from->isAlive()
                && player->hasFlag("TanhuTarget"))
                return true;
        } else if (player->tag["TanhuInvoke"].value<PlayerStar>() != NULL) {
            if (event == EventPhaseChanging) {
                PhaseChangeStruct change = data.value<PhaseChangeStruct>();
                if (change.to != Player::NotActive)
                    return false;
            }
            if (event == Death) {
                DeathStruct death = data.value<DeathStruct>();
                if (death.who != player)
                    return false;
            }

            ServerPlayer *target = player->tag["TanhuInvoke"].value<PlayerStar>();

            room->setPlayerFlag(target, "-TanhuTarget");
            room->setFixedDistance(player, target, -1);
            player->tag.remove("TanhuInvoke");
        }
        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const{
        return 1;
    }
};

class MouduanStart: public GameStartSkill {
public:
    MouduanStart(): GameStartSkill("#mouduan-start") {
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual void onGameStart(ServerPlayer *lvmeng) const{
        Room *room = lvmeng->getRoom();
        lvmeng->gainMark("@wu");
        room->acquireSkill(lvmeng, "jiang");
        room->acquireSkill(lvmeng, "qianxun");
    }
};

class Mouduan: public TriggerSkill {
public:
    Mouduan(): TriggerSkill("mouduan") {
        events << EventPhaseStart << CardsMoveOneTime;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual int getPriority() const{
        return 1;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *lvmeng = room->findPlayerBySkillName(objectName());

        if (event == CardsMoveOneTime) {
            CardsMoveOneTimeStar move = data.value<CardsMoveOneTimeStar>();
            if (move->from == player && player->getMark("@wu") > 0 && player->getHandcardNum() <= 2) {
                room->broadcastSkillInvoke(objectName());

                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = player;
                log.arg = "mouduan";
                room->sendLog(log);

                player->loseMark("@wu");
                player->gainMark("@wen");
                room->detachSkillFromPlayer(player, "jiang");
                room->detachSkillFromPlayer(player, "qianxun");
                room->acquireSkill(player, "yingzi");
                room->acquireSkill(player, "keji");
            }
        } else if (player->getPhase() == Player::RoundStart && lvmeng && lvmeng->getMark("@wen") > 0
                   && !lvmeng->isNude() && lvmeng->askForSkillInvoke(objectName())) {
            room->askForDiscard(lvmeng, "mouduan", 1, 1, false, true);
            if (lvmeng->getHandcardNum() > 2) {
                room->broadcastSkillInvoke(objectName());
                lvmeng->loseMark("@wen");
                lvmeng->gainMark("@wu");
                room->detachSkillFromPlayer(lvmeng, "yingzi");
                room->detachSkillFromPlayer(lvmeng, "keji");
                room->acquireSkill(lvmeng, "jiang");
                room->acquireSkill(lvmeng, "qianxun");
            }
        }
        return false;
    }
};

class MouduanClear: public TriggerSkill {
public:
    MouduanClear(): TriggerSkill("#mouduan-clear") {
        events << EventLoseSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill("mouduan");
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (data.toString() == "mouduan") {
            if (player->getMark("@wu") > 0) {
                player->loseMark("@wu");
                room->detachSkillFromPlayer(player, "jiang");
                room->detachSkillFromPlayer(player, "qianxun");
            } else if (player->getMark("@wen") > 0) {
                player->loseMark("@wen");
                room->detachSkillFromPlayer(player, "yingzi");
                room->detachSkillFromPlayer(player, "keji");
            }
        }
        return false;
    }
};

class Zhaolie: public DrawCardsSkill {
public:
    Zhaolie(): DrawCardsSkill("zhaolie") {
    }

    virtual int getDrawNum(ServerPlayer *liubei, int n) const{
        Room *room = liubei->getRoom();
        QList<ServerPlayer *> targets = room->getOtherPlayers(liubei);
        QList<ServerPlayer *> victims;
        foreach (ServerPlayer *p, targets) {
            if (liubei->inMyAttackRange(p))
                victims << p;
        }
        if (victims.empty())
            return n;
        if (room->askForSkillInvoke(liubei, objectName())) {
            liubei->setFlags("zhaolie");
            return n - 1;
        }

        return n;
    }
};

class ZhaolieAct: public TriggerSkill {
public:
    ZhaolieAct(): TriggerSkill("#zhaolie") {
        events << AfterDrawNCards;
    }

    virtual bool triggerable(const ServerPlayer *player) const{
        return player != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *liubei, QVariant &) const{
        if (!liubei->hasFlag("zhaolie")) return false;
        room->setPlayerFlag(liubei, "-zhaolie");

        QList<ServerPlayer *> targets = room->getOtherPlayers(liubei);
        QList<ServerPlayer *> victims;
        QList<const Card *> cards;
        int no_basic = 0;
        foreach (ServerPlayer *p, targets) {
            if (liubei->inMyAttackRange(p))
                victims << p;
        }
        ServerPlayer *victim = room->askForPlayerChosen(liubei, victims, "zhaolie");
        QList<int> cardIds;
        for (int i = 0; i < 3; i++) {
            int id = room->drawCard();
            cardIds << id;
            CardsMoveStruct move;
            move.card_ids << id;
            move.to_place = Player::PlaceTable;
            move.reason = CardMoveReason(CardMoveReason::S_REASON_TURNOVER, liubei->objectName(), QString(), "zhaolie", QString());
            room->moveCardsAtomic(move, true);
            room->getThread()->delay();
        }
        for (int i = 0; i < 3; i++) {
            int card_id = cardIds[i];
            const Card *card = Sanguosha->getCard(card_id);
            if (!card->isKindOf("BasicCard") || card->isKindOf("Peach")) {
                if (!card->isKindOf("BasicCard"))
                    no_basic++;
                CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, QString(), "zhaolie", QString());
                room->throwCard(Sanguosha->getCard(card_id), reason, NULL);
            } else {
                cards << card;
            }
        }
        QStringList choicelist;
        choicelist << "damage";
        if (victim->getCards("he").length() >= no_basic)
            choicelist << "throw";

        QVariant data = QVariant::fromValue(no_basic);
        QString choice = room->askForChoice(victim, "zhaolie", choicelist.join("+"), data);

        if (choice == "damage") {
            room->broadcastSkillInvoke("zhaolie", 1);
            if (no_basic > 0) {
                DamageStruct damage;
                damage.card = NULL;
                damage.from = liubei;
                damage.to = victim;
                damage.damage = no_basic;

                room->damage(damage);
            }
            if (!cards.empty()) {
                DummyCard *dummy_card = new DummyCard;
                foreach (const Card *c, cards)
                    dummy_card->addSubcard(c);
                CardMoveReason reason(CardMoveReason::S_REASON_GOTBACK, victim->objectName());
                if (victim->isAlive())
                    room->obtainCard(victim, dummy_card, reason, true);
                else {
                    CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, victim->objectName(), "zhaolie", QString());
                    room->throwCard(dummy_card, reason, NULL);
                }
                delete dummy_card;
            }
        } else {
            room->broadcastSkillInvoke("zhaolie", 2);
            if (no_basic > 0)
                room->askForDiscard(victim, "zhaolie", no_basic, no_basic, false, true);
            if (!cards.empty()) {
                foreach (const Card *c, cards) {
                    CardMoveReason reason(CardMoveReason::S_REASON_GOTBACK, liubei->objectName());
                    room->obtainCard(liubei, c, reason);
                }
            }
        }
        return false;
    }
};

class Shichou: public TriggerSkill {
public:
    Shichou(): TriggerSkill("shichou$") {
        events << GameStart << EventPhaseStart << DamageInflicted << Dying;
        frequency = Limited;
    }

    virtual bool triggerable(const ServerPlayer *player) const{
        return player != NULL;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        if (event == GameStart) {
            foreach(ServerPlayer *p, room->getAlivePlayers())
                if (p->hasLordSkill("shichou"))
                    room->setPlayerMark(p, "@hate", 1);
        } else if (event == EventPhaseStart && player->getMark("xhate") == 0 && player->hasLordSkill("shichou")
                  && player->getPhase() == Player::Start && player->getCards("he").length() > 1) {
            QList<ServerPlayer *> targets = room->getOtherPlayers(player);
            QList<ServerPlayer *> victims;

            foreach (ServerPlayer *p, targets) {
                if(p->getKingdom() == "shu")
                    victims << p;
            }
            if (victims.empty())
                return false;
            if (player->askForSkillInvoke(objectName())) {
                room->broadcastSkillInvoke(objectName());
                room->broadcastInvoke("animate", "lightbox:$ShichouAnimate:4500");
                room->getThread()->delay(4500);
                player->loseMark("@hate", 1);
                room->setPlayerMark(player, "xhate", 1);
                ServerPlayer *victim = room->askForPlayerChosen(player, victims, objectName());
                victim->gainMark("@hate_to");
                room->setPlayerMark(victim, "hate_" + player->objectName(), 1);

                const Card *card = room->askForExchange(player, objectName(), 2, true, "ShichouGive");
                CardMoveReason reason(CardMoveReason::S_REASON_GIVE, player->objectName());
                reason.m_playerId = victim->objectName();
                room->obtainCard(victim, card, reason, false);
            }
        } else if(event == DamageInflicted && player->hasLordSkill(objectName()) && !player->hasFlag("ShichouTarget")) {
            ServerPlayer *target = NULL;
            foreach (ServerPlayer *p, room->getOtherPlayers(player)) {
                if (p->getMark("hate_" + player->objectName()) > 0 && p->getMark("@hate_to") > 0) {
                    target = p;
                    break;
                }
            }
            if (target == NULL || target->isDead())
                return false;
            LogMessage log;
            log.type = "#ShichouProtect";
            log.arg = objectName();
            log.from = player;
            log.to << target;
            room->sendLog(log);
            DamageStruct damage = data.value<DamageStruct>();
            DamageStruct newdamage = damage;
            newdamage.to = target;
            newdamage.transfer = true;

            room->setPlayerFlag(target, "ShichouTarget");
            room->damage(newdamage);
            return true;
        } else if (event == Dying) {
            DyingStruct dying = data.value<DyingStruct>();
            if (dying.who != player)
                return false;
            if (player->getMark("@hate_to") > 0)
                player->loseAllMarks("@hate_to");
        }
        return false;
    }
};

class ShichouDraw: public TriggerSkill {
public:
    ShichouDraw(): TriggerSkill("#shichou") {
        events << DamageComplete;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if (player->isAlive() && player->hasFlag("ShichouTarget") && damage.transfer){
            player->drawCards(damage.damage);
            room->setPlayerFlag(player, "-ShichouTarget");
        }

        return false;
    }
};

YanxiaoCard::YanxiaoCard(Suit suit, int number)
    : DelayedTrick(suit, number)
{
    mute = true;
    setObjectName("YanxiaoCard");
}

bool YanxiaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const{
    if (!targets.isEmpty())
        return false;

    if (to_select->containsTrick(objectName()))
        return false;

    return true;
}

void YanxiaoCard::onUse(Room *room, const CardUseStruct &card_use) const{
    bool has_sunce = false;
    foreach (ServerPlayer *to, card_use.to)
        if (to->getGeneralName().contains("sunce")) {
            has_sunce = true;
            break;
        }
    int index = has_sunce ? 2 : 1;
    room->broadcastSkillInvoke("yanxiao", index);
    DelayedTrick::onUse(room, card_use);
}

void YanxiaoCard::takeEffect(ServerPlayer *) const{
}

class YanxiaoViewAsSkill: public OneCardViewAsSkill {
public:
    YanxiaoViewAsSkill(): OneCardViewAsSkill("yanxiao") {
    }

    virtual bool viewFilter(const Card *to_select) const{
        return to_select->getSuit() == Card::Diamond;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        YanxiaoCard *yanxiao = new YanxiaoCard(originalCard->getSuit(), originalCard->getNumber());
        yanxiao->addSubcard(originalCard->getId());
        yanxiao->setSkillName(objectName());
        return yanxiao;
    }
};

class Yanxiao: public PhaseChangeSkill {
public:
    Yanxiao(): PhaseChangeSkill("yanxiao") {
        view_as_skill = new YanxiaoViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target && target->getPhase() == Player::Judge && target->containsTrick("YanxiaoCard");
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        CardsMoveStruct move;
        LogMessage log;
        log.type = "$YanxiaoGot";
        log.from = target;

        foreach (const Card *delayed_trick, target->getJudgingArea())
            move.card_ids << delayed_trick->getEffectiveId();
        log.card_str = Card::IdsToStrings(move.card_ids).join("+");
        target->getRoom()->sendLog(log);

        move.to = target;
        move.to_place = Player::PlaceHand;
        target->getRoom()->moveCardsAtomic(move, true);

        return false;
    }
};

class Anxian: public TriggerSkill {
public:
    Anxian(): TriggerSkill("anxian") {
        events << DamageCaused << TargetConfirming << SlashEffected;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *daqiao, QVariant &data) const{
        if (event == DamageCaused) {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.card && damage.card->isKindOf("Slash")
                && !damage.chain && !damage.transfer && !damage.to->isKongcheng()
                && daqiao->askForSkillInvoke(objectName(), data)) {
                room->broadcastSkillInvoke(objectName(), 1);
                LogMessage log;
                log.type = "#Anxian";
                log.from = daqiao;
                log.arg = objectName();
                room->sendLog(log);
                if (!damage.to->isKongcheng())
                    room->askForDiscard(damage.to, "anxian", 1, 1);
                daqiao->drawCards(1);
                return true;
            }
        } else if (event == TargetConfirming) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (!use.to.contains(daqiao) || daqiao->isKongcheng())
                return false;
            if (use.card && use.card->isKindOf("Slash")) {
                if (room->askForCard(daqiao, ".", "@anxian-discard", data, objectName())) {
                    room->broadcastSkillInvoke(objectName(), 2);
                    daqiao->addMark("anxian");
                    use.from->drawCards(1);
                    LogMessage log;
                    log.type = "#AnxianDiscard";
                    log.from = use.from;
                    log.to << daqiao;
                    log.arg = objectName();
                    room->sendLog(log);
                }
            }
        } else if (event == SlashEffected) {
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if (daqiao->getMark("anxian") > 0) {
                LogMessage log;
                log.type = "#AnxianAvoid";
                log.from = effect.from;
                log.to << daqiao;
                log.arg = objectName();
                room->sendLog(log);
                room->setPlayerMark(daqiao, "anxian", daqiao->getMark("anxian") - 1);
                return true;
            }
        } else {
            room->setPlayerMark(daqiao, "anxian", 0);
        }
        return false;
    }
};

YinlingCard::YinlingCard() {
}

bool YinlingCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void YinlingCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    if (effect.to->isNude() || effect.from->getPile("brocade").length() >= 4)
        return;
    int card_id = room->askForCardChosen(effect.from, effect.to, "he", "yinling");
    effect.from->addToPile("brocade", card_id);
}

class Yinling: public OneCardViewAsSkill {
public:
    Yinling(): OneCardViewAsSkill("yinling") {
    }

    virtual bool viewFilter(const Card *to_select) const{
        return to_select->isBlack();
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getPile("brocade").length() < 4;
    }

    virtual const Card *viewAs(const Card *originalcard) const{
        YinlingCard *card = new YinlingCard;
        card->addSubcard(originalcard);
        return card;
    }
};

class YinlingClear: public TriggerSkill {
public:
    YinlingClear(): TriggerSkill("#yinling-clear") {
        events << EventLoseSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target && !target->hasSkill("yinling") && target->getPile("brocade").length() > 0;
    }

    virtual bool trigger(TriggerEvent, Room *, ServerPlayer *player, QVariant &) const{
        player->clearOnePrivatePile("brocade");
        return false;
    }
};

class Junwei: public TriggerSkill {
public:
    Junwei(): TriggerSkill("junwei") {
        events << EventPhaseStart;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *ganning, QVariant &) const{
        if (ganning->getPhase() == Player::Finish && ganning->getPile("brocade").length() >= 3
            && ganning->askForSkillInvoke(objectName())) {
            QList<int> brocade = ganning->getPile("brocade");
            room->broadcastSkillInvoke(objectName());

            int ai_delay = Config.AIDelay;
            Config.AIDelay = 0;

            QList<const Card *> to_throw;
            for (int i = 0; i < 3; i++) {
                int card_id = 0;
                room->fillAG(brocade, ganning);
                if (brocade.length() == 3 - i)
                    card_id = brocade.first();
                else
                    card_id = room->askForAG(ganning, brocade, false, objectName());
                ganning->invoke("clearAG");

                brocade.removeOne(card_id);

                to_throw << Sanguosha->getCard(card_id);
            }
            DummyCard *dummy = new DummyCard;
            dummy->addSubcards(to_throw);
            CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, QString(), objectName(), QString());
            room->throwCard(dummy, reason, NULL);
            dummy->deleteLater();

            Config.AIDelay = ai_delay;

            ServerPlayer *target = room->askForPlayerChosen(ganning, room->getAllPlayers(), objectName());
            QVariant ai_data = QVariant::fromValue((PlayerStar)ganning);
            const Card *card = room->askForCard(target, "Jink", "@junwei-show", ai_data, Card::MethodNone);
            if (card) {
                room->showCard(target, card->getEffectiveId());
                ServerPlayer *receiver = room->askForPlayerChosen(ganning, room->getAllPlayers(), "junweigive");
                if (receiver != target)
                    receiver->obtainCard(card);
            } else {
                room->loseHp(target, 1);
                if (!target->isAlive()) return false;
                if (target->hasEquip()) {
                    int card_id = room->askForCardChosen(ganning, target, "e", objectName());
                    target->addToPile("junwei_equip", card_id);
                }
            }
        }
        return false;
    }
};

class JunweiGot: public TriggerSkill {
public:
    JunweiGot(): TriggerSkill("#junwei-got") {
        events << EventPhaseChanging;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const {
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (change.to != Player::NotActive || player->getPile("junwei_equip").length() == 0)
            return false;
        foreach (int card_id, player->getPile("junwei_equip")) {
            const Card *card = Sanguosha->getCard(card_id);

            int equip_index = -1;
            const EquipCard *equip = qobject_cast<const EquipCard *>(card->getRealCard());
            equip_index = static_cast<int>(equip->location());

            QList<CardsMoveStruct> exchangeMove;
            CardsMoveStruct move1;
            move1.card_ids << card_id;
            move1.to = player;
            move1.to_place = Player::PlaceEquip;
            move1.reason = CardMoveReason(CardMoveReason::S_REASON_EXCHANGE_FROM_PILE, player->objectName());
            exchangeMove.push_back(move1);
            if (player->getEquip(equip_index) != NULL) {
                CardsMoveStruct move2;
                move2.card_ids << player->getEquip(equip_index)->getId();
                move2.to = NULL;
                move2.to_place = Player::DiscardPile;
                move2.reason = CardMoveReason(CardMoveReason::S_REASON_CHANGE_EQUIP, player->objectName());
                exchangeMove.push_back(move2);
            }
            LogMessage log;
            log.from = player;
            log.type = "$JunweiGot";
            log.card_str = QString::number(card_id);
            room->sendLog(log);

            room->moveCardsAtomic(exchangeMove, true);
        }
        return false;
    }
};

class Fenyong: public TriggerSkill {
public:
    Fenyong(): TriggerSkill("fenyong") {
        events << Damaged << DamageInflicted;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &) const {
        if (event == Damaged) {
            if (player->getMark("@fenyong") == 0 && room->askForSkillInvoke(player, objectName())) {
                player->gainMark("@fenyong");
                room->broadcastSkillInvoke(objectName(), 1);
            }
        } else if (event == DamageInflicted) {
            if (player->getMark("@fenyong") > 0) {
                room->broadcastSkillInvoke(objectName(), 2);
                LogMessage log;
                log.type = "#FenyongAvoid";
                log.from = player;
                log.arg = objectName();
                room->sendLog(log);
                return true;
            }
        }
        return false;
    }
};

class FenyongForHuashen: public TriggerSkill {
public:
    FenyongForHuashen():TriggerSkill("#fenyong-for-huashen") {
        events << EventPhaseStart;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual int getPriority() const{
        return 1;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const {
        ServerPlayer *zuoci = room->findPlayerBySkillName("fenyong");
        if (zuoci == NULL)
            return false;
        if (player->getPhase() == Player::Finish && zuoci->getMark("@fenyong") > 0) {
            if (zuoci->tag["HuashenSkill"].toString() == "fenyong")
                room->setPlayerMark(zuoci, "@fenyong", 0);
        }
        return false;
    }
};

class FenyongClear: public TriggerSkill {
public:
    FenyongClear(): TriggerSkill("#fenyong-clear") {
        events << EventLoseSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target && !target->hasSkill("fenyong") && target->getMark("@fenyong") > 0;
    }

    virtual bool trigger(TriggerEvent, Room *, ServerPlayer *player, QVariant &) const{
        player->loseAllMarks("@fenyong");
        return false;
    }
};

class Xuehen: public TriggerSkill {
public:
    Xuehen(): TriggerSkill("xuehen") {
        events << EventPhaseStart;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual int getPriority() const{
        return 1;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const{
        ServerPlayer *xiahou = room->findPlayerBySkillName(objectName());
        if (xiahou == NULL)
            return false;
        if (player->getPhase() == Player::Finish && xiahou->getMark("@fenyong") > 0) {
            LogMessage log;
            log.from = xiahou;
            log.arg = objectName();
            log.type = "#TriggerSkill";
            room->sendLog(log);

            xiahou->loseMark("@fenyong");
            QList<ServerPlayer *> targets;
            foreach (ServerPlayer *p, room->getOtherPlayers(xiahou))
                if (xiahou->canSlash(p, NULL, false))
                    targets << p;
            QString choice;
            if (targets.isEmpty())
                choice = "discard";
            else
                choice = room->askForChoice(xiahou, objectName(), "discard+slash");
            if (choice == "slash") {
                room->broadcastSkillInvoke(objectName(), 2);

                ServerPlayer *victim = room->askForPlayerChosen(xiahou, targets, objectName());

                Slash *slash = new Slash(Card::NoSuitNoColor, 0);
                slash->setSkillName(objectName());
                CardUseStruct card_use;
                card_use.from = xiahou;
                card_use.to << victim;
                card_use.card = slash;
                room->useCard(card_use, false);
            } else {
                room->broadcastSkillInvoke(objectName(), 1);
                room->setPlayerFlag(player, "XuehenTarget_InTempMoving");
                DummyCard *dummy = new DummyCard;
                QList<int> card_ids;
                QList<Player::Place> original_places;
                for (int i = 0; i < xiahou->getLostHp(); i++) {
                    if (player->isNude())
                        break;
                    card_ids << room->askForCardChosen(xiahou, player, "he", objectName());
                    original_places << room->getCardPlace(card_ids[i]);
                    dummy->addSubcard(card_ids[i]);
                    player->addToPile("#xuehen", card_ids[i], false);
                }
                for (int i = 0; i < dummy->subcardsLength(); i++)
                    room->moveCardTo(Sanguosha->getCard(card_ids[i]), player, original_places[i], false);
                room->setPlayerFlag(player, "-XuehenTarget_InTempMoving");
                if (dummy->subcardsLength() > 0)
                    room->throwCard(dummy, player, xiahou);
                dummy->deleteLater();
            }
        }
        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const {
        return -2;
	}
};

class XuehenAvoidTriggeringCardsMove: public TriggerSkill {
public:
    XuehenAvoidTriggeringCardsMove(): TriggerSkill("#xuehen-avoid-triggering-cards-move") {
        events << CardsMoving << CardsMoveOneTime;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual int getPriority() const{
        return 10;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *, QVariant &) const{
        foreach (ServerPlayer *p, room->getAllPlayers())
            if (p->hasFlag("XuehenTarget_InTempMoving"))
                return true;
        return false;
    }
};

BGMPackage::BGMPackage(): Package("BGM") {
    General *bgm_zhaoyun = new General(this, "bgm_zhaoyun", "qun", 3);
    bgm_zhaoyun->addSkill("longdan");
    bgm_zhaoyun->addSkill(new ChongZhen);

    General *bgm_diaochan = new General(this, "bgm_diaochan", "qun", 3, false);
    bgm_diaochan->addSkill(new Lihun);
    bgm_diaochan->addSkill("biyue");

    General *bgm_caoren = new General(this, "bgm_caoren", "wei");
    bgm_caoren->addSkill(new Kuiwei);
    bgm_caoren->addSkill(new Yanzheng);

    General *bgm_pangtong = new General(this, "bgm_pangtong", "qun", 3);
    bgm_pangtong->addSkill(new Manjuan);
    bgm_pangtong->addSkill(new Zuixiang);
    bgm_pangtong->addSkill(new ZuixiangClear);
    bgm_pangtong->addSkill(new MarkAssignSkill("@sleep", 1));
    related_skills.insertMulti("zuixiang", "#zuixiang-clear");
    related_skills.insertMulti("zuixiang", "#@sleep-1");

    General *bgm_zhangfei = new General(this, "bgm_zhangfei", "shu");
    bgm_zhangfei->addSkill(new Jie);
    bgm_zhangfei->addSkill(new Dahe);
    bgm_zhangfei->addSkill(new DahePindian);
    related_skills.insertMulti("dahe", "#dahe");

    General *bgm_lvmeng = new General(this, "bgm_lvmeng", "wu", 3);
    bgm_lvmeng->addSkill(new Tanhu);
    bgm_lvmeng->addSkill(new MouduanStart);
    bgm_lvmeng->addSkill(new Mouduan);
    bgm_lvmeng->addSkill(new MouduanClear);
    related_skills.insertMulti("mouduan", "#mouduan-start");
    related_skills.insertMulti("mouduan", "#mouduan-clear");

    General *bgm_liubei = new General(this, "bgm_liubei$", "shu");
    bgm_liubei->addSkill(new Zhaolie);
    bgm_liubei->addSkill(new ZhaolieAct);
    bgm_liubei->addSkill(new Shichou);
    bgm_liubei->addSkill(new ShichouDraw);
    related_skills.insertMulti("zhaolie", "#zhaolie");
    related_skills.insertMulti("shichou", "#shichou");

    General *bgm_daqiao = new General(this, "bgm_daqiao", "wu", 3, false);
    bgm_daqiao->addSkill(new Yanxiao);
    bgm_daqiao->addSkill(new Anxian);

    General *bgm_ganning = new General(this, "bgm_ganning", "qun");
    bgm_ganning->addSkill(new Yinling);
    bgm_ganning->addSkill(new YinlingClear);
    bgm_ganning->addSkill(new Junwei);
    bgm_ganning->addSkill(new JunweiGot);
    related_skills.insertMulti("yinling", "#yinling-clear");
    related_skills.insertMulti("junwei", "#junwei-got");

    General *bgm_xiahoudun = new General(this, "bgm_xiahoudun", "wei");
    bgm_xiahoudun->addSkill(new Fenyong);
    bgm_xiahoudun->addSkill(new FenyongClear);
    bgm_xiahoudun->addSkill(new FenyongForHuashen);
    bgm_xiahoudun->addSkill(new Xuehen);
    bgm_xiahoudun->addSkill(new XuehenAvoidTriggeringCardsMove);
    related_skills.insertMulti("fenyong", "#fenyong-clear");
    related_skills.insertMulti("fenyong", "#fenyong-for-huashen");
    related_skills.insertMulti("xuehen", "#xuehen-avoid-triggering-cards-move");

    addMetaObject<LihunCard>();
    addMetaObject<DaheCard>();
    addMetaObject<TanhuCard>();
    addMetaObject<YanxiaoCard>();
    addMetaObject<YinlingCard>();
}

ADD_PACKAGE(BGM)

class Zhaoxin: public TriggerSkill {
public:
    Zhaoxin(): TriggerSkill("zhaoxin") {
        events << EventPhaseEnd;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *simazhao, QVariant &) const{
        if (simazhao->getPhase() != Player::Draw)
            return false;

        QList<ServerPlayer *> targets;
        foreach (ServerPlayer *p, room->getAllPlayers())
            if (simazhao->canSlash(p))
                targets << p;

        if (targets.isEmpty())
            return false;

        if (room->askForSkillInvoke(simazhao, objectName())) {
            room->showAllCards(simazhao);
            ServerPlayer *victim = room->askForPlayerChosen(simazhao, targets, objectName());

            Slash *slash = new Slash(Card::NoSuitNoColor, 0);
            slash->setSkillName("zhaoxin");
            CardUseStruct card_use;
            card_use.from = simazhao;
            card_use.to << victim;
            card_use.card = slash;
            room->useCard(card_use);
        }

        return false;
    }
};

LangguCard::LangguCard() {
    target_fixed = true;
    will_throw = false;
    handling_method = Card::MethodResponse;
}

void LangguCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
}

class LangguViewAsSkill: public OneCardViewAsSkill {
public:
    LangguViewAsSkill(): OneCardViewAsSkill("langgu") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@langgu";
    }

    virtual bool viewFilter(const Card *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Card *card = new LangguCard;
        card->addSubcard(originalCard);
        return card;
    }
};

class Langgu: public TriggerSkill {
public:
    Langgu(): TriggerSkill("langgu") {
        view_as_skill = new LangguViewAsSkill;
        events << Damaged << AskForRetrial;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *simazhao, QVariant &data) const{
        if (event == Damaged) {
            DamageStruct damage = data.value<DamageStruct>();

            for (int i = 0; i < damage.damage; i++) {
                if (!simazhao->askForSkillInvoke(objectName(), data))
                    return false;

                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(.*):(.*)");
                judge.good = true;
                judge.play_animation = false;
                judge.who = simazhao;
                judge.reason = objectName();

                if (damage.from->hasSkill("hongyan"))
                    room->setPlayerFlag(simazhao, "langgu_hongyan"); //for AI
                room->judge(judge);
                room->setPlayerFlag(simazhao, "-langgu_hongyan");

                if (damage.from && damage.from->isAlive() && !damage.from->isKongcheng()) {
                    room->showAllCards(damage.from, simazhao);

                    QList<int> langgu_discard;
                    foreach (int card_id, damage.from->handCards()) {
                        if (Sanguosha->getWrappedCard(card_id)->getSuit() == judge.card->getSuit())
                            langgu_discard << card_id;
                    }
                    if (langgu_discard.empty())
                        return false;

                    while (!langgu_discard.empty()) {
                        room->fillAG(langgu_discard, simazhao);
                        int id = room->askForAG(simazhao, langgu_discard, true, objectName());
                        if (id == -1) {
                            simazhao->invoke("clearAG");
                            break;
                        }
                        langgu_discard.removeOne(id);
                        simazhao->invoke("clearAG");
                    }

                    if (!langgu_discard.empty()) {
                        DummyCard *dummy = new DummyCard;
                        foreach(int card_id, langgu_discard)
                            dummy->addSubcard(card_id);
                        room->throwCard(dummy, damage.from, simazhao);
                        dummy->deleteLater();
                    }
                }
            }
        } else {
            JudgeStar judge = data.value<JudgeStar>();
            if (judge->reason != objectName() || simazhao->isKongcheng())
                return false;

            QStringList prompt_list;
            prompt_list << "@langgu-card" << judge->who->objectName()
                        << objectName() << judge->reason << QString::number(judge->card->getEffectiveId());
            QString prompt = prompt_list.join(":");
            const Card *card = room->askForCard(simazhao, "@langgu", prompt, data, Card::MethodResponse, judge->who, true);

            room->retrial(card, simazhao, judge, objectName());
        }

        return false;
    }
};

FuluanCard::FuluanCard() {
}

bool FuluanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if (!Self->inMyAttackRange(to_select) || Self == to_select)
        return false;

    if (Self->getWeapon() && subcards.contains(Self->getWeapon()->getId())) {
        const Weapon *weapon = qobject_cast<const Weapon *>(Self->getWeapon()->getRealCard());
        int distance_fix = weapon->getRange() - 1;
        if (Self->getOffensiveHorse() && subcards.contains(Self->getOffensiveHorse()->getId()))
            distance_fix += 1;
        return Self->distanceTo(to_select, distance_fix) <= Self->getAttackRange();
    } else if (Self->getOffensiveHorse() && subcards.contains(Self->getOffensiveHorse()->getId())) {
        return Self->distanceTo(to_select, 1) <= Self->getAttackRange();
    } else
        return true;
}

void FuluanCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    effect.to->turnOver();
    room->setPlayerCardLimitation(effect.from, "use", "Slash", true);
}

class Fuluan: public ViewAsSkill {
public:
    Fuluan(): ViewAsSkill("fuluan") {
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return false;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("FuluanCard") && !player->hasFlag("ForbidFuluan");
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *card) const{
        if (selected.length() >= 3)
            return false;

        if (!selected.isEmpty()) {
            Card::Suit suit = selected.first()->getSuit();
            return card->getSuit() == suit;
        }

        return true;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() != 3)
            return NULL;

        FuluanCard *card = new FuluanCard;
        card->addSubcards(cards);

        return card;
    }
};

class FuluanForbid: public TriggerSkill {
public:
    FuluanForbid(): TriggerSkill("#fuluan-forbid") {
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card->isKindOf("Slash") && player->getPhase() == Player::Play && !player->hasFlag("ForbidFuluan"))
            room->setPlayerFlag(player, "ForbidFuluan");
        return false;
    }
};

class Shude: public TriggerSkill {
public:
    Shude(): TriggerSkill("shude") {
        events << EventPhaseStart;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *wangyuanji, QVariant &) const{
        if (wangyuanji->getPhase() == Player::Finish) {
            int upper = wangyuanji->getMaxHp();
            int handcard = wangyuanji->getHandcardNum();
            if (handcard < upper && room->askForSkillInvoke(wangyuanji, objectName()))
                wangyuanji->drawCards(upper - handcard);
        }

        return false;
    }
};

HuangenCard::HuangenCard() {
}

bool HuangenCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.length() < Self->getHp() && to_select->hasFlag("HuangenTarget");
}

void HuangenCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->tag["Huangen"] = effect.from->tag["Huangen_user"].toString();
    effect.to->drawCards(1);
}

class HuangenViewAsSkill: public ZeroCardViewAsSkill {
public:
    HuangenViewAsSkill():ZeroCardViewAsSkill("huangen"){
    }

    virtual const Card *viewAs() const{
        HuangenCard *card = new HuangenCard;
        return card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@huangen";
    }
};

class Huangen: public TriggerSkill {
public:
    Huangen(): TriggerSkill("huangen") {
        events << TargetConfirmed << CardEffected;
        view_as_skill = new HuangenViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *liuxie = room->findPlayerBySkillName(objectName());
        if (liuxie == NULL) return false;
        if (event == TargetConfirmed) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (player != liuxie || liuxie->getHp() <= 0) return false;
            if (use.to.length() <= 1 || !use.card->isKindOf("TrickCard") || use.card->isKindOf("Collateral"))
                    return false;

            liuxie->tag["Huangen_user"] = use.card->toString();
            foreach (ServerPlayer *p, use.to)
                room->setPlayerFlag(p, "HuangenTarget");
            room->askForUseCard(liuxie, "@@huangen", "@huangen-card");
            foreach (ServerPlayer *p, use.to)
                room->setPlayerFlag(p, "-HuangenTarget");
        } else if (event == CardEffected) {
            CardEffectStruct effect = data.value<CardEffectStruct>();
            if (liuxie->tag["Huangen_user"].toString() == effect.card->toString())
                liuxie->tag.remove("Huangen_user");
            if (!player->isAlive())
                return false;

            if (player->tag["Huangen"].isNull() || player->tag["Huangen"].toString() != effect.card->toString())
                return false;

            player->tag["Huangen"] = QVariant(QString());

            LogMessage log;
            log.type = "#DanlaoAvoid";
            log.from = player;
            log.arg = effect.card->objectName();
            log.arg2 = objectName();
            room->sendLog(log);

            return true;
        }

        return false;
    }
};

HantongCard::HantongCard() {
    target_fixed = true;
}

void HantongCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const{
    QList<int> edict = source->getPile("edict");
    room->broadcastSkillInvoke("hantong");

    int ai_delay = Config.AIDelay;
    Config.AIDelay = 0;

    int card_id = 0;
    room->fillAG(edict, source);
    if (edict.length() == 1)
        card_id = edict.first();
    else
        card_id = room->askForAG(source, edict, false, objectName());
    source->invoke("clearAG");

    edict.removeOne(card_id);

    CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, QString(), "hantong", QString());
    room->throwCard(Sanguosha->getCard(card_id), reason, NULL);

    Config.AIDelay = ai_delay;

    source->tag["Hantong_use"] = true;
    room->acquireSkill(source, "jijiang");
    room->askForUseCard(source, "@jijiang", "@hantong-jijiang");
}

class HantongViewAsSkill: public ZeroCardViewAsSkill {
public:
    HantongViewAsSkill(): ZeroCardViewAsSkill("hantong") {
    }

    virtual const Card *viewAs() const{
        HantongCard *card = new HantongCard;
        return card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getPile("edict").length() > 0 && !player->hasLordSkill("jijiang") && Slash::IsAvailable(player);
    }
};


class Hantong: public TriggerSkill {
public:
    Hantong(): TriggerSkill("hantong") {
        events << CardsMoving;
        view_as_skill = new HantongViewAsSkill;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *liuxie, QVariant &data) const{
        if (liuxie->getPhase() != Player::Discard)
            return false;

        CardsMoveOneTimeStar move = data.value<CardsMoveOneTimeStar>();
        if (move->from != liuxie)
            return false;
        if (move->to_place == Player::DiscardPile
            && ((move->reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_DISCARD)) {
            if (liuxie->askForSkillInvoke(objectName())) {
                int i = 0;
                foreach (int card_id, move->card_ids) {
                    if (room->getCardPlace(card_id) == Player::DiscardPile && move->from_places[i] == Player::PlaceHand)
                        liuxie->addToPile("edict", card_id);
                    i++;
                }
            }
        }
        return false;
    }
};

class HantongAcquire: public TriggerSkill {
public:
    HantongAcquire(): TriggerSkill("#hantong-acquire"){
        events << CardAsked //For JiJiang and HuJia
               << TargetConfirmed //For JiuYuan
               << EventPhaseStart; //For XueYi
    }

    void RemoveEdict(ServerPlayer *liuxie) const{
        Room *room = liuxie->getRoom();
        QList<int> edict = liuxie->getPile("edict");
        room->broadcastSkillInvoke("hantong");

        int ai_delay = Config.AIDelay;
        Config.AIDelay = 0;

        int card_id = 0;
        room->fillAG(edict, liuxie);
        if (edict.length() == 1)
            card_id = edict.first();
        else
            card_id = room->askForAG(liuxie, edict, false, objectName());
        liuxie->invoke("clearAG");

        edict.removeOne(card_id);

        CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, QString(), "hantong_acquire", QString());
        room->throwCard(Sanguosha->getCard(card_id), reason, NULL);

        Config.AIDelay = ai_delay;
    }

    virtual int getPriority() const{
        return 4;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *liuxie, QVariant &data) const{
        ServerPlayer *current = room->getCurrent();
        if (current == NULL || !current->isAlive())
            return false;
        if (liuxie->getPile("edict").length() == 0)
            return false;
        switch (event) {
        case CardAsked: {
                QString pattern = data.toStringList().first();
                if (pattern == "slash" && !liuxie->hasFlag("jijiang_failed")) {
                    QVariant data_for_ai = "jijiang";
                    if (room->askForSkillInvoke(liuxie, "hantong_acquire", data_for_ai)) {
                        RemoveEdict(liuxie);
                        room->acquireSkill(liuxie, "jijiang");
                    }
                } else if (pattern == "jink") {
                    QVariant data_for_ai = "hujia";
                    if (room->askForSkillInvoke(liuxie, "hantong_acquire", data_for_ai)) {
                        RemoveEdict(liuxie);
                        room->acquireSkill(liuxie, "hujia");
                    }
                }
                break;
            }
        case TargetConfirmed: {
                CardUseStruct use = data.value<CardUseStruct>();
                if (!use.card->isKindOf("Peach") || use.from == NULL || use.from->getKingdom() != "wu"
                    || liuxie == use.from || !liuxie->hasFlag("dying"))
                    return false;

                QVariant data_for_ai = "jiuyuan";
                if (room->askForSkillInvoke(liuxie, "hantong_acquire", data_for_ai)) {
                    RemoveEdict(liuxie);
                    room->acquireSkill(liuxie, "jiuyuan");
                }
                break;
            }
        case EventPhaseStart: {
                if (liuxie->getPhase() != Player::Discard)
                    return false;
                QVariant data_for_ai = "xueyi";
                if (room->askForSkillInvoke(liuxie, "hantong_acquire", data_for_ai)) {
                    RemoveEdict(liuxie);
                    room->acquireSkill(liuxie, "xueyi");
                }
                break;
            }
        default:
            break;
        }
        liuxie->tag["Hantong_use"] = true;
        return false;
    }
};

class HantongDetach: public TriggerSkill {
public:
    HantongDetach(): TriggerSkill("#hantong-detach") {
        events << EventPhaseChanging;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *, QVariant &data) const{
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (change.to != Player::NotActive)
            return false;

        foreach (ServerPlayer *p, room->getAllPlayers()) {
            if (!p->tag.value("Hantong_use", false).toBool())
                continue;
            room->detachSkillFromPlayer(p, "hujia");
            room->detachSkillFromPlayer(p, "jijiang");
            room->detachSkillFromPlayer(p, "jiuyuan");
            room->detachSkillFromPlayer(p, "xueyi");
            p->tag.remove("Hantong_use");
        }
        return false;
    }
};

class HantongClear: public TriggerSkill {
public:
    HantongClear(): TriggerSkill("#hantong-clear") {
        events << EventLoseSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target && !target->hasSkill("hantong") && target->getPile("edict").length() > 0;
    }

    virtual bool trigger(TriggerEvent, Room *, ServerPlayer *player, QVariant &) const{
        player->clearOnePrivatePile("edict");
        return false;
    }
};

DIYYicongCard::DIYYicongCard() {
    will_throw = false;
    target_fixed = true;
    handling_method = Card::MethodNone;
}

void DIYYicongCard::use(Room *, ServerPlayer *source, QList<ServerPlayer *> &) const{
    foreach (int id, this->subcards) {
        source->addToPile("retinue", id, true);
    }
}

class DIYYicongViewAsSkill: public ViewAsSkill {
public:
    DIYYicongViewAsSkill(): ViewAsSkill("diyyicong") {
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@diyyicong";
    }

    virtual bool viewFilter(const QList<const Card *> &, const Card *) const{
        return true;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() == 0)
            return NULL;

        Card *acard = new DIYYicongCard;
        acard->addSubcards(cards);
        return acard;
    }
};

class DIYYicong: public TriggerSkill {
public:
    DIYYicong(): TriggerSkill("diyyicong") {
        events << EventPhaseEnd;
        view_as_skill = new DIYYicongViewAsSkill;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *gongsunzan, QVariant &) const{
        if (gongsunzan->getPhase() == Player::Discard && !gongsunzan->isNude()) {
            room->askForUseCard(gongsunzan, "@@diyyicong", "@diyyicong", -1, Card::MethodNone);
        }
        return false;
    }
};

class DIYYicongDistance: public DistanceSkill {
public:
    DIYYicongDistance(): DistanceSkill("#diyyicong-dist") {
    }

    virtual int getCorrect(const Player *, const Player *to) const{
        if (to->hasSkill("diyyicong"))
            return to->getPile("retinue").length();
        else
            return 0;
    }
};

class DIYYicongClear: public TriggerSkill {
public:
    DIYYicongClear(): TriggerSkill("#diyyicong-clear") {
        events << EventLoseSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target && !target->hasSkill("diyyicong") && target->getPile("retinue").length() > 0;
    }

    virtual bool trigger(TriggerEvent, Room *, ServerPlayer *player, QVariant &) const{
        player->clearOnePrivatePile("retinue");
        return false;
    }
};

class Tuqi: public TriggerSkill {
public:
    Tuqi(): TriggerSkill("tuqi") {
        events << EventPhaseStart << EventPhaseChanging;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *gongsunzan, QVariant &data) const{
        if (event == EventPhaseStart) {
            if (gongsunzan->getPhase() == Player::Start && gongsunzan->getPile("retinue").length() > 0) {
                LogMessage log;
                log.from = gongsunzan;
                log.arg = objectName();
                log.type = "#TriggerSkill";
                room->sendLog(log);

                int n = gongsunzan->getPile("retinue").length();
                room->setPlayerMark(gongsunzan, "tuqi_dist", n);
                gongsunzan->clearOnePrivatePile("retinue");
                if (n <= 2)
                    gongsunzan->drawCards(1);
            }
        } else {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to != Player::NotActive)
                return false;
            room->setPlayerMark(gongsunzan, "tuqi_dist", 0);
        }
        return false;
    }
};

class TuqiDistance: public DistanceSkill {
public:
    TuqiDistance(): DistanceSkill("#tuqi-dist") {
    }

    virtual int getCorrect(const Player *from, const Player *) const{
        if (from->hasSkill("tuqi"))
            return -from->getMark("tuqi_dist");
        else
            return 0;
    }
};

BGMDIYPackage::BGMDIYPackage(): Package("BGMDIY") {
    General *diy_simazhao = new General(this, "diy_simazhao", "wei", 3);
    diy_simazhao->addSkill(new Zhaoxin);
    diy_simazhao->addSkill(new Langgu);

    General *diy_wangyuanji = new General(this, "diy_wangyuanji", "wei", 3, false);
    diy_wangyuanji->addSkill(new Fuluan);
    diy_wangyuanji->addSkill(new FuluanForbid);
    diy_wangyuanji->addSkill(new Shude);
    related_skills.insertMulti("fuluan", "#fuluan-forbid");

    General *diy_liuxie = new General(this, "diy_liuxie", "qun");
    diy_liuxie->addSkill(new Huangen);
    diy_liuxie->addSkill(new Hantong);
    diy_liuxie->addSkill(new HantongAcquire);
    diy_liuxie->addSkill(new HantongDetach);
    diy_liuxie->addSkill(new HantongClear);
    related_skills.insertMulti("hantong", "#hantong-acquire");
    related_skills.insertMulti("hantong", "#hantong-detach");
    related_skills.insertMulti("hantong", "#hantong-clear");

    General *diy_gongsunzan = new General(this, "diy_gongsunzan", "qun");
    diy_gongsunzan->addSkill(new DIYYicong);
    diy_gongsunzan->addSkill(new DIYYicongDistance);
    diy_gongsunzan->addSkill(new DIYYicongClear);
    diy_gongsunzan->addSkill(new Tuqi);
    diy_gongsunzan->addSkill(new TuqiDistance);
    related_skills.insertMulti("diyyicong", "#diyyicong-clear");
    related_skills.insertMulti("diyyicong", "#diyyicong-dist");
    related_skills.insertMulti("tuqi", "#tuqi-dist");

    addMetaObject<LangguCard>();
    addMetaObject<FuluanCard>();
    addMetaObject<HuangenCard>();
    addMetaObject<HantongCard>();
    addMetaObject<DIYYicongCard>();
}

ADD_PACKAGE(BGMDIY)
// FORMATTED