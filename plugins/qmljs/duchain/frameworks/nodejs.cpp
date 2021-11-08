/*
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "nodejs.h"
#include "../helper.h"
#include "../parsesession.h"

#include <language/duchain/duchain.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/declaration.h>
#include <language/duchain/types/structuretype.h>
#include <language/duchain/types/integraltype.h>
#include <util/path.h>

#include <QFile>
#include <QStandardPaths>

using namespace KDevelop;

namespace QmlJS {

NodeJS::NodeJS()
{
}

NodeJS& NodeJS::instance()
{
    static NodeJS* i = nullptr;

    if (!i) {
        i = new NodeJS();
    }

    return *i;
}

void NodeJS::initialize(DeclarationBuilder* builder)
{
    QMutexLocker lock(&m_mutex);

    // Create "module", a structure that may contain "exports" if the module
    // refers to module.exports
    createObject(QStringLiteral("module"), 1, builder);

    // Create "exports", that can also contain the exported symbols of the module
    createObject(QStringLiteral("exports"), 2, builder);
}

void NodeJS::createObject(const QString& name, int index, DeclarationBuilder* builder)
{
    Identifier identifier(name);

    StructureType::Ptr type(new StructureType);
    auto* decl = builder->openDeclaration<Declaration>(identifier, RangeInRevision());

    type->setDeclaration(decl);
    decl->setAlwaysForceDirect(true);
    decl->setKind(Declaration::Type);   // Not exactly what the user would expect, but this ensures that QmlJS::getInternalContext does not recurse infinitely
    decl->setInternalContext(builder->openContext(
        (QmlJS::AST::Node*)nullptr + index,                // Index is used to disambiguate the contexts. "node" is never dereferenced and is only stored in a hash table
        RangeInRevision(),
        DUContext::Class,
        QualifiedIdentifier(identifier)
    ));

    builder->closeContext();
    builder->openType(type);
    builder->closeAndAssignType();
}

DeclarationPointer NodeJS::moduleExports(const QString& moduleName, const IndexedString& url)
{
    QString urlStr = url.str();
    QString fileName = moduleFileName(moduleName, urlStr);
    DeclarationPointer exports;

    if (fileName.isEmpty() || urlStr.contains(QLatin1String("__builtin_ecmascript.js")) || urlStr == fileName) {
        return exports;
    }

    ReferencedTopDUContext topContext = ParseSession::contextOfFile(fileName, url, 0);
    DUChainReadLocker lock;

    if (topContext) {
        static const QualifiedIdentifier idModule(QStringLiteral("module"));
        static const QualifiedIdentifier idExports(QStringLiteral("exports"));

        // Try "module.exports". If this declaration exists, it contains the
        // module's exports
        exports = getDeclaration(idModule, topContext.data());

        if (exports && exports->internalContext()) {
            exports = getDeclaration(idExports, exports->internalContext(), false);
        }

        // Try "exports", that always exist, has a structure type, and contains
        // the exported symbols
        if (!exports) {
            exports = getDeclaration(idExports, topContext.data());
        }
    }

    return exports;
}

DeclarationPointer NodeJS::moduleMember(const QString& moduleName,
                                        const QString& memberName,
                                        const IndexedString& url)
{
    DeclarationPointer module = moduleExports(moduleName, url);
    DeclarationPointer member;

    if (module) {
        member = QmlJS::getDeclaration(
            QualifiedIdentifier(memberName),
            QmlJS::getInternalContext(module),
            false
        );
    }

    return member;
}

Path::List NodeJS::moduleDirectories(const QString& url)
{
    Path::List paths;

    // QML/JS ships several modules that exist only in binary form in Node.js
    const QStringList& dirs = QStandardPaths::locateAll(
        QStandardPaths::GenericDataLocation,
        QStringLiteral("kdevqmljssupport/nodejsmodules"),
        QStandardPaths::LocateDirectory
    );

    paths.reserve(dirs.size());
    for (auto& dir : dirs) {
        paths.append(Path(dir));
    }

    // url/../node_modules, then url/../../node_modules, etc
    Path path(url);
    path.addPath(QStringLiteral(".."));

    const int maxPathSize = path.isLocalFile() ? 1 : 2;
    while (path.segments().size() > maxPathSize) {
        paths.append(path.cd(QStringLiteral("node_modules")));
        path.addPath(QStringLiteral(".."));
    }

    return paths;
}

QString NodeJS::moduleFileName(const QString& moduleName, const QString& url)
{
    QMutexLocker lock(&m_mutex);
    auto pair = qMakePair(moduleName, url);

    const auto cachedModuleFileNameIt = m_cachedModuleFileNames.constFind(pair);
    if (cachedModuleFileNameIt != m_cachedModuleFileNames.constEnd()) {
        return *cachedModuleFileNameIt;
    }

    QString& fileName = m_cachedModuleFileNames[pair];

    // Absolute and relative URLs
    if (moduleName.startsWith(QLatin1Char('/')) || moduleName.startsWith(QLatin1Char('.'))) {
        // NOTE: This is not portable to Windows, but the Node.js documentation
        // only talks about module names that start with /, ./ and ../ .
        fileName = fileOrDirectoryPath(Path(url).cd(QStringLiteral("..")).cd(moduleName).toLocalFile());
        return fileName;
    }

    // Try all the paths that might contain modules
    const auto& paths = moduleDirectories(url);
    for (auto& path : paths) {
        fileName = fileOrDirectoryPath(path.cd(moduleName).toLocalFile());

        if (!fileName.isEmpty()) {
            break;
        }
    }

    return fileName;
}

QString NodeJS::fileOrDirectoryPath(const QString& baseName)
{
    if (QFile::exists(baseName)) {
        return baseName;
    } else if (QFile::exists(baseName + QLatin1String(".js"))) {
        return baseName + QLatin1String(".js");
    } else if (QFile::exists(baseName + QLatin1String("/index.js"))) {
        // TODO: package.json files currently not supported
        return baseName + QLatin1String("/index.js");
    }

    return QString();
}

}
