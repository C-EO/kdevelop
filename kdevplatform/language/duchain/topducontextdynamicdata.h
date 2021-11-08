/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_TOPDUCONTEXTDYNAMICDATA_H
#define KDEVPLATFORM_TOPDUCONTEXTDYNAMICDATA_H

#include <QVector>
#include <QByteArray>
#include "problem.h"

class QFile;

namespace KDevelop {
class TopDUContext;
class DUContext;
class Declaration;
class IndexedString;
class IndexedDUContext;
class DUChainBaseData;

///This class contains dynamic data of a top-context, and also the repository that contains all the data within this top-context.
class TopDUContextDynamicData
{
public:
    explicit TopDUContextDynamicData(TopDUContext* topContext);
    ~TopDUContextDynamicData();

    void clear();

    /**
     * Allocates an index for the given declaration in this top-context.
     * The returned index is never zero.
     * @param temporary whether the declaration is temporary. If it is, it will be stored separately, not stored to disk,
     *                   and a duchain write-lock is not needed. Else, you need a write-lock when calling this.
     */
    uint allocateDeclarationIndex(Declaration* decl, bool temporary);

    Declaration* declarationForIndex(uint index) const;

    bool isDeclarationForIndexLoaded(uint index) const;

    void clearDeclarationIndex(Declaration* decl);

    /**
     * Allocates an index for the given context in this top-context.
     * The returned index is never zero.
     * @param temporary whether the context is temporary. If it is, it will be stored separately, not stored to disk,
     *                   and a duchain write-lock is not needed. Else, you need a write-lock when calling this.
     */
    uint allocateContextIndex(DUContext* ctx, bool temporary);

    DUContext* contextForIndex(uint index) const;

    bool isContextForIndexLoaded(uint index) const;

    void clearContextIndex(DUContext* ctx);

    /**
     * Allocates an index for the given problem in this top-context.
     * The returned index is never zero.
     */
    uint allocateProblemIndex(const ProblemPointer& problem);
    ProblemPointer problemForIndex(uint index) const;
    void clearProblems();

    ///Stores this top-context to disk
    void store();

    ///Stores all remnants of this top-context that are on disk. The top-context will be fully dynamic after this.
    void deleteOnDisk();

    ///Whether this top-context is on disk(Either has been loaded, or has been stored)
    bool isOnDisk() const;

    ///Loads the top-context from disk, or returns zero on failure. The top-context will not be registered anywhere, and will have no ParsingEnvironmentFile assigned.
    ///Also loads all imported contexts. The Declarations/Contexts will be correctly initialized, and put into the symbol tables if needed.
    static TopDUContext* load(uint topContextIndex);

    ///Loads only the url out of the data stored on disk for the top-context.
    static IndexedString loadUrl(uint topContextIndex);

    static bool fileExists(uint topContextIndex);

    ///Loads only the list of importers out of the data stored on disk for the top-context.
    static QList<IndexedDUContext> loadImporters(uint topContextIndex);

    static QList<IndexedDUContext> loadImports(uint topContextIndex);

    bool isTemporaryContextIndex(uint index) const;
    bool isTemporaryDeclarationIndex(uint index) const;

    bool m_deleting; ///Flag used during destruction

    struct ItemDataInfo
    {
        uint dataOffset; /// Offset of the data
        uint parentContext; /// Parent context of the data (0 means the global context)
    };

    struct ArrayWithPosition
    {
        QByteArray array;
        uint position;
    };

private:
    bool hasChanged() const;

    void unmap();
    //Converts away from an mmap opened file to a data array

    QString filePath() const;

    void loadData() const;

    const char* pointerInData(uint offset) const;

    ItemDataInfo writeDataInfo(const ItemDataInfo& info, const DUChainBaseData* data, uint& totalDataOffset);

    TopDUContext* m_topContext;

    template <class Item>
    struct DUChainItemStorage
    {
        explicit DUChainItemStorage(TopDUContextDynamicData* data);
        ~DUChainItemStorage();

        void clearItems();
        bool itemsHaveChanged() const;

        void storeData(uint& currentDataOffset, const QVector<ArrayWithPosition>& oldData);
        Item itemForIndex(uint index) const;

        void clearItemIndex(const Item& item, const uint index);

        uint allocateItemIndex(const Item& item, const bool temporary);

        void deleteOnDisk();
        bool isItemForIndexLoaded(uint index) const;

        void loadData(QFile* file) const;
        void writeData(QFile* file);

        //May contain zero items if they were deleted
        mutable QVector<Item> items;
        mutable QVector<ItemDataInfo> offsets;
        QVector<Item> temporaryItems;
        TopDUContextDynamicData* const data;
    };

    DUChainItemStorage<DUContext*> m_contexts;
    DUChainItemStorage<Declaration*> m_declarations;
    DUChainItemStorage<ProblemPointer> m_problems;

    //For temporary declarations that will not be stored to disk, like template instantiations

    mutable QVector<ArrayWithPosition> m_data;
    mutable QVector<ArrayWithPosition> m_topContextData;
    bool m_onDisk;
    mutable bool m_dataLoaded;

    mutable QFile* m_mappedFile;
    mutable uchar* m_mappedData;
    mutable size_t m_mappedDataSize;
    mutable bool m_itemRetrievalForbidden;
};
}

Q_DECLARE_TYPEINFO(KDevelop::TopDUContextDynamicData::ItemDataInfo, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(KDevelop::TopDUContextDynamicData::ArrayWithPosition, Q_MOVABLE_TYPE);

#endif
