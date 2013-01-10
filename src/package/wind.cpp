#include "settings.h"
#include "standard.h"
#include "skill.h"
#include "wind.h"
#include "client.h"
#include "engine.h"
#include "ai.h"
#include "general.h"

// skill cards

GuidaoCard::GuidaoCard(){
    target_fixed = true;
    will_throw = false;
    handling_method = Card::MethodResponse;
}

void GuidaoCard::use(Room *room, ServerPlayer *zhangjiao, QList<ServerPlayer *> &targets) const{

}

LeijiCard::LeijiCard(){

}

bool LeijiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty();
}

void LeijiCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *zhangjiao = effect.from;
    ServerPlayer *target = effect.to;

    Room *room = zhangjiao->getRoom();

    JudgeStruct judge;
    judge.pattern = QRegExp("(.*):(spade):(.*)");
    judge.good = false;
    judge.negative = true;
    judge.reason = "leiji";
    judge.who = target;

    room->judge(judge);

    if(judge.isBad()){
        DamageStruct damage;
        damage.card = NULL;
        damage.damage = 2;
        damage.from = zhangjiao;
        damage.to = target;
        damage.nature = DamageStruct::Thunder;

        room->damage(damage);
    }
}

HuangtianCard::HuangtianCard(){
    will_throw = false;
    handling_method = Card::MethodNone;
    m_skillName = "huangtianv";
    mute = true;
}

void HuangtianCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    ServerPlayer *zhangjiao = targets.first();
    if(zhangjiao->hasLordSkill("huangtian")){
        room->setPlayerFlag(zhangjiao, "HuangtianInvoked");
        room->broadcastSkillInvoke("huangtian");
        zhangjiao->obtainCard(this);
        QList<int> subcards = this->getSubcards();
        foreach(int card_id, subcards)
            room->setCardFlag(card_id,"visible");
        QList<ServerPlayer *> zhangjiaos;
        QList<ServerPlayer *> players = room->getOtherPlayers(source);
        foreach(ServerPlayer *p, players){
            if(p->hasLordSkill("huangtian") && !p->hasFlag("HuangtianInvoked")){
                zhangjiaos << p;
            }
        }
        if(zhangjiaos.empty())
            room->setPlayerFlag(source, "ForbidHuangtian");
    }
}

bool HuangtianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->hasLordSkill("huangtian")
            && to_select != Self && !to_select->hasFlag("HuangtianInvoked");
}

class GuidaoViewAsSkill:public OneCardViewAsSkill{
public:
    GuidaoViewAsSkill():OneCardViewAsSkill("guidao"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@guidao";
    }

    virtual bool viewFilter(const Card *to_select) const{
        return to_select->isBlack();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        GuidaoCard *guidaoCard = new GuidaoCard;
        guidaoCard->setSuit(originalCard->getSuit());
        guidaoCard->addSubcard(originalCard);

        return guidaoCard;
    }
};

class Guidao: public TriggerSkill{
public:
    Guidao():TriggerSkill("guidao"){
        view_as_skill = new GuidaoViewAsSkill;

        events << AskForRetrial;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        
        if(target == NULL || !TriggerSkill::triggerable(target))
            return false;

        if(target->isKongcheng()){
            bool has_black = false;
            for(int i = 0; i < 4; i++){
                const EquipCard *equip = target->getEquip(i);
                if(equip && equip->isBlack()){
                    has_black = true;
                    break;
                }
            }

            return has_black;
        }else
            return true;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if (player == NULL) return false;
        JudgeStar judge = data.value<JudgeStar>();

        QStringList prompt_list;
        prompt_list << "@guidao-card" << judge->who->objectName()
                << objectName() << judge->reason << QString::number(judge->card->getEffectiveId());
        QString prompt = prompt_list.join(":");
        const Card *card = room->askForCard(player, "@guidao", prompt, data, Card::MethodResponse, judge->who, true);

        if (card != NULL){
            room->broadcastSkillInvoke(objectName());
            room->retrial(card, player, judge, objectName(), true);
        }
        return false;
    }
};

class HuangtianViewAsSkill: public OneCardViewAsSkill{
public:
    HuangtianViewAsSkill():OneCardViewAsSkill("huangtianv"){
        attached_lord_skill = true;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getKingdom() == "qun" && !player->hasFlag("ForbidHuangtian") && !player->isKongcheng();
    }

    virtual bool viewFilter(const Card *to_select) const{
        const Card *card = to_select;
        return card->objectName() == "jink" || card->objectName() == "lightning";
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        HuangtianCard *card = new HuangtianCard;
        card->addSubcard(originalCard);

        return card;
    }
};

class Huangtian: public TriggerSkill{
public:
    Huangtian():TriggerSkill("huangtian$"){
        events << GameStart << EventPhaseChanging;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        if(event == GameStart && player->isLord()){
            QList<ServerPlayer *> lords;
            foreach(ServerPlayer *p, room->getAlivePlayers())
                if(p->hasLordSkill(objectName()))
                    lords << p;

            foreach(ServerPlayer *lord, lords){
                QList<ServerPlayer *> players = room->getOtherPlayers(lord);
                foreach(ServerPlayer *p, players){
                    if(!p->hasSkill("huangtianv"))
                        room->attachSkillToPlayer(p, "huangtianv");
                }
            }
        }
        else if(event == EventPhaseChanging){
            PhaseChangeStruct phase_change = data.value<PhaseChangeStruct>();
            if (phase_change.from != Player::Play)
                  return false;
            if(player->hasFlag("ForbidHuangtian")){
                room->setPlayerFlag(player, "-ForbidHuangtian");
            }
            QList<ServerPlayer *> players = room->getOtherPlayers(player);
            foreach(ServerPlayer *p, players){
                if(p->hasFlag("HuangtianInvoked")){
                    room->setPlayerFlag(p, "-HuangtianInvoked");
                }
            }
        }
        return false;
    }
};

class LeijiViewAsSkill: public ZeroCardViewAsSkill{
public:
    LeijiViewAsSkill():ZeroCardViewAsSkill("leiji"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@leiji";
    }

    virtual const Card *viewAs() const{
        return new LeijiCard;
    }
};

class Leiji: public TriggerSkill{
public:
    Leiji():TriggerSkill("leiji"){
        events << CardResponded;
        view_as_skill = new LeijiViewAsSkill;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *zhangjiao, QVariant &data) const{
        if (zhangjiao == NULL) return false;
        CardStar card_star = data.value<CardResponseStruct>().m_card;
        if(!card_star->isKindOf("Jink"))
            return false;
        room->askForUseCard(zhangjiao, "@@leiji", "@leiji");

        return false;
    }
};

ShensuCard::ShensuCard(){
    mute = true;
}

bool ShensuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select->hasSkill("kongcheng") && to_select->isKongcheng())
        return false;

    if(to_select == Self)
        return false;

    return true;
}

void ShensuCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    Slash *slash = new Slash(Card::NoSuitNoColor, 0);
    slash->setSkillName("shensu");
    CardUseStruct use;
    use.card = slash;
    use.from = source;
    use.to = targets;

    room->useCard(use);
}

class ShensuViewAsSkill: public ViewAsSkill{
public:
    ShensuViewAsSkill():ViewAsSkill("shensu"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        if (Sanguosha->currentRoomState()->getCurrentCardUsePattern().endsWith("1"))
            return false;
        else
            return selected.isEmpty() && to_select->isKindOf("EquipCard");
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        Slash *slash = new Slash(Card::NoSuitNoColor, 0);
        slash->deleteLater();
        if (pattern.startsWith("@@shensu") && !player->isCardLimited(slash, Card::MethodUse))
            return true;

        return false;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if(Sanguosha->currentRoomState()->getCurrentCardUsePattern().endsWith("1")){
            if(cards.isEmpty())
                return new ShensuCard;
            else
                return NULL;
        }else{
            if(cards.length() != 1)
                return NULL;

            ShensuCard *card = new ShensuCard;
            card->addSubcards(cards);

            return card;
        }
    }
};

class Shensu: public TriggerSkill{
public:
    Shensu():TriggerSkill("shensu"){
        events << EventPhaseChanging;
        view_as_skill = new ShensuViewAsSkill;
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *xiahouyuan, QVariant &data) const{
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if(change.to == Player::Judge && !xiahouyuan->isSkipped(Player::Judge)
                && !xiahouyuan->isSkipped(Player::Draw)){
            if(Slash::IsAvailable(xiahouyuan) && room->askForUseCard(xiahouyuan, "@@shensu1", "@shensu1", 1)){
                xiahouyuan->skip(Player::Judge);
                xiahouyuan->skip(Player::Draw);
            }
        }else if(Slash::IsAvailable(xiahouyuan) && change.to == Player::Play && !xiahouyuan->isSkipped(Player::Play)){
            if(room->askForUseCard(xiahouyuan, "@@shensu2", "@shensu2", 2)){
                xiahouyuan->skip(Player::Play);
            }
        }
        return false;
    }
};

Jushou::Jushou():PhaseChangeSkill("jushou"){
}

int Jushou::getJushouDrawNum(ServerPlayer *) const{
    return 3;
}

bool Jushou::onPhaseChange(ServerPlayer *target) const{
    if(target->getPhase() == Player::Finish){
        Room *room = target->getRoom();
        if(room->askForSkillInvoke(target, objectName())){
            target->drawCards(getJushouDrawNum(target));
            target->turnOver();

            room->broadcastSkillInvoke("jushou");
        }
    }

    return false;
}

class Liegong:public TriggerSkill{
public:
    Liegong():TriggerSkill("liegong"){
        events << TargetConfirmed << CardFinished;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target && target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        if (event == TargetConfirmed){
            CardUseStruct use = data.value<CardUseStruct>();
            if(!player->isAlive() || player->getPhase() != Player::Play || player != use.from
               || !use.card->isKindOf("Slash"))
                return false;
            int count = 1;
            int mark_n = player->getMark("no_jink" + use.card->toString());
            foreach(ServerPlayer *p, use.to){
                int handcardnum = p->getHandcardNum();
                if ((player->getHp() <= handcardnum || player->getAttackRange() >= handcardnum) &&
                    player->askForSkillInvoke("liegong", QVariant::fromValue(p))){
                    room->broadcastSkillInvoke(objectName());

                    LogMessage log;
                    log.type = "#NoJink";
                    log.from = p;
                    room->sendLog(log);

                    mark_n += count;
                    room->setPlayerMark(player, "no_jink" + use.card->toString(), mark_n);
                }
                count *= 10;
            }

        }else if(event == CardFinished){
            CardUseStruct use = data.value<CardUseStruct>();
            if (!use.card->isKindOf("Slash"))
                return false;
            room->setPlayerMark(player, "no_jink" + use.card->toString(), 0);
        }

        return false;
    }
};

class Kuanggu: public TriggerSkill{
public:
    Kuanggu():TriggerSkill("kuanggu"){
        frequency = Compulsory;
        events << Damage << DamageDone;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(event == DamageDone && damage.from && damage.from->hasSkill("kuanggu") && damage.from->isAlive()){
            ServerPlayer *weiyan = damage.from;
            weiyan->tag["InvokeKuanggu"] = weiyan->distanceTo(damage.to) <= 1;
        }else if(event == Damage && player->hasSkill("kuanggu") && player->isAlive()){
            bool invoke = player->tag.value("InvokeKuanggu", false).toBool();
            player->tag["InvokeKuanggu"] = false;
            if(invoke && player->isWounded()){
                room->broadcastSkillInvoke(objectName());

                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = player;
                log.arg = objectName();
                room->sendLog(log);

                RecoverStruct recover;
                recover.who = player;
                recover.recover = damage.damage;
                room->recover(player, recover);
            }
        }

        return false;
    }
};

class BuquRemove: public TriggerSkill{
public:
    BuquRemove():TriggerSkill("#buqu-remove"){
        events << HpRecover;
    }

    static void Remove(ServerPlayer *zhoutai){
        Room *room = zhoutai->getRoom();
        QList<int> buqu(zhoutai->getPile("buqu"));

        CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, QString(), "buqu", QString());
        int need = 1 - zhoutai->getHp();
        if(need <= 0){
            // clear all the buqu cards
            foreach(int card_id, buqu) {

                LogMessage log;
                log.type = "$BuquRemove";
                log.from = zhoutai;
                log.card_str = Sanguosha->getCard(card_id)->toString();
                room->sendLog(log);

                room->throwCard(Sanguosha->getCard(card_id), reason, NULL);
            }
        }else{
            int to_remove = buqu.length() - need;

            for(int i = 0; i < to_remove; i++){
                room->fillAG(buqu);
                int card_id = room->askForAG(zhoutai, buqu, false, "buqu");

                LogMessage log;
                log.type = "$BuquRemove";
                log.from = zhoutai;
                log.card_str = Sanguosha->getCard(card_id)->toString();
                room->sendLog(log);

                buqu.removeOne(card_id);
                room->throwCard(Sanguosha->getCard(card_id), reason, NULL);
                room->broadcastInvoke("clearAG");
            }
        }
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *zhoutai, QVariant &) const{
        if(zhoutai->getPile("buqu").length() > 0)
            Remove(zhoutai);

        return false;
    }
};

class Buqu: public TriggerSkill{
public:
    Buqu():TriggerSkill("buqu"){
        events << PostHpReduced << AskForPeachesDone;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *zhoutai, QVariant &) const{
        if (event == PostHpReduced && zhoutai->getHp() < 1) {
            if (room->askForSkillInvoke(zhoutai, objectName())) {
                room->setTag("Buqu", zhoutai->objectName());
                room->broadcastSkillInvoke(objectName());
                const QList<int> &buqu = zhoutai->getPile("buqu");

                int need = 1 - zhoutai->getHp(); // the buqu cards that should be turned over
                int n = need - buqu.length();
                if (n > 0) {
                    QList<int> card_ids = room->getNCards(n, false);
                    zhoutai->addToPile("buqu", card_ids);
                }
                const QList<int> &buqunew = zhoutai->getPile("buqu");
                QList<int> duplicate_numbers;

                QSet<int> numbers;
                foreach (int card_id, buqunew) {
                    const Card *card = Sanguosha->getCard(card_id);
                    int number = card->getNumber();

                    if (numbers.contains(number)) {
                        duplicate_numbers << number;
                    } else
                        numbers << number;
                }

                if (duplicate_numbers.isEmpty()) {
                    room->setTag("Buqu", QVariant());
                    return true;
                }
            }
        } else if(event == AskForPeachesDone) {
            const QList<int> &buqu = zhoutai->getPile("buqu");

            if (zhoutai->getHp() > 0)
                return false;
            if (room->getTag("Buqu").toString() != zhoutai->objectName())
                return false;
            room->setTag("Buqu", QVariant());
            QList<int> duplicate_numbers;

            QSet<int> numbers;
            foreach (int card_id, buqu) {
                const Card *card = Sanguosha->getCard(card_id);
                int number = card->getNumber();

                if (numbers.contains(number) && !duplicate_numbers.contains(number)) {
                    duplicate_numbers << number;
                } else
                    numbers << number;
            }

            if (duplicate_numbers.isEmpty()) {
                room->broadcastSkillInvoke(objectName());
                room->setPlayerFlag(zhoutai, "-dying");
                return true;
            } else {
                LogMessage log;
                log.type = "#BuquDuplicate";
                log.from = zhoutai;
                log.arg = QString::number(duplicate_numbers.length());
                room->sendLog(log);

                for (int i = 0; i < duplicate_numbers.length(); i++) {
                    int number = duplicate_numbers.at(i);

                    LogMessage log;
                    log.type = "#BuquDuplicateGroup";
                    log.from = zhoutai;
                    log.arg = QString::number(i+1);

                    if (number == 10)
                        log.arg2 = "10";
                    else {
                        const char *number_string = "-A23456789-JQK";
                        log.arg2 = QString(number_string[number]);
                    }
                    room->sendLog(log);

                    foreach (int card_id, buqu) {
                        const Card *card = Sanguosha->getCard(card_id);
                        if (card->getNumber() == number) {
                            LogMessage log;
                            log.type = "$BuquDuplicateItem";
                            log.from = zhoutai;
                            log.card_str = QString::number(card_id);
                            room->sendLog(log);
                        }
                    }
                }
            }
        }
        return false;
    }
};

class BuquClear: public TriggerSkill {
public:
    BuquClear(): TriggerSkill("#buqu-clear") {
        events << EventLoseSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target && !target->hasSkill("buqu") && target->getPile("buqu").length() > 0;
    }

    virtual bool trigger(TriggerEvent, Room *, ServerPlayer *player, QVariant &) const{
        player->clearOnePrivatePile("buqu");
        return false;
    }
};

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCommandLinkButton>

class Hongyan: public FilterSkill{
public:
    Hongyan():FilterSkill("hongyan"){

    }

    static WrappedCard *changeToHeart(int cardId){
        WrappedCard *new_card = Sanguosha->getWrappedCard(cardId);
        new_card->setSkillName("hongyan");
        new_card->setSuit(Card::Heart);
        new_card->setModified(true);
        return new_card;
    }

    virtual bool viewFilter(const Card *to_select) const{
        return to_select->getSuit() == Card::Spade;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        return changeToHeart(originalCard->getEffectiveId());
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const{
        return -2;
    }
};

TianxiangCard::TianxiangCard()
{
}

void TianxiangCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    room->setPlayerFlag(effect.to, "TianxiangTarget");
    DamageStruct damage = effect.from->tag.value("TianxiangDamage").value<DamageStruct>();
    damage.to = effect.to;
    damage.transfer = true;
    room->damage(damage);
}

class TianxiangViewAsSkill: public OneCardViewAsSkill{
public:
    TianxiangViewAsSkill():OneCardViewAsSkill("tianxiang"){

    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@tianxiang";
    }

    virtual bool viewFilter(const Card *to_select) const{
        return !to_select->isEquipped() && to_select->getSuit() == Card::Heart;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        TianxiangCard *tianxiangCard = new TianxiangCard;
        tianxiangCard->addSubcard(originalCard);
        return tianxiangCard;
    }
};

class Tianxiang: public TriggerSkill{
public:
    Tianxiang():TriggerSkill("tianxiang"){
        events << DamageInflicted;

        view_as_skill = new TianxiangViewAsSkill;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *xiaoqiao, QVariant &data) const{
        if(!xiaoqiao->isKongcheng()){
            DamageStruct damage = data.value<DamageStruct>();

            xiaoqiao->tag["TianxiangDamage"] = QVariant::fromValue(damage);
            if(room->askForUseCard(xiaoqiao, "@@tianxiang", "@tianxiang-card", -1, Card::MethodDiscard)){
                return true;
            }
        }

        return false;
    }
};

class TianxiangDraw: public TriggerSkill{
public:
    TianxiangDraw():TriggerSkill("#tianxiang"){
        events << DamageComplete;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if (player->isAlive() && player->hasFlag("TianxiangTarget") && damage.transfer){
            player->drawCards(player->getLostHp());
            room->setPlayerFlag(player, "-TianxiangTarget");
        }

        return false;
    }
};

GuhuoCard::GuhuoCard(){
    mute = true;
    will_throw = false;
    handling_method = Card::MethodNone;
}

bool GuhuoCard::guhuo(ServerPlayer* yuji, const QString& message) const{
    Room *room = yuji->getRoom();
    room->setTag("Guhuoing", true);
    room->setTag("GuhuoType", this->user_string);

    QList<ServerPlayer *> players = room->getOtherPlayers(yuji);
    QSet<ServerPlayer *> questioned;

    QList<int> used_cards;
    QList<CardsMoveStruct> moves;
    foreach(int card_id, getSubcards()){
        used_cards << card_id;
    }

    foreach(ServerPlayer *player, players){
        if(player->getHp() <= 0){
            LogMessage log;
            log.type = "#GuhuoCannotQuestion";
            log.from = player;
            log.arg = QString::number(player->getHp());
            room->sendLog(log);

            room->setEmotion(player, "no-question");

            continue;
        }

        QString choice = room->askForChoice(player, "guhuo", "noquestion+question");
        if(choice == "question"){
            room->setEmotion(player, "question");
            questioned << player;
        }else
            room->setEmotion(player, "no-question");

        LogMessage log;
        log.type = "#GuhuoQuery";
        log.from = player;
        log.arg = choice;

        room->sendLog(log);
    }
    
    LogMessage log;
    log.type = "$GuhuoResult";
    log.from = yuji;
    log.card_str = QString::number(subcards.first());
    room->sendLog(log);

    bool success = false;
    if(questioned.isEmpty()){
        success = true;

        foreach(ServerPlayer *player, players)
            room->setEmotion(player, ".");
        if(yuji->getPhase() == Player::Play){
            CardMoveReason reason(CardMoveReason::S_REASON_USE, yuji->objectName(), QString(), "guhuo");
            CardsMoveStruct move(used_cards, yuji, NULL, Player::PlaceTable, reason);
            moves.append(move);
            room->moveCardsAtomic(moves, true);
        }
        else if(yuji->getPhase() == Player::NotActive)
            room->moveCardTo(this, yuji, NULL, Player::DiscardPile,
                CardMoveReason(CardMoveReason::S_REASON_RESPONSE, yuji->objectName(), QString(), "guhuo"), true);

    }else{
        const Card *card = Sanguosha->getCard(subcards.first());
        bool real;
        if(user_string == "peach+analeptic")
            real = card->objectName() == yuji->tag["GuhuoSaveSelf"].toString();
        else if (user_string == "slash")
            real = card->objectName().contains("slash");
        else if (user_string == "natural_slash")
            real = card->objectName() == "slash";
        else
            real = card->match(user_string);

        success = real && card->getSuit() == Card::Heart;
        if(success && yuji->getPhase() == Player::Play){
            CardMoveReason reason(CardMoveReason::S_REASON_USE, yuji->objectName(), QString(), "guhuo");
            CardsMoveStruct move(used_cards, yuji, NULL, Player::PlaceTable, reason);
            moves.append(move);
            room->moveCardsAtomic(moves, true);
        }
        else if(success && yuji->getPhase() == Player::NotActive){
            room->moveCardTo(this, yuji, NULL, Player::DiscardPile,
                CardMoveReason(CardMoveReason::S_REASON_RESPONSE, yuji->objectName(), QString(), "guhuo"), true);
        }
        else{
            room->moveCardTo(this, yuji, NULL, Player::DiscardPile,
                CardMoveReason(CardMoveReason::S_REASON_PUT, yuji->objectName(), QString(), "guhuo"), true);
        }
        foreach(ServerPlayer *player, players){
            room->setEmotion(player, ".");

            if(questioned.contains(player)){
                if(real)
                    room->loseHp(player);
                else
                    player->drawCards(1);
            }
        }
    }
    yuji->tag.remove("GuhuoSaveSelf");
    yuji->tag.remove("GuhuoSlash");
    room->setTag("Guhuoing", false);
    room->removeTag("GuhuoType");
    if(!success)
        room->setPlayerFlag(yuji, "guhuo_failed");

    return success;
}

GuhuoDialog *GuhuoDialog::getInstance(const QString &object, bool left, bool right){
    static GuhuoDialog *instance;
    if(instance == NULL || instance->objectName() != object)
        instance = new GuhuoDialog(object, left, right);

    return instance;
}

GuhuoDialog::GuhuoDialog(const QString &object, bool left, bool right):object_name(object)
{
    setObjectName(object);
    setWindowTitle(Sanguosha->translate(object));
    group = new QButtonGroup(this);

    QHBoxLayout *layout = new QHBoxLayout;
    if(left)
        layout->addWidget(createLeft());
    if(right)
        layout->addWidget(createRight());

    setLayout(layout);

    connect(group, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(selectCard(QAbstractButton*)));
}

void GuhuoDialog::popup(){
    //if(ClientInstance->getStatus() != Client::Playing)
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() != CardUseStruct::CARD_USE_REASON_PLAY)
        return;

    foreach(QAbstractButton *button, group->buttons()){
        const Card *card = map[button->objectName()];
        bool enabled = !Self->isCardLimited(card, Card::MethodUse, true) && card->isAvailable(Self);
        button->setEnabled(enabled);
    }

    Self->tag.remove(object_name);
    exec();
}

void GuhuoDialog::selectCard(QAbstractButton *button){
    CardStar card = map.value(button->objectName());
    Self->tag[object_name] = QVariant::fromValue(card);
    if (button->objectName().contains("slash"))
        Self->tag["GuhuoSlash"] = button->objectName();
    emit onButtonClick();
    accept();
}

QGroupBox *GuhuoDialog::createLeft(){
    QGroupBox *box = new QGroupBox;
    box->setTitle(Sanguosha->translate("basic"));

    QVBoxLayout *layout = new QVBoxLayout;

    QList<const Card *> cards = Sanguosha->findChildren<const Card *>();
    foreach(const Card *card, cards){
        if(card->getTypeId() == Card::TypeBasic && !map.contains(card->objectName())
           && !Config.BanPackages.contains(card->getPackage())){
            Card *c = Sanguosha->cloneCard(card->objectName(), Card::NoSuitNoColor, 0);
            c->setParent(this);

            layout->addWidget(createButton(c));

            if (card->objectName() == "slash"  && !Config.BanPackages.contains("maneuvering")) {
                Card *c2 = Sanguosha->cloneCard(card->objectName(), Card::NoSuitNoColor, 0);
                c2->setParent(this);

                layout->addWidget(createButton(c2));
            }
        }
    }

    layout->addStretch();

    box->setLayout(layout);
    return box;
}

QGroupBox *GuhuoDialog::createRight(){
    QGroupBox *box = new QGroupBox(Sanguosha->translate("ndtrick"));
    QHBoxLayout *layout = new QHBoxLayout;

    QGroupBox *box1 = new QGroupBox(Sanguosha->translate("single_target"));
    QVBoxLayout *layout1 = new QVBoxLayout;

    QGroupBox *box2 = new QGroupBox(Sanguosha->translate("multiple_targets"));
    QVBoxLayout *layout2 = new QVBoxLayout;


    QList<const Card *> cards = Sanguosha->findChildren<const Card *>();
    foreach(const Card *card, cards){
        if(card->isNDTrick() && !map.contains(card->objectName())
           && !Config.BanPackages.contains(card->getPackage())){
            Card *c = Sanguosha->cloneCard(card->objectName(), Card::NoSuitNoColor, 0);
            c->setSkillName(object_name);
            c->setParent(this);

            QVBoxLayout *layout = c->isKindOf("SingleTargetTrick") ? layout1 : layout2;
            layout->addWidget(createButton(c));
        }
    }

    box->setLayout(layout);
    box1->setLayout(layout1);
    box2->setLayout(layout2);

    layout1->addStretch();
    layout2->addStretch();

    layout->addWidget(box1);
    layout->addWidget(box2);
    return box;
}

QAbstractButton *GuhuoDialog::createButton(const Card *card){
    if (card->objectName() == "slash" && map.contains(card->objectName()) && !map.contains("natural_slash")) {
        QCommandLinkButton *button = new QCommandLinkButton(Sanguosha->translate("natural_slash"));
        button->setObjectName("natural_slash");
        button->setToolTip(card->getDescription());

        map.insert("natural_slash", card);
        group->addButton(button);

        return button;
    } else {
        QCommandLinkButton *button = new QCommandLinkButton(Sanguosha->translate(card->objectName()));
        button->setObjectName(card->objectName());
        button->setToolTip(card->getDescription());

        map.insert(card->objectName(), card);
        group->addButton(button);

        return button;
    }
}

bool GuhuoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    CardStar card = Self->tag.value("guhuo").value<CardStar>();
    return card && card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, card);
}

bool GuhuoCard::targetFixed() const{
    if(Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE)
    {
        if(!ClientInstance->hasNoTargetResponding()){
            CardStar card = Sanguosha->cloneCard(user_string, NoSuitNoColor, 0);
            Self->tag["guhuo"] = QVariant::fromValue(card);
            return card && card->targetFixed();
        }
        else return true;
    }

    CardStar card = Self->tag.value("guhuo").value<CardStar>();
    return card && card->targetFixed();
}

bool GuhuoCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    CardStar card = Self->tag.value("guhuo").value<CardStar>();
    return card && card->targetsFeasible(targets, Self);
}

const Card *GuhuoCard::validate(const CardUseStruct *card_use) const{
    ServerPlayer *yuji = card_use->from;
    Room *room = yuji->getRoom();

    QString to_guhuo = user_string;
    if (user_string == "slash"
        && Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE) {
        QStringList guhuo_list;
        guhuo_list << "slash";
        if (!Config.BanPackages.contains("maneuvering"))
            guhuo_list << "natural_slash" << "thunder_slash" << "fire_slash";
        to_guhuo = room->askForChoice(yuji, "guhuo_slash", guhuo_list.join("+"));
        yuji->tag["GuhuoSlash"] = QVariant(to_guhuo);
    }
    room->broadcastSkillInvoke("guhuo");

    LogMessage log;
    log.type = card_use->to.isEmpty() ? "#GuhuoNoTarget" : "#Guhuo";
    log.from = yuji;
    log.to = card_use->to;
    log.arg = to_guhuo;
    log.arg2 = "guhuo";

    room->sendLog(log);

    if(guhuo(card_use->from, log.toString())){
        const Card *card = Sanguosha->getCard(subcards.first());
        QString user_str;
        if (to_guhuo == "slash") {
            if (card->isKindOf("Slash"))
                user_str = card->objectName();
            else
                user_str = "slash";
        } else if (to_guhuo == "natural_slash")
            user_str = "slash";
        else
            user_str = to_guhuo;
        Card *use_card = Sanguosha->cloneCard(user_str, card->getSuit(), card->getNumber());
        use_card->setSkillName("guhuo");
        use_card->addSubcard(this);
        /* @todo: verify this...
        CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, card_use->from->objectName());
        room->throwCard(this, reason, NULL); */
        use_card->deleteLater();
        return use_card;
    }else
        return NULL;
}

const Card *GuhuoCard::validateInResponse(ServerPlayer *yuji, bool &continuable) const{
    continuable = true;
    Room *room = yuji->getRoom();
    room->broadcastSkillInvoke("guhuo");

    QString to_guhuo;
    if(user_string == "peach+analeptic") {
        QStringList guhuo_list;
        guhuo_list << "peach";
        if (!Config.BanPackages.contains("maneuvering"))
            guhuo_list << "analeptic";
        to_guhuo = room->askForChoice(yuji, "guhuo_saveself", guhuo_list.join("+"));
        yuji->tag["GuhuoSaveSelf"] = QVariant(to_guhuo);
    }
    else if (user_string == "slash") {
        QStringList guhuo_list;
        guhuo_list << "slash";
        if (!Config.BanPackages.contains("maneuvering"))
            guhuo_list << "natural_slash" << "thunder_slash" << "fire_slash";
        to_guhuo = room->askForChoice(yuji, "guhuo_slash", guhuo_list.join("+"));
        yuji->tag["GuhuoSlash"] = QVariant(to_guhuo);
    }
    else
        to_guhuo = user_string;

    LogMessage log;
    log.type = "#GuhuoNoTarget";
    log.from = yuji;
    log.arg = to_guhuo;
    log.arg2 = "guhuo";
    room->sendLog(log);

    if (guhuo(yuji,log.toString())){
        const Card *card = Sanguosha->getCard(subcards.first());
        QString user_str;
        if (to_guhuo == "slash") {
            if (card->isKindOf("Slash"))
                user_str = card->objectName();
            else
                user_str = "slash";
        } else if (to_guhuo == "natural_slash")
            user_str = "slash";
        else
            user_str = to_guhuo;
        Card *use_card = Sanguosha->cloneCard(user_str, card->getSuit(), card->getNumber());
        use_card->setSkillName("guhuo");
        use_card->deleteLater();
        return use_card;
    }else
        return NULL;
}

class Guhuo: public ViewAsSkill{
public:
    Guhuo():ViewAsSkill("guhuo"){
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        if (player->isKongcheng() || pattern.startsWith(".") || pattern.startsWith("@")) return false;
        for (int i = 0; i < pattern.length(); i++) {
            QChar ch = pattern.at(i);
            if (ch.isUpper() || ch.isDigit()) return false; // This is an extremely dirty hack!! For we need to prevent patterns like 'BasicCard'
        }
        return true;
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        return selected.isEmpty() && !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if (cards.length() != 1) return NULL;
        const Card *originalCard = cards.first();
        if(Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE) {
            GuhuoCard *card = new GuhuoCard;
            card->setUserString(Sanguosha->currentRoomState()->getCurrentCardUsePattern());
            card->addSubcard(originalCard);
            return card;
        }

        CardStar c = Self->tag.value("guhuo").value<CardStar>();
        if(c) {
            GuhuoCard *card = new GuhuoCard;
            if (!c->objectName().contains("slash"))
                card->setUserString(c->objectName());
            else
                card->setUserString(Self->tag["GuhuoSlash"].toString());
            card->addSubcard(originalCard);

            return card;
        }else
            return NULL;
    }

    virtual QDialog *getDialog() const{
        return GuhuoDialog::getInstance("guhuo");
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *card) const{
        if (!card->isKindOf("GuhuoCard"))
            return -2;
        else
            return -1;
    }

    virtual bool isEnabledAtNullification(const ServerPlayer *player) const{
        return !player->isKongcheng();
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng();
    }
};

class CVXiaoqiao: public GameStartSkill{
public:
    CVXiaoqiao():GameStartSkill("cv_xiaoqiao"){
        default_choice = "wz_xiaoqiao";
        sp_convert_skill = true;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        if (target == NULL) return false;
        if (Sanguosha->getBanPackages().contains("sp") && Sanguosha->getBanPackages().contains("hegemony"))
            return false;
        bool canInvoke = ServerInfo.GameMode.endsWith("p") || ServerInfo.GameMode.endsWith("pd") ||
                ServerInfo.GameMode.endsWith("pz");
        return GameStartSkill::triggerable(target) && target->getGeneralName() == "xiaoqiao" && canInvoke;
    }

    virtual void onGameStart(ServerPlayer *player) const{
        if(player->getGeneral()->hasSkill(objectName()) && player->askForSkillInvoke(objectName(), "convert")){
            Room *room = player->getRoom();
            QStringList choicelist;
            if (!Sanguosha->getBanPackages().contains("sp"))
                choicelist << "wz_xiaoqiao";
            if (!Sanguosha->getBanPackages().contains("hegemony"))
                choicelist << "heg_xiaoqiao";
            QString choice = room->askForChoice(player, objectName(), choicelist.join("+"));

            LogMessage log;
            log.type = "#Transfigure";
            log.from = player;
            log.arg = choice;
            room->sendLog(log);
            room->setPlayerProperty(player, "general", choice);
        }
    }
};

WindPackage::WindPackage()
    :Package("wind")
{
    General *xiahouyuan, *caoren, *huangzhong, *weiyan, *zhangjiao, *zhoutai;

    xiahouyuan = new General(this, "xiahouyuan", "wei");
    xiahouyuan->addSkill(new Shensu);

    caoren = new General(this, "caoren", "wei");
    caoren->addSkill(new Jushou);

    huangzhong = new General(this, "huangzhong", "shu");
    huangzhong->addSkill(new Liegong);

    weiyan = new General(this, "weiyan", "shu");
    weiyan->addSkill(new Kuanggu);

    General *xiaoqiao = new General(this, "xiaoqiao", "wu", 3, false);
    xiaoqiao->addSkill(new Tianxiang);
    xiaoqiao->addSkill(new TianxiangDraw);
    xiaoqiao->addSkill(new Hongyan);
    related_skills.insertMulti("tianxiang", "#tianxiang");
    xiaoqiao->addSkill(new CVXiaoqiao);

    zhoutai = new General(this, "zhoutai", "wu");
    zhoutai->addSkill(new Buqu);
    zhoutai->addSkill(new BuquRemove);
    zhoutai->addSkill(new BuquClear);
    related_skills.insertMulti("buqu", "#buqu-remove");
    related_skills.insertMulti("buqu", "#buqu-clear");

    zhangjiao = new General(this, "zhangjiao$", "qun", 3);
    zhangjiao->addSkill(new Guidao);
    zhangjiao->addSkill(new Leiji);
    zhangjiao->addSkill(new Huangtian);

    General *yuji = new General(this, "yuji", "qun", 3);
    yuji->addSkill(new Guhuo);

    addMetaObject<ShensuCard>();
    addMetaObject<TianxiangCard>();
    addMetaObject<GuidaoCard>();
    addMetaObject<HuangtianCard>();
    addMetaObject<LeijiCard>();
    addMetaObject<GuhuoCard>();

    skills << new HuangtianViewAsSkill;

}

ADD_PACKAGE(Wind)