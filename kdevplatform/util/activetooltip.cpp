/* This file is part of the KDE project
   Copyright 2007 Vladimir Prus
   Copyright 2009-2010 David Nolden

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "activetooltip.h"
#include "debug.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QPalette>
#include <QPoint>
#include <QPointer>
#include <QStyleOption>
#include <QStylePainter>

#include <limits>

using namespace KDevelop;

namespace {

class ActiveToolTipManager : public QObject
{
    Q_OBJECT
public Q_SLOTS:
    void doVisibility();

public:
    typedef QMultiMap<float, QPair<QPointer<ActiveToolTip>, QString> > ToolTipPriorityMap;
    ToolTipPriorityMap registeredToolTips;
};

ActiveToolTipManager* manager()
{
    static ActiveToolTipManager m;
    return &m;
}

QWidget* masterWidget(QWidget* w)
{
    while (w && w->parent() && qobject_cast<QWidget*>(w->parent())) {
        w = qobject_cast<QWidget*>(w->parent());
    }
    return w;
}

void ActiveToolTipManager::doVisibility()
{
    bool exclusive = false;
    int lastBottomPosition = -1;
    int lastLeftPosition = -1;
    QRect fullGeometry; //Geometry of all visible tooltips together

    for (auto it = registeredToolTips.constBegin(); it != registeredToolTips.constEnd(); ++it) {
        QPointer< ActiveToolTip > w = (*it).first;
        if (w) {
            if (exclusive) {
                (w.data())->hide();
            } else {
                QRect geom = (w.data())->geometry();
                if ((w.data())->geometry().top() < lastBottomPosition) {
                    geom.moveTop(lastBottomPosition);
                }
                if (lastLeftPosition != -1) {
                    geom.moveLeft(lastLeftPosition);
                }

                (w.data())->setGeometry(geom);

                lastBottomPosition = (w.data())->geometry().bottom();
                lastLeftPosition = (w.data())->geometry().left();

                if (it == registeredToolTips.constBegin()) {
                    fullGeometry = (w.data())->geometry();
                } else {
                    fullGeometry = fullGeometry.united((w.data())->geometry());
                }
            }
            if (it.key() == 0) {
                exclusive = true;
            }
        }
    }
    if (!fullGeometry.isEmpty()) {
        QRect oldFullGeometry = fullGeometry;
        QRect screenGeometry = QApplication::desktop()->screenGeometry(fullGeometry.topLeft());
        if (fullGeometry.bottom() > screenGeometry.bottom()) {
            //Move up, avoiding the mouse-cursor
            fullGeometry.moveBottom(fullGeometry.top()-10);
            if (fullGeometry.adjusted(-20, -20, 20, 20).contains(QCursor::pos())) {
                fullGeometry.moveBottom(QCursor::pos().y() - 20);
            }
        }
        if (fullGeometry.right() > screenGeometry.right()) {
            //Move to left, avoiding the mouse-cursor
            fullGeometry.moveRight(fullGeometry.left()-10);
            if (fullGeometry.adjusted(-20, -20, 20, 20).contains(QCursor::pos())) {
                fullGeometry.moveRight(QCursor::pos().x() - 20);
            }
        }
        // Now fit this to screen
        if (fullGeometry.left() < 0) {
            fullGeometry.setLeft(0);
        }
        if (fullGeometry.top() < 0) {
            fullGeometry.setTop(0);
        }

        QPoint offset = fullGeometry.topLeft() - oldFullGeometry.topLeft();
        if (!offset.isNull()) {
            for (auto it = registeredToolTips.constBegin(); it != registeredToolTips.constEnd(); ++it)
                if (it->first) {
                    it->first.data()->move(it->first.data()->pos() + offset);
                }
        }
    }

    //Always include the mouse cursor in the full geometry, to avoid
    //closing the tooltip inexpectedly
    fullGeometry = fullGeometry.united(QRect(QCursor::pos(), QCursor::pos()));

    //Set bounding geometry, and remove old tooltips
    for (auto it = registeredToolTips.begin(); it != registeredToolTips.end(); ) {
        if (!it->first) {
            it = registeredToolTips.erase(it);
        } else {
            it->first.data()->setBoundingGeometry(fullGeometry);
            ++it;
        }
    }

    //Final step: Show tooltips
    foreach (const auto& tooltip, registeredToolTips) {
        if (tooltip.first.data() && masterWidget(tooltip.first.data())->isActiveWindow()) {
            tooltip.first.data()->show();
        }
        if (exclusive) {
            break;
        }
    }
}

}

namespace KDevelop {
class ActiveToolTipPrivate
{
public:
    QRect rect_;
    QRect handleRect_;
    QList<QPointer<QObject> > friendWidgets_;
};
}

ActiveToolTip::ActiveToolTip(QWidget *parent, const QPoint& position)
    : QWidget(parent, Qt::ToolTip)
    , d(new ActiveToolTipPrivate)
{
    Q_ASSERT(parent);
    setMouseTracking(true);
    d->rect_ = QRect(position, position);
    d->rect_.adjust(-10, -10, 10, 10);
    move(position);

    QPalette p;

    // adjust background color to use tooltip colors
    p.setColor(backgroundRole(), p.color(QPalette::ToolTipBase));
    p.setColor(QPalette::Base, p.color(QPalette::ToolTipBase));

    // adjust foreground color to use tooltip colors
    p.setColor(foregroundRole(), p.color(QPalette::ToolTipText));
    p.setColor(QPalette::Text, p.color(QPalette::ToolTipText));
    setPalette(p);

    setWindowFlags(Qt::WindowDoesNotAcceptFocus | windowFlags());

    qApp->installEventFilter(this);
}

ActiveToolTip::~ActiveToolTip() = default;

bool ActiveToolTip::eventFilter(QObject *object, QEvent *e)
{
    switch (e->type()) {
    case QEvent::MouseMove:
        if (underMouse() || insideThis(object)) {
            return false;
        } else {
            QPoint globalPos = static_cast<QMouseEvent*>(e)->globalPos();
            QRect mergedRegion = d->rect_.united(d->handleRect_);

            if (mergedRegion.contains(globalPos)) {
                return false;
            }
            close();
        }
        break;

    case QEvent::WindowActivate:
        if (insideThis(object)) {
            return false;
        }
        close();
        break;

    case QEvent::WindowBlocked:
        // Modal dialog activated somewhere, it is the only case where a cursor
        // move may be missed and the popup has to be force-closed
        close();
        break;

    default:
        break;
    }
    return false;
}

void ActiveToolTip::addFriendWidget(QWidget* widget)
{
    d->friendWidgets_.append((QObject*)widget);
}

bool ActiveToolTip::insideThis(QObject* object)
{
    while (object) {
        if (dynamic_cast<QMenu*>(object)) {
            return true;
        }

        if (object == this || object == (QObject*)this->windowHandle() || d->friendWidgets_.contains(object)) {
            return true;
        }
        object = object->parent();
    }

    // If the object clicked is inside a QQuickWidget, its parent is null even
    // if it is part of a tool-tip. This check ensures that a tool-tip is never
    // closed while the mouse is in it
    return underMouse();
}

void ActiveToolTip::showEvent(QShowEvent*)
{
    adjustRect();
}

void ActiveToolTip::resizeEvent(QResizeEvent*)
{
    adjustRect();

    // set mask from style
    QStyleOptionFrame opt;
    opt.init(this);

    QStyleHintReturnMask mask;
    if (style()->styleHint( QStyle::SH_ToolTip_Mask, &opt, this, &mask ) && !mask.region.isEmpty()) {
        setMask( mask.region );
    }

    emit resized();
}

void ActiveToolTip::paintEvent(QPaintEvent* event)
{
    QStylePainter painter( this );
    painter.setClipRegion( event->region() );
    QStyleOptionFrame opt;
    opt.init(this);
    painter.drawPrimitive(QStyle::PE_PanelTipLabel, opt);
}

void ActiveToolTip::setHandleRect(const QRect& rect)
{
    d->handleRect_= rect;
}

void ActiveToolTip::adjustRect()
{
    // For tooltip widget, geometry() returns global coordinates.
    QRect r = geometry();
    r.adjust(-10, -10, 10, 10);
    d->rect_ = r;
}

void ActiveToolTip::setBoundingGeometry(const QRect& geometry)
{
    d->rect_ = geometry;
    d->rect_.adjust(-10, -10, 10, 10);
}

void ActiveToolTip::showToolTip(ActiveToolTip* tooltip, float priority, const QString& uniqueId)
{
    auto& registeredToolTips = manager()->registeredToolTips;
    if (!uniqueId.isEmpty()) {
        foreach (const auto& tooltip, registeredToolTips) {
            if (tooltip.second == uniqueId) {
                delete tooltip.first.data();
            }
        }
    }

    registeredToolTips.insert(priority, qMakePair(QPointer<ActiveToolTip>(tooltip), uniqueId));

    connect(tooltip, &ActiveToolTip::resized,
            manager(), &ActiveToolTipManager::doVisibility);
    QMetaObject::invokeMethod(manager(), "doVisibility", Qt::QueuedConnection);
}


void ActiveToolTip::closeEvent(QCloseEvent* event)
{
    QWidget::closeEvent(event);
    deleteLater();
}

#include "activetooltip.moc"