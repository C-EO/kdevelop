/* KDevelop QMake Support
 *
 * Copyright 2006 Andreas Pakulat <apaku@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef QMAKEPROJECTFILE_H
#define QMAKEPROJECTFILE_H

#include "qmakefile.h"

class QMakeMkSpecs;

template <typename T> class QList;

class QMakeMkSpecs;
class QMakeCache;

namespace KDevelop {
class IProject;
}

class QMakeProjectFile : public QMakeFile
{
public:
    using DefinePair = QPair<QString, QString>;
    static const QStringList FileVariables;

    explicit QMakeProjectFile( const QString& projectfile );
    ~QMakeProjectFile() override;

    bool read() override;

    QStringList subProjects() const;
    bool hasSubProject(const QString& file) const;

    QStringList files() const;
    QStringList filesForTarget( const QString& ) const;
    QStringList includeDirectories() const;
    QStringList frameworkDirectories() const;
    QStringList extraArguments() const;

    QStringList targets() const;

    QString getTemplate() const;

    void setMkSpecs( QMakeMkSpecs* mkspecs );
    void setOwnMkSpecs(bool own);
    QMakeMkSpecs* mkSpecs() const;
    void setQMakeCache( QMakeCache* cache );
    QMakeCache* qmakeCache() const;
    QStringList resolveVariable(const QString& variable, VariableInfo::VariableType type) const override;
    QList< DefinePair > defines() const;

    /// current pwd, e.g. absoluteDir even for included files
    virtual QString pwd() const;
    /// path to build dir for the current .pro file
    virtual QString outPwd() const;
    /// path to dir of current .pro file
    virtual QString proFilePwd() const;
    /// path to current .pro file
    virtual QString proFile() const;

private:
    void addPathsForVariable(const QString& variable, QStringList* list, const QString& base = {}) const;

    QMakeMkSpecs* m_mkspecs;
    QMakeCache* m_cache;
    static QHash<QString, QHash<QString, QString> > m_qmakeQueryCache;
    QString m_qtIncludeDir;
    QString m_qtVersion;
    // On OS X, QT_INSTALL_LIBS is typically a framework directory and should thus be added to the framework search path
    QString m_qtLibDir;
    bool m_ownMkSpecs = false;
};

#endif

