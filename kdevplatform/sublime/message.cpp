/*
    SPDX-FileCopyrightText: 2012-2013 Dominik Haumann <dhaumann@kde.org>
    SPDX-FileCopyrightText: 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "message.h"

namespace Sublime {

class MessagePrivate
{
public:
    QVector<QAction*> actions;
    QString text;
    QIcon icon;
    int autoHideDelay = -1;
    int priority = 0;
    Message::MessageType messageType;
    bool wordWrap = true;
};


Message::Message(const QString& richtext, MessageType type)
    : d(new MessagePrivate())
{
    d->messageType = type;
    d->text = richtext;
}

Message::~Message()
{
    emit closed(this);
}

QString Message::text() const
{
    return d->text;
}

void Message::setText(const QString& text)
{
    if (d->text == text) {
        return;
    }

    d->text = text;
    emit textChanged(text);
}

void Message::setIcon(const QIcon& icon)
{
    d->icon = icon;
    emit iconChanged(d->icon);
}

QIcon Message::icon() const
{
    return d->icon;
}

Message::MessageType Message::messageType() const
{
    return d->messageType;
}

void Message::addAction(QAction* action, bool closeOnTrigger)
{
    // make sure this is the parent, so all actions are deleted in the destructor
    action->setParent(this);
    d->actions.append(action);

    // call close if wanted
    if (closeOnTrigger) {
        connect(action, &QAction::triggered,
                this, &QObject::deleteLater);
    }
}

QVector<QAction*> Message::actions() const
{
    return d->actions;
}

void Message::setAutoHide(int delay)
{
    d->autoHideDelay = delay;
}

int Message::autoHide() const
{
    return d->autoHideDelay;
}

void Message::setWordWrap(bool wordWrap)
{
    d->wordWrap = wordWrap;
}

bool Message::wordWrap() const
{
    return d->wordWrap;
}

void Message::setPriority(int priority)
{
    d->priority = priority;
}

int Message::priority() const
{
    return d->priority;
}

}

#include "moc_message.cpp"
