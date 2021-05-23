/*
    Copyright David Nolden  <david.nolden.kdevelop@art-master.de>
    Copyright 2010 Milian Wolff <mail@milianw.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "workingsetwidget.h"
#include "debug_workingset.h"

#include <sublime/area.h>

#include "workingsetcontroller.h"
#include "workingset.h"
#include "workingsettoolbutton.h"

#include <core.h>

using namespace KDevelop;

WorkingSet* getSet(const QString& id)
{
    if (id.isEmpty()) {
        return nullptr;
    }

    return Core::self()->workingSetControllerInternal()->workingSet(id);
}

WorkingSetWidget::WorkingSetWidget(Sublime::Area* area, QWidget* parent)
    : WorkingSetToolButton(parent, nullptr)
    , m_area(area)
{
    //Queued connect so the change is already applied to the area when we start processing
    connect(m_area.data(), &Sublime::Area::changingWorkingSet, this,
            &WorkingSetWidget::changingWorkingSet, Qt::QueuedConnection);

    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored));

    changingWorkingSet(m_area, nullptr, QString(), area->workingSet());
}

void WorkingSetWidget::setVisible( bool visible )
{
    // never show empty working sets
    // TODO: I overloaded this only because hide() in the ctor does not work, other ideas?
    // It's not that it doesn't work from the constructor, it's that the value changes when the button is added on a layout.
    QWidget::setVisible( visible && (workingSet() && !workingSet()->isEmpty()) );
}

void WorkingSetWidget::changingWorkingSet(Sublime::Area* area, Sublime::Area* /*oldArea*/, const QString& /*from*/, const QString& newSet)
{
    qCDebug(WORKINGSET) << "re-creating widget" << m_area;

    Q_ASSERT(area == m_area);
    Q_UNUSED(area);

    if (workingSet()) {
        disconnect(workingSet(), &WorkingSet::setChangedSignificantly,
                   this, &WorkingSetWidget::setChangedSignificantly);
    }

    WorkingSet* set = getSet(newSet);
    setWorkingSet(set);

    if (set) {
        connect(set, &WorkingSet::setChangedSignificantly,
                     this, &WorkingSetWidget::setChangedSignificantly);
    }
    setVisible(set && !set->isEmpty());
}

void WorkingSetWidget::setChangedSignificantly()
{
    setVisible(!workingSet()->isEmpty());
}

