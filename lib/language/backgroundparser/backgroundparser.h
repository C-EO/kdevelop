/*
 * This file is part of KDevelop
 *
 * Copyright (c) 2006 Adam Treat <treat@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KDEVBACKGROUNDPARSER_H
#define KDEVBACKGROUNDPARSER_H

#include <QObject>

#include <kurl.h>
#include <QMap>
#include <QPair>
#include <QMutex>
#include <QHash>
#include <QPointer>

#include "kdevexport.h"

namespace ThreadWeaver
{
class Weaver;
class Job;
}

namespace KTextEditor
{
class Document;
}

class QTimer;
class QProgressBar;
class QMutex;
class QWaitCondition;

namespace KDevelop
{
class ILanguageSupport;
class ParseJob;
class ParserDependencyPolicy;

using namespace ThreadWeaver;

class KDEVPLATFORMLANGUAGE_EXPORT BackgroundParser : public QObject
{
    Q_OBJECT
public:
    explicit BackgroundParser(ILanguageSupport *languageSupport, QObject* parent = 0 );
    virtual ~BackgroundParser();

    /**
     * Abort or dequeue all current running jobs with the specified \a parent.
     */
    void clear(QObject* parent);

    /**
     * Queries the background parser as to whether there is currently
     * a parse job for \a document, and if so, returns it.
     *
     * This may not contain all of the parse jobs that are intended
     * unless you call in from your job's ThreadWeaver::Job::aboutToBeQueued()
     * function.
     */
    ParseJob* parseJobForDocument(const KUrl& document) const;

    /**
     * The dependency policy which applies to all jobs (it is
     * applied automatically).
     */
    ParserDependencyPolicy* dependencyPolicy() const;

public Q_SLOTS:
    void suspend();
    void resume();
    void setDelay( int msecs );
    void setThreads( int threads );

    void addDocument( const KUrl &url );
//     void addDocument( Document *document );
    void addDocumentList( const KUrl::List &urls );
    void removeDocument( const KUrl &url );
//     void removeDocument( Document *document );

    void parseDocuments();

private Q_SLOTS:
    void parseComplete( Job *job );
//     void documentChanged( KTextEditor::Document *document );

protected:
    virtual void loadSettings( bool projectIsLoaded );
    virtual void saveSettings( bool projectIsLoaded );
    virtual void initialize();
    virtual void cleanup();

private Q_SLOTS:
    void acceptAddDocument(const QUrl& url);

Q_SIGNALS:
    void requestAddDocument(const QUrl& url);

private:
    // Non-mutex guarded functions, only call with m_mutex acquired.
    void parseDocumentsInternal();
//     void cacheModelsInternal( uint modelsToCache );

    ILanguageSupport *m_languageSupport;

    QTimer *m_timer;
    int m_delay;
    int m_threads;
    uint m_modelsToCache;

    // A list of known documents, and whether they are due to be parsed or not
    QMap<KUrl, bool> m_documents;
    // A list of open documents
//     QMap<KUrl, Document*> m_openDocuments;
    // Current parse jobs
    QHash<KUrl, ParseJob*> m_parseJobs;

    QPointer<QProgressBar> m_progressBar;

    ThreadWeaver::Weaver* m_weaver;

    ParserDependencyPolicy* m_dependencyPolicy;

    QMutex* m_mutex;
    QWaitCondition* m_waitForJobCreation;
};

}
#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
