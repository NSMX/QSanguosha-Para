#include "clientlogbox.h"
#include "settings.h"
#include "engine.h"
#include "clientplayer.h"
#include "client.h"
#include "roomscene.h"

#include <QPalette>

ClientLogBox::ClientLogBox(QWidget *parent) :
    QTextEdit(parent)
{
    setReadOnly(true);
}

void ClientLogBox::appendLog(
        const QString &type,
        const QString &from_general,
        const QStringList &tos,
        QString card_str,
        QString arg,
        QString arg2)
{
    if (type == "$AppendSeparator") {
        append(tr("<font color='white'>------------------------------</font>"));
        return;
    }
    QString from;
    if(!from_general.isEmpty()){
        from = ClientInstance->getPlayerName(from_general);
        from = bold(from, Qt::green);
    }

    QString to;
    if(!tos.isEmpty()){
        QStringList to_list;
        foreach(QString to, tos)
            to_list << ClientInstance->getPlayerName(to);
        to = to_list.join(", ");
        arg = Sanguosha->translate(arg);

        to = bold(to, Qt::red);
    }

    QString log;

    if(type.startsWith("$")){
        QString log_name;
        foreach(QString one_card, card_str.split("+")){
            const Card *card;
            if(type == "$JudgeResult" || type == "$PasteCard")
                card = Sanguosha->getCard(one_card.toInt());
            else
                card = Sanguosha->getEngineCard(one_card.toInt());
            if(log_name.isEmpty())
                log_name = card->getLogName();
            else
                log_name += ", " + card->getLogName();
        }
        log_name = bold(log_name, Qt::yellow);

        log = Sanguosha->translate(type);
        log.replace("%from", from);
        log.replace("%to", to);
        log.replace("%card", log_name);

        if(!arg2.isEmpty()){
            arg2 = bold(Sanguosha->translate(arg2), Qt::yellow);
            log.replace("%arg2", arg2);
        }

        if(!arg.isEmpty()){
            arg = bold(Sanguosha->translate(arg), Qt::yellow);
            log.replace("%arg", arg);
        }

        log = QString("<font color='%2'>%1</font>").arg(log).arg(Config.TextEditColor.name());
        append(log);

        return;
    }

    if(!card_str.isEmpty() && !from_general.isEmpty()){
        // do Indicator animation
        foreach(QString to, tos){
            RoomSceneInstance->showIndicator(from_general, to);
        }

        const Card *card = Card::Parse(card_str);
        if(card == NULL)
            return;
        QString card_name = card->getLogName();
        card_name = bold(card_name, Qt::yellow);

        QString reason = tr("using");
        if (type.endsWith("_Resp"))
            reason = tr("playing");
        if (type.endsWith("_Recast"))
            reason = tr("recasting");

        if(card->isVirtualCard()){
            QString skill_name = Sanguosha->translate(card->getSkillName());
            skill_name = bold(skill_name, Qt::yellow);

            QList<int> card_ids = card->getSubcards();
            QStringList subcard_list;
            foreach(int card_id, card_ids){
                const Card *subcard = Sanguosha->getCard(card_id);
                subcard_list << bold(subcard->getLogName(), Qt::yellow);
            }

            QString subcard_str = subcard_list.join(", ");
            if(card->getTypeId() == Card::TypeSkill){
                const SkillCard *skill_card = qobject_cast<const SkillCard *>(card);
                if(subcard_list.isEmpty() || !skill_card->willThrow())
                    log = tr("%from use skill [%1]").arg(skill_name);
                else{
                    if(card->isKindOf("DummyCard"))
                        skill_name = bold(Sanguosha->translate("free-discard"), Qt::yellow);
                    log = tr("%from use skill [%1], and the cost is %2").arg(skill_name).arg(subcard_str);
                }
            }else{
                if(subcard_list.isEmpty())
                    log = tr("%from use skill [%1], %3 [%2]").arg(skill_name).arg(card_name).arg(reason);
                else
                    log = tr("%from use skill [%1] %4 %2 as %3")
                          .arg(skill_name)
                          .arg(subcard_str)
                          .arg(card_name)
                          .arg(reason);
            }

            delete card;
        }else if(card->isModified() && card->getSkillName() != QString()){
            const Card *real = Sanguosha->getEngineCard(card->getEffectiveId());
            QString skill_name = Sanguosha->translate(card->getSkillName());
            skill_name = bold(skill_name, Qt::yellow);

            QString subcard_str = bold(real->getLogName(), Qt::yellow);

            log = tr("%from use skill [%1] %4 %2 as %3")
                    .arg(skill_name)
                    .arg(subcard_str)
                    .arg(card_name)
                    .arg(reason);
        }
        else
            log = tr("%from %2 %1").arg(card_name).arg(reason);

        if(!to.isEmpty())
            log.append(tr(", target is %to"));
    }else
        log = Sanguosha->translate(type);

    log.replace("%from", from);
    log.replace("%to", to);

    if(!arg2.isEmpty()){
        arg2 = bold(Sanguosha->translate(arg2), Qt::yellow);
        log.replace("%arg2", arg2);
    }

    if(!arg.isEmpty()){
        arg = bold(Sanguosha->translate(arg), Qt::yellow);
        log.replace("%arg", arg);
    }

    log = QString("<font color='%2'>%1</font>").arg(log).arg(Config.TextEditColor.name());

    append(log);
}

QString ClientLogBox::bold(const QString &str, QColor color) const{
    return QString("<font color='%1'><b>%2</b></font>")
            .arg(color.name()).arg(str);
}

void ClientLogBox::appendLog(const QString &log_str){
    QRegExp rx("([#$]\\w+):(\\w*)->([+\\w]*):(.*):(@?\\w*):(\\w*)");

    if(!rx.exactMatch(log_str)){
        append(tr("Log string is not well formatted: %1").arg(log_str));
        return;
    }

    QStringList texts = rx.capturedTexts();   

    QString type = texts.at(1);
    QString from = texts.at(2);
    QStringList tos;
    if(!texts.at(3).isEmpty())
        tos = texts.at(3).split("+");
    QString card_str = texts.at(4);
    QString arg = texts.at(5);
    QString arg2 = texts.at(6);

    appendLog(type, from, tos, card_str, arg, arg2);
}

void ClientLogBox::append(const QString &text)
{
    QTextEdit::append(QString("<p style=\"margin:3px 2px; line-height:120%;\">%1</p>").arg(text));
}