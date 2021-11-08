/*
    SPDX-FileCopyrightText: 2007-2009 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Lior Mualem <lior.m.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_CLASSMODELNODE_H
#define KDEVPLATFORM_CLASSMODELNODE_H

#include "classmodel.h"

#include "../duchain/identifier.h"
#include "../duchain/duchainpointer.h"
#include "classmodelnodescontroller.h"

#include <QIcon>

class NodesModelInterface;

namespace KDevelop {
class ClassDeclaration;
class ClassFunctionDeclaration;
class ClassMemberDeclaration;
class Declaration;
}

namespace ClassModelNodes {
/// Base node class - provides basic functionality.
class Node
{
public:
    Node(const QString& a_displayName, NodesModelInterface* a_model);
    virtual ~Node();

public: // Operations
    /// Clear all the children from the node.
    void clear();

    /// Called by the model to collapse the node and remove sub-items if needed.
    virtual void collapse() {};

    /// Called by the model to expand the node and populate it with sub-nodes if needed.
    virtual void expand() {};

    /// Append a new child node to the list.
    void addNode(Node* a_child);

    /// Remove child node from the list and delete it.
    void removeNode(Node* a_child);

    /// Remove this node and delete it.
    void removeSelf() { m_parentNode->removeNode(this); }

    /// Called once the node has been populated to sort the entire tree / branch.
    void recursiveSort();

public: // Info retrieval
    /// Return the parent associated with this node.
    Node* parent() const { return m_parentNode; }

    /// Get my index in the parent node
    int row();

    /// Return the display name for the node.
    QString displayName() const { return m_displayName; }

    /// Returns a list of child nodes
    const QList<Node*>& children() const { return m_children; }

    /// Return an icon representation for the node.
    /// @note It calls the internal getIcon and caches the result.
    QIcon cachedIcon();

public: // overridables
    /// Return a score when sorting the nodes.
    virtual int score() const = 0;

    /// Return true if the node contains sub-nodes.
    virtual bool hasChildren() const { return !m_children.empty(); }

    /// We use this string when sorting items.
    virtual QString sortableString() const { return m_displayName; }

protected:
    /// fill a_resultIcon with a display icon for the node.
    /// @param a_resultIcon returned icon.
    /// @return true if result was returned.
    virtual bool getIcon(QIcon& a_resultIcon) = 0;

private:
    Node* m_parentNode;

    /// Called once the node has been populated to sort the entire tree / branch.
    void recursiveSortInternal();

protected:
    using NodesList = QList<Node*>;
    NodesList m_children;
    QString m_displayName;
    QIcon m_cachedIcon;
    NodesModelInterface* m_model;
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/// Base class for nodes that generate and populate their child nodes dynamically
class DynamicNode
    : public Node
{
public:
    DynamicNode(const QString& a_displayName, NodesModelInterface* a_model);

    /// Return true if the node was populated already.
    bool isPopulated() const { return m_populated; }

    /// Populate the node and mark the flag - called from expand or can be used internally.
    void performPopulateNode(bool a_forceRepopulate = false);

public: // Node overrides.
    void collapse() override;
    void expand() override;
    bool hasChildren() const override;

protected: // overridables
    /// Called by the framework when the node is about to be expanded
    /// it should be populated with sub-nodes if applicable.
    virtual void populateNode() {}

    /// Called after the nodes have been removed.
    /// It's for derived classes to clean cached data.
    virtual void nodeCleared() {}

private:
    bool m_populated;

    /// Clear all the child nodes and mark flag.
    void performNodeCleanup();
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/// Base class for nodes associated with a @ref KDevelop::QualifiedIdentifier
class IdentifierNode
    : public DynamicNode
{
public:
    IdentifierNode(KDevelop::Declaration* a_decl, NodesModelInterface* a_model,
                   const QString& a_displayName = QString());

public:
    /// Returns the qualified identifier for this node by going through the tree
    const KDevelop::IndexedQualifiedIdentifier& identifier() const { return m_identifier; }

public: // Node overrides
    bool getIcon(QIcon& a_resultIcon) override;

public: // Overridables
    /// Return the associated declaration
    /// @note DU CHAIN MUST BE LOCKED FOR READ
    virtual KDevelop::Declaration* declaration();

private:
    KDevelop::IndexedQualifiedIdentifier m_identifier;
    KDevelop::IndexedDeclaration m_indexedDeclaration;
    KDevelop::DeclarationPointer m_cachedDeclaration;
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/// A node that represents an enum value.
class EnumNode
    : public IdentifierNode
{
public:
    EnumNode(KDevelop::Declaration* a_decl, NodesModelInterface* a_model);

public: // Node overrides
    int score() const override { return 102; }
    bool getIcon(QIcon& a_resultIcon) override;
    void populateNode() override;
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/// Provides display for a single class.
class ClassNode
    : public IdentifierNode
    , public ClassModelNodeDocumentChangedInterface
{
public:
    ClassNode(KDevelop::Declaration* a_decl, NodesModelInterface* a_model);
    ~ClassNode() override;

    /// Lookup a contained class and return the related node.
    /// @return the node pointer or 0 if non was found.
    ClassNode* findSubClass(const KDevelop::IndexedQualifiedIdentifier& a_id);

public: // Node overrides
    int score() const override { return 300; }
    void populateNode() override;
    void nodeCleared() override;
    bool hasChildren() const override { return true; }

protected: // ClassModelNodeDocumentChangedInterface overrides
    void documentChanged(const KDevelop::IndexedString& a_file) override;

private:
    using SubIdentifiersMap = QMap<uint, Node*>;
    /// Set of known sub-identifiers. It's used for updates check.
    SubIdentifiersMap m_subIdentifiers;

    /// We use this variable to know if we've registered for change notification or not.
    KDevelop::IndexedString m_cachedUrl;

    /// Updates the node to reflect changes in the declaration.
    /// @note DU CHAIN MUST BE LOCKED FOR READ
    /// @return true if something was updated.
    bool updateClassDeclarations();

    /// Add "Base classes" and "Derived classes" folders, if needed
    /// @return true if one of the folders was added.
    bool addBaseAndDerived();
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/// Provides a display for a single class function.
class FunctionNode
    : public IdentifierNode
{
public:
    FunctionNode(KDevelop::Declaration* a_decl, NodesModelInterface* a_model);

public: // Node overrides
    int score() const override { return 400; }
    QString sortableString() const override { return m_sortableString; }

private:
    QString m_sortableString;
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/// Provides display for a single class variable.
class ClassMemberNode
    : public IdentifierNode
{
public:
    ClassMemberNode(KDevelop::ClassMemberDeclaration* a_decl, NodesModelInterface* a_model);

public: // Node overrides
    int score() const override { return 500; }
    bool getIcon(QIcon& a_resultIcon) override;
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/// Provides a folder node with a static list of nodes.
class FolderNode
    : public Node
{
public:
    FolderNode(const QString& a_displayName, NodesModelInterface* a_model);

public: // Node overrides
    bool getIcon(QIcon& a_resultIcon) override;
    int score() const override { return 100; }
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/// Provides a folder node with a dynamic list of nodes.
class DynamicFolderNode
    : public DynamicNode
{
public:
    DynamicFolderNode(const QString& a_displayName, NodesModelInterface* a_model);

public: // Node overrides
    bool getIcon(QIcon& a_resultIcon) override;
    int score() const override { return 100; }
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/// Special folder - the parent is assumed to be a ClassNode.
/// It then displays the base classes for the class it sits in.
class BaseClassesFolderNode
    : public DynamicFolderNode
{
public:
    explicit BaseClassesFolderNode(NodesModelInterface* a_model);

public: // Node overrides
    void populateNode() override;
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

/// Special folder - the parent is assumed to be a ClassNode.
/// It then displays list of derived classes from the parent class.
class DerivedClassesFolderNode
    : public DynamicFolderNode
{
public:
    explicit DerivedClassesFolderNode(NodesModelInterface* a_model);

public: // Node overrides
    void populateNode() override;
};
} // namespace classModelNodes

#endif
