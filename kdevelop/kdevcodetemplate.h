/* $Id$
 *
 *  Copyright (C) 2002 Roberto Raggi (raggi@cli.di.unipi.it)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */
#ifndef kdevcodetemplate_h
#define kdevcodetemplate_h

#include <qobject.h>
#include <qasciidict.h>

namespace Kate{
    class Document;
    class View;
}

struct Template{
    QString description;
    QString code;
};

class KDevCodeTemplate: public QObject{
    Q_OBJECT
protected:
    KDevCodeTemplate();

public:
    virtual ~KDevCodeTemplate();

    void clearTemplates();
    void addTemplate( const QString&, const QString&, const QString& );
    void removeTemplate( const QString& );
    QAsciiDictIterator<Template> templates() const;

    void expandTemplate( Kate::View* );

    static KDevCodeTemplate* self();

public slots:
    void save();
    void load();

protected:
    void insertChars( Kate::View*, const QString& );

private:
    QAsciiDict<Template> m_templates;
    static KDevCodeTemplate* m_pSelf;
};

#endif
