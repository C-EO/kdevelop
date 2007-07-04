/***************************************************************************
 *   Copyright (C) 2007 by Dukju Ahn                                       *
 *   dukjuahn@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SVNDIFFWIDGETS_H
#define SVNDIFFWIDGETS_H

#include <kdialog.h>
#include "ui_uidiff_option_dlg.h"

class SvnRevisionWidget;
class KUrl;
namespace SvnUtils
{
    class SvnRevision;
}

/**
 * Choose revisions and extra options for peg diff of given path. (ie. one path is given
 * and two revisions are specified. Diff between two revision for one path). To see the
 * concept of peg revision, consult online subversion book.
 */
class SvnPegDiffDialog : public KDialog
{
    Q_OBJECT
public:
    explicit SvnPegDiffDialog( QWidget *parent = 0 );
    ~SvnPegDiffDialog();

    void setUrl( const KUrl& url );

    SvnUtils::SvnRevision startRev();
    SvnUtils::SvnRevision endRev();
    bool recurse();
    bool noDiffDeleted();
    bool ignoreContentType();

private:
    Ui::SvnPegDiffDialogWidget ui;
    SvnRevisionWidget *m_startRev;
    SvnRevisionWidget *m_endRev;
};

#endif
