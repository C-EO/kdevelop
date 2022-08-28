/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "projectfilequickopen.h"

#include <QIcon>
#include <QTextBrowser>

#include <KLocalizedString>

#include <interfaces/iprojectcontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/icore.h>

#include <language/duchain/topducontext.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <serialization/indexedstring.h>
#include <language/duchain/parsingenvironment.h>
#include <util/algorithm.h>
#include <util/texteditorhelpers.h>

#include <project/projectmodel.h>
#include <project/projectutils.h>

#include "../openwith/iopenwith.h"

#include <timsort/timsort.hpp>

#include <algorithm>
#include <iterator>
#include <utility>
#include <vector>

using namespace KDevelop;

namespace {
QSet<IndexedString> openFiles()
{
    QSet<IndexedString> openFiles;
    const QList<IDocument*>& docs = ICore::self()->documentController()->openDocuments();
    openFiles.reserve(docs.size());
    for (IDocument* doc : docs) {
        openFiles << IndexedString(doc->url());
    }

    return openFiles;
}

QString iconNameForUrl(const IndexedString& url)
{
    if (url.isEmpty()) {
        return QStringLiteral("tab-duplicate");
    }
    ProjectBaseItem* item = ICore::self()->projectController()->projectModel()->itemForPath(url);
    if (item) {
        return item->iconName();
    }
    return QStringLiteral("unknown");
}
}

ProjectFile::ProjectFile(const ProjectFileItem* fileItem)
    : path{fileItem->path()}
    , projectPath{fileItem->project()->path()}
    , indexedPath{fileItem->indexedPath()}
    , outsideOfProject{!projectPath.isParentOf(path)}
{
}

ProjectFileData::ProjectFileData(const ProjectFile& file)
    : m_file(file)
{
}

QString ProjectFileData::text() const
{
    return m_file.projectPath.relativePath(m_file.path);
}

QString ProjectFileData::htmlDescription() const
{
    return
        QLatin1String("<small><small>") +
        i18nc("%1: project name", "Project %1", project()) +
        QLatin1String("</small></small>");
}

bool ProjectFileData::execute(QString& filterText)
{
    const QUrl url = m_file.path.toUrl();
    IOpenWith::openFiles(QList<QUrl>() << url);

    auto cursor = KTextEditorHelpers::extractCursor(filterText);
    if (cursor.isValid()) {
        IDocument* doc = ICore::self()->documentController()->documentForUrl(url);
        if (doc) {
            doc->setCursorPosition(cursor);
        }
    }
    return true;
}

bool ProjectFileData::isExpandable() const
{
    return true;
}

QList<QVariant> ProjectFileData::highlighting() const
{
    QTextCharFormat boldFormat;
    boldFormat.setFontWeight(QFont::Bold);
    QTextCharFormat normalFormat;

    QString txt = text();

    int fileNameLength = m_file.path.lastPathSegment().length();

    const QList<QVariant> ret{
        0,
        txt.length() - fileNameLength,
        QVariant(normalFormat),
        txt.length() - fileNameLength,
        fileNameLength,
        QVariant(boldFormat),
    };
    return ret;
}

QWidget* ProjectFileData::expandingWidget() const
{
    const QUrl url = m_file.path.toUrl();
    DUChainReadLocker lock;

    ///Find a du-chain for the document
    const QList<TopDUContext*> contexts = DUChain::self()->chainsForDocument(url);

    ///Pick a non-proxy context
    TopDUContext* chosen = nullptr;
    for (TopDUContext* ctx : contexts) {
        if (!(ctx->parsingEnvironmentFile() && ctx->parsingEnvironmentFile()->isProxyContext())) {
            chosen = ctx;
        }
    }

    if (chosen) {
        // TODO: show project name, by introducing a generic wrapper widget that supports QuickOpenEmbeddedWidgetInterface
        return chosen->createNavigationWidget();
    } else {
        auto* ret = new QTextBrowser();
        ret->resize(400, 100);
        ret->setText(
            QLatin1String("<small><small>")
            + i18nc("%1: project name", "Project %1", project())
            + QLatin1String("<br>") + i18n("Not parsed yet")
            + QLatin1String("</small></small>"));
        return ret;
    }

    return nullptr;
}

QIcon ProjectFileData::icon() const
{
    return QIcon::fromTheme(iconNameForUrl(m_file.indexedPath));
}

QString ProjectFileData::project() const
{
    const IProject* project = ICore::self()->projectController()->findProjectForUrl(m_file.path.toUrl());
    if (project) {
        return project->name();
    } else {
        return i18nc("@item no project", "none");
    }
}

Path ProjectFileData::projectPath() const
{
    return m_file.projectPath;
}

BaseFileDataProvider::BaseFileDataProvider()
{
}

void BaseFileDataProvider::setFilterText(const QString& text)
{
    int pathLength;
    KTextEditorHelpers::extractCursor(text, &pathLength);
    QString path(text.mid(0, pathLength));
    if (path.startsWith(QLatin1String("./")) || path.startsWith(QLatin1String("../"))) {
        // assume we want to filter relative to active document's url
        IDocument* doc = ICore::self()->documentController()->activeDocument();
        if (doc) {
            path = Path(Path(doc->url()).parent(), path).pathOrUrl();
        }
    }
    setFilter(path.split(QLatin1Char('/'), Qt::SkipEmptyParts));
}

uint BaseFileDataProvider::itemCount() const
{
    return filteredItems().count();
}

uint BaseFileDataProvider::unfilteredItemCount() const
{
    return items().count();
}

QuickOpenDataPointer BaseFileDataProvider::data(uint row) const
{
    return QuickOpenDataPointer(new ProjectFileData(filteredItems().at(row)));
}

ProjectFileDataProvider::ProjectFileDataProvider()
{
    auto projectController = ICore::self()->projectController();
    connect(projectController, &IProjectController::projectClosing,
            this, &ProjectFileDataProvider::projectClosing);
    connect(projectController, &IProjectController::projectOpened,
            this, &ProjectFileDataProvider::projectOpened);
    const auto projects = projectController->projects();
    for (auto* project : projects) {
        projectOpened(project);
    }
}

void ProjectFileDataProvider::projectClosing(IProject* project)
{
    // Once we remove all project's files from set, there is no need to listen
    // to &IProject::fileRemovedFromSet signal from this project and waste time
    // searching in m_projectFiles. No need to listen to &IProject::fileAddedToSet
    // signal from this project either - we are not interested in hypothetical
    // file additions to the project that is about to be closed and destroyed.
    disconnect(project, nullptr, this, nullptr);

    if (ICore::self()->projectController()->projectCount() == 0) {
        // No open projects left => just remove all files. This is a little faster
        // than the algorithm below. Releasing the memory here would slow down the
        // next call to projectOpened() => keep the capacity of m_projectFiles.
        m_projectFiles.clear();
        return;
    }

    const Path projectPath = project->path();
    const auto logicalEnd = std::remove_if(m_projectFiles.begin(), m_projectFiles.end(),
                                           [&projectPath](const ProjectFile& f) {
                                               return f.projectPath == projectPath;
                                           });
    m_projectFiles.erase(logicalEnd, m_projectFiles.end());
}

void ProjectFileDataProvider::projectOpened(IProject* project)
{
    connect(project, &IProject::fileAddedToSet,
            this, &ProjectFileDataProvider::fileAddedToSet);
    connect(project, &IProject::fileRemovedFromSet,
            this, &ProjectFileDataProvider::fileRemovedFromSet);

    // Collect the opened project's files.
    const auto oldSize = m_projectFiles.size();
    KDevelop::forEachFile(project->projectItem(), [this](ProjectFileItem* fileItem) {
        m_projectFiles.emplace_back(fileItem);
    });
    const auto justAddedBegin = m_projectFiles.begin() + oldSize;

    // Sort the opened project's files.
    // Sorting stability is not useful here, but timsort vastly outperforms all
    // std and boost sorting algorithms (boost::sort::flat_stable_sort is the
    // second best) on the files of large real-life projects, because
    // KDevelop::forEachFile() collects files in an almost sorted order.
    gfx::timsort(justAddedBegin, m_projectFiles.end());

    // Merge the sorted ranges of files belonging to previously opened projects
    // and to the just opened project.
    // Since the file sets from different projects usually don't overlap or overlap
    // very little, timmerge is the perfect merge algorithm. Furthermore, the
    // comparison of ProjectFile objects is expensive and cache-unfriendly. This
    // aspect lets timmerge outperform std::inplace_merge even more here. This same
    // aspect also helps timsort outperform other sorting algorithms.
    gfx::timmerge(m_projectFiles.begin(), justAddedBegin, m_projectFiles.end());

    // Remove duplicates across all open projects. Usually different projects have no
    // common files. But since a file can belong to multiple targets within one project,
    // a single call to KDevelop::forEachFile() often produces many duplicates.
    const auto equalFiles = [](const ProjectFile& a, const ProjectFile& b) {
        return a.indexedPath == b.indexedPath;
    };
    m_projectFiles.erase(std::unique(m_projectFiles.begin(), m_projectFiles.end(), equalFiles),
                         m_projectFiles.end());
}

void ProjectFileDataProvider::fileAddedToSet(ProjectFileItem* fileItem)
{
    ProjectFile f(fileItem);
    auto it = std::lower_bound(m_projectFiles.begin(), m_projectFiles.end(), f);
    if (it == m_projectFiles.end() || it->indexedPath != f.indexedPath) {
        m_projectFiles.insert(it, std::move(f));
    }
}

void ProjectFileDataProvider::fileRemovedFromSet(ProjectFileItem* file)
{
    ProjectFile item;
    item.path = file->path();
    item.indexedPath = file->indexedPath();

    // fast-path for non-generated files
    // NOTE: figuring out whether something is generated is expensive... and since
    // generated files are rare we apply this two-step algorithm here
    auto it = std::lower_bound(m_projectFiles.begin(), m_projectFiles.end(), item);
    if (it != m_projectFiles.end() && it->indexedPath == item.indexedPath) {
        m_projectFiles.erase(it);
        return;
    }

    // last try: maybe it was generated
    item.outsideOfProject = true;
    it = std::lower_bound(m_projectFiles.begin(), m_projectFiles.end(), item);
    if (it != m_projectFiles.end() && it->indexedPath == item.indexedPath) {
        m_projectFiles.erase(it);
        return;
    }
}

void ProjectFileDataProvider::reset()
{
    updateItems([this](QVector<ProjectFile>& closedFiles) {
        const auto open = openFiles();
        // Don't "optimize" by assigning m_projectFiles to closedFiles and
        // returning early if there are no open files. Such an optimization may
        // speed up this call to reset() sometimes - if the destruction of the
        // previous data of closedFiles doesn't take long for some reason. But
        // this "optimization" will discard the current elements and capacity of
        // closedFiles, eventually will trigger an extra allocation and
        // construction of many ProjectFile objects: when a project is opened or
        // closed, when a file is added to/removed from a project, or when a
        // file is opened and reset() is called again. See also the
        // documentation of PathFilter::updateItems().

        closedFiles.resize(m_projectFiles.size());
        const auto logicalEnd = std::remove_copy_if(
                m_projectFiles.cbegin(), m_projectFiles.cend(),
                closedFiles.begin(), [&open](const ProjectFile& f) {
                                         return open.contains(f.indexedPath);
                                     });
        closedFiles.erase(logicalEnd, closedFiles.end());
    });
}

QSet<IndexedString> ProjectFileDataProvider::files() const
{
    const auto projects = ICore::self()->projectController()->projects();
    if (projects.empty()) {
        return {}; // don't call openFiles() needlessly
    }

    std::vector<QSet<IndexedString>> sets;
    sets.reserve(projects.size());
    std::transform(projects.cbegin(), projects.cend(), std::back_inserter(sets),
                   [](const IProject* project) {
                       return project->fileSet();
                   });

    auto result = Algorithm::unite(std::move(sets));
    result.subtract(openFiles());
    return result;
}

void OpenFilesDataProvider::reset()
{
    updateItems([](QVector<ProjectFile>& currentFiles) {
        const auto* const projCtrl = ICore::self()->projectController();
        const auto docs = ICore::self()->documentController()->openDocuments();

        currentFiles.resize(docs.size());
        std::transform(docs.cbegin(), docs.cend(), currentFiles.begin(),
                       [projCtrl](const IDocument* doc) {
                           ProjectFile f;
                           const QUrl docUrl = doc->url();
                           f.path = Path(docUrl);
                           if (const IProject* project = projCtrl->findProjectForUrl(docUrl)) {
                               f.projectPath = project->path();
                           }
                           return f;
                       });
        std::sort(currentFiles.begin(), currentFiles.end());
    });
}

QSet<IndexedString> OpenFilesDataProvider::files() const
{
    return openFiles();
}

