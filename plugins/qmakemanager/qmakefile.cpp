/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "qmakefile.h"

#include <QDir>
#include <QFileInfo>

#include <debug.h>
#include "parser/ast.h"
#include "qmakedriver.h"

#define ifDebug(x)

//@TODO: Make the globbing stuff work with drives on win32

void resolveShellGlobbingInternal(QStringList& entries, const QStringList& segments, const QFileInfo& match, QDir& dir,
                                  int offset);

QStringList resolveShellGlobbingInternal(const QStringList& segments, QDir& dir, int offset = 0)
{
    if (offset >= segments.size()) {
        return QStringList();
    }

    const QString& pathPattern = segments.at(offset);

    QStringList entries;
    if (pathPattern.contains(QLatin1Char('*')) ||
        pathPattern.contains(QLatin1Char('?')) ||
        pathPattern.contains(QLatin1Char('['))) {
        // pattern contains globbing chars
        foreach (const QFileInfo& match, dir.entryInfoList(QStringList() << pathPattern,
                                                           QDir::AllEntries | QDir::NoDotAndDotDot, QDir::Unsorted)) {
            resolveShellGlobbingInternal(entries, segments, match, dir, offset);
        }
    } else {
        // pattern is "simple" hence be fast, but make sure the file exists
        QFileInfo info(dir.filePath(pathPattern));
        if (info.exists()) {
            resolveShellGlobbingInternal(entries, segments, info, dir, offset);
        }
    }

    return entries;
}

void resolveShellGlobbingInternal(QStringList& entries, const QStringList& segments, const QFileInfo& match, QDir& dir,
                                  int offset)
{
    if (match.isDir() && offset + 1 < segments.size()) {
        dir.cd(match.fileName());
        entries += resolveShellGlobbingInternal(segments, dir, offset + 1);
        dir.cdUp();
    } else {
        entries << match.canonicalFilePath();
    }
}

QStringList resolveShellGlobbingInternal(const QString& pattern, const QString& dir)
{
    if (pattern.isEmpty()) {
        return QStringList();
    }

    QDir dir_(pattern.startsWith(QLatin1Char('/')) ? QStringLiteral("/") : dir);

    // break up pattern into path segments
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    return resolveShellGlobbingInternal(pattern.split(QLatin1Char('/'), Qt::SkipEmptyParts), dir_);
#else
    return resolveShellGlobbingInternal(pattern.split(QLatin1Char('/'), QString::SkipEmptyParts), dir_);
#endif
}

QMakeFile::QMakeFile(QString file)
    : m_ast(nullptr)
    , m_projectFile(std::move(file))
    , m_project(nullptr)
{
    Q_ASSERT(!m_projectFile.isEmpty());
}

bool QMakeFile::read()
{
    Q_ASSERT(!m_projectFile.isEmpty());
    QFileInfo fi(m_projectFile);
    ifDebug(qCDebug(KDEV_QMAKE) << "Is" << m_projectFile << "a dir?" << fi.isDir();) if (fi.isDir())
    {
        QDir dir(m_projectFile);
        QStringList l = dir.entryList(QStringList() << QStringLiteral("*.pro"));

        QString projectfile;

        if (!l.count() || (l.count() && l.indexOf(fi.baseName() + QLatin1String(".pro")) != -1)) {
            projectfile = fi.baseName() + QLatin1String(".pro");
        } else {
            projectfile = l.first();
        }
        m_projectFile += QLatin1Char('/') + projectfile;
    }
    QMake::Driver d;
    d.readFile(m_projectFile);

    if (!d.parse(&m_ast)) {
        qCWarning(KDEV_QMAKE) << "Couldn't parse project:" << m_projectFile;
        delete m_ast;
        m_ast = nullptr;
        m_projectFile = QString();
        return false;
    } else {
        ifDebug(qCDebug(KDEV_QMAKE) << "found ast:" << m_ast->statements.count();) QMakeFileVisitor visitor(this, this);
        /// TODO: cleanup, re-use m_variableValues directly in the visitor
        visitor.setVariables(m_variableValues);
        m_variableValues = visitor.visitFile(m_ast);
        ifDebug(qCDebug(KDEV_QMAKE) << "Variables found:" << m_variableValues;)
    }
    return true;
}

QMakeFile::~QMakeFile()
{
    delete m_ast;
    m_ast = nullptr;
}

QString QMakeFile::absoluteDir() const
{
    return QFileInfo(m_projectFile).absoluteDir().canonicalPath();
}

QString QMakeFile::absoluteFile() const
{
    return m_projectFile;
}

QMake::ProjectAST* QMakeFile::ast() const
{
    return m_ast;
}

QStringList QMakeFile::variables() const
{
    return m_variableValues.keys();
}

QStringList QMakeFile::variableValues(const QString& variable) const
{
    return m_variableValues.value(variable, QStringList());
}

bool QMakeFile::containsVariable(const QString& variable) const
{
    return m_variableValues.contains(variable);
}

QMakeFile::VariableMap QMakeFile::variableMap() const
{
    return m_variableValues;
}

QStringList QMakeFile::resolveVariable(const QString& variable, VariableInfo::VariableType type) const
{
    if (type == VariableInfo::QMakeVariable) {
        const auto variableValueIt = m_variableValues.find(variable);
        if (variableValueIt != m_variableValues.end()) {
            return *variableValueIt;
        }
    }

    qCWarning(KDEV_QMAKE) << "unresolved variable:" << variable << "type:" << type;
    return QStringList();
}

QStringList QMakeFile::resolveShellGlobbing(const QString& pattern, const QString& base) const
{
    return resolveShellGlobbingInternal(pattern, base.isEmpty() ? absoluteDir() : base);
}

QString QMakeFile::resolveToSingleFileName(const QString& file, const QString& base) const
{
    QStringList l = resolveFileName(file, base);
    if (l.isEmpty())
        return QString();
    else
        return l.first();
}

QStringList QMakeFile::resolveFileName(const QString& file, const QString& base) const
{
    return resolveShellGlobbing(file, base);
}

void QMakeFile::setProject(KDevelop::IProject* project)
{
    m_project = project;
}

KDevelop::IProject* QMakeFile::project() const
{
    return m_project;
}
