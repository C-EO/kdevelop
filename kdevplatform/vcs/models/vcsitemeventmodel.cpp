/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "vcsitemeventmodel.h"

#include <QIcon>
#include <QModelIndex>
#include <QVariant>
#include <QList>
#include <QUrl>
#include <QMimeType>
#include <QMimeDatabase>

#include <KLocalizedString>

#include "../vcsrevision.h"
#include "../vcsevent.h"

namespace KDevelop
{

VcsItemEventModel::VcsItemEventModel( QObject* parent )
    : QStandardItemModel( parent )
{
    setColumnCount(2);
}

VcsItemEventModel::~VcsItemEventModel()
{}

void VcsItemEventModel::addItemEvents( const QList<KDevelop::VcsItemEvent>& list )
{
    if(rowCount()==0)
        setColumnCount(2);

    bool copySource = false;
    QMimeDatabase mimeDataBase;
    for (const KDevelop::VcsItemEvent& ev : list) {

        KDevelop::VcsItemEvent::Actions act = ev.actions();
        QStringList actionStrings;
        if( act & KDevelop::VcsItemEvent::Added )
            actionStrings << i18nc("@item", "Added");
        else if( act & KDevelop::VcsItemEvent::Deleted )
            actionStrings << i18nc("@item", "Deleted");
        else if( act & KDevelop::VcsItemEvent::Modified )
            actionStrings << i18nc("@item", "Modified");
        else if( act & KDevelop::VcsItemEvent::Copied )
            actionStrings << i18nc("@item", "Copied");
        else if( act & KDevelop::VcsItemEvent::Replaced )
            actionStrings << i18nc("@item", "Replaced");
        QUrl repoUrl = QUrl::fromLocalFile(ev.repositoryLocation());
        QMimeType mime = repoUrl.isLocalFile()
                ? mimeDataBase.mimeTypeForFile(repoUrl.toLocalFile(), QMimeDatabase::MatchExtension)
                : mimeDataBase.mimeTypeForUrl(repoUrl);
        QList<QStandardItem*> rowItems{
            new QStandardItem(QIcon::fromTheme(mime.iconName()), ev.repositoryLocation()),
            new QStandardItem(actionStrings.join(i18nc("separates an action list", ", "))),
        };
        QString loc = ev.repositoryCopySourceLocation();
        if(!loc.isEmpty()) { //according to the documentation, those are optional. don't force them on the UI
            rowItems << new QStandardItem(ev.repositoryCopySourceLocation());
            VcsRevision rev = ev.repositoryCopySourceRevision();
            if(rev.revisionType()!=VcsRevision::Invalid) {
                rowItems << new QStandardItem(ev.repositoryCopySourceRevision().revisionValue().toString());
            }
            copySource = true;
        }

        rowItems.first()->setData(QVariant::fromValue(ev));
        appendRow(rowItems);
    }
    if(copySource)
        setColumnCount(4);
}

QVariant VcsItemEventModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role==Qt::DisplayRole) {
        switch(section)
        {
            case 0: return i18nc("@title:column", "Location");
            case 1: return i18nc("@title:column", "Actions");
            case 2: return i18nc("@title:column", "Source Location");
            case 3: return i18nc("@title:column", "Source Revision");
        }
    }
    return QStandardItemModel::headerData(section, orientation, role);
}


KDevelop::VcsItemEvent VcsItemEventModel::itemEventForIndex( const QModelIndex& idx ) const
{
    return itemFromIndex(idx)->data().value<VcsItemEvent>();
}

}

#include "moc_vcsitemeventmodel.cpp"
