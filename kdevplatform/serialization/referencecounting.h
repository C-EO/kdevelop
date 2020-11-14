/*
 * This file is part of KDevelop
 *
 * Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
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
#ifndef KDEVPLATFORM_REFERENCECOUNTING_H
#define KDEVPLATFORM_REFERENCECOUNTING_H

#include "serializationexport.h"

#include <QMap>
#include <QPair>
#include <QMutexLocker>

#include <utility>

//When this is enabled, the duchain unloading is disabled as well, and you should start
//with a cleared ~/.kdevduchain
// #define TEST_REFERENCE_COUNTING

namespace KDevelop {
///Since shouldDoDUChainReferenceCounting is called extremely often, we export some internals into the header here,
///so the reference-counting code can be inlined.

KDEVPLATFORMSERIALIZATION_EXPORT extern thread_local bool doReferenceCounting;
KDEVPLATFORMSERIALIZATION_EXPORT extern QMutex refCountingLock;
KDEVPLATFORMSERIALIZATION_EXPORT extern QMap<void*, QPair<uint, uint>>* refCountingRanges;
KDEVPLATFORMSERIALIZATION_EXPORT extern bool refCountingHasAdditionalRanges;
KDEVPLATFORMSERIALIZATION_EXPORT extern void* refCountingFirstRangeStart;
KDEVPLATFORMSERIALIZATION_EXPORT extern QPair<uint, uint> refCountingFirstRangeExtent;

KDEVPLATFORMSERIALIZATION_EXPORT void initReferenceCounting();

///@internal The spin-lock ,must already be locked
inline bool shouldDoDUChainReferenceCountingInternal(void* item)
{
    auto it = std::as_const(*refCountingRanges).upperBound(item);
    if (it != refCountingRanges->constBegin()) {
        --it;
        return reinterpret_cast<char*>(it.key()) <= reinterpret_cast<char*>(item) &&
               reinterpret_cast<char*>(item) < reinterpret_cast<char*>(it.key()) + it.value().first;
    }

    return false;
}

///This is used by indexed items to decide whether they should do reference-counting
inline bool shouldDoDUChainReferenceCounting(void* item)
{
    if (!doReferenceCounting) //Fast path, no place has been marked for reference counting, 99% of cases
        return false;

    QMutexLocker lock(&refCountingLock);

    if (refCountingFirstRangeStart &&
        (reinterpret_cast<char*>(refCountingFirstRangeStart) <= reinterpret_cast<char*>(item)) &&
        (reinterpret_cast<char*>(item) < reinterpret_cast<char*>(refCountingFirstRangeStart) + refCountingFirstRangeExtent.first))
        return true;

    if (refCountingHasAdditionalRanges)
        return shouldDoDUChainReferenceCountingInternal(item);
    else
        return false;
}

///Enable reference-counting for the given range
///You should only enable the reference-counting for the time it's really needed,
///and it always has to be enabled too when the items are deleted again, else
///it will lead to inconsistencies in the repository.
///@warning If you are not working on the duchain internal storage mechanism, you should
///not care about this stuff at all.
///@param start Position where to start the reference-counting
///@param size Size of the area in bytes
KDEVPLATFORMSERIALIZATION_EXPORT void enableDUChainReferenceCounting(void* start, unsigned int size);
///Must be called as often as enableDUChainReferenceCounting, with the same ranges
///Must never be called for the same range twice, and not for overlapping ranges
///@param start Position where the reference-counting was started
KDEVPLATFORMSERIALIZATION_EXPORT void disableDUChainReferenceCounting(void* start);

///Use this as local variable within the object that maintains the reference-count,
///and use
struct ReferenceCountManager
{
    #ifndef TEST_REFERENCE_COUNTING
    inline void increase(uint& ref, uint /*targetId*/)
    {
        ++ref;
    }
    inline void decrease(uint& ref, uint /*targetId*/)
    {
        Q_ASSERT(ref);
        --ref;
    }

    #else

    ReferenceCountManager();
    ~ReferenceCountManager();

    ReferenceCountManager(const ReferenceCountManager& rhs);
    ReferenceCountManager& operator=(const ReferenceCountManager& rhs);

    void increase(uint& ref, uint targetId);
    void decrease(uint& ref, uint targetId);

//     bool hasReferenceCount() const;

    uint m_id;
    #endif
};
}

#endif
