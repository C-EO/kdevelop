/***************************************************************************
                          ctagsdialog_impl.h
                          --------------------
    begin                : Wed April 26 2001
    copyright            : (C) 2001 by rokrau, the kdevelop-team
    email                : rokrau@yahoo.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef SEARCHTAGSDIALOGIMPL_H
#define SEARCHTAGSDIALOGIMPL_H
#include "cctags.h"
#include "ctagsdialog.h"

class searchTagsDialogImpl : public searchTagsDialog
{ 
    Q_OBJECT

public:
    enum tagType {
      all,
      file,
      definition,
      declaration
    };
    searchTagsDialogImpl( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~searchTagsDialogImpl();
    void setTagType(tagType type);
    void setSearchResult(const CTagList& taglist);
    void gotoTag(const CTag* tag);
    void searchTags(const QString& text, int* ntags, const CTagList** taglist=0L) ;
public slots:
    void slotLBItemSelected(int i);
    void slotClear();
    void slotSearchTag();
    void slotGotoFile(QString text);
    void slotGotoDefinition(QString text);
    void slotGotoDeclaration(QString text);
    void slotGotoTagType(tagType type, QString text);
signals:
    void switchToFile(const QString& filename, int line);

private:
    CTagList m_currentTagList;
};

#endif // SEARCHTAGSDIALOGIMPL_H
