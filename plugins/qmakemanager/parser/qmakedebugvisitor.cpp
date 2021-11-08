/*
    SPDX-FileCopyrightText: 2006 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qmakedebugvisitor.h"

#include "qmakeparser.h"
#include "qmakeast.h"
#include "kdev-pg-token-stream.h"
#include <debug.h>

namespace QMake {

DebugVisitor::DebugVisitor(QMake::Parser* parser)
    : m_parser(parser)
    , indent(0)
{
}

QString DebugVisitor::getTokenInfo(qint64 idx)
{
    qint64 line, col;
    QMake::Parser::Token token = m_parser->tokenStream->at(idx);
    m_parser->tokenStream->startPosition(idx, &line, &col);
    return QStringLiteral("%1,%2,%3").arg(line).arg(col).arg(m_parser->tokenText(token.begin, token.end).replace(QLatin1Char('\n'), QLatin1String("\\n")));
}

QString DebugVisitor::getIndent()
{
    return QString().fill(QLatin1Char(' '), indent * 4);
}

void DebugVisitor::visitArgumentList(ArgumentListAst* node)
{
    qCDebug(KDEV_QMAKE) << getIndent() << "BEGIN(arg_list)(" << getTokenInfo(node->startToken) << ")";
    indent++;
    DefaultVisitor::visitArgumentList(node);
    indent--;
    qCDebug(KDEV_QMAKE) << getIndent() << "END(arg_list)(" << getTokenInfo(node->startToken) << ")";
}

void DebugVisitor::visitOrOperator(OrOperatorAst* node)
{
    qCDebug(KDEV_QMAKE) << getIndent() << "BEGIN(or_op)(" << getTokenInfo(node->startToken) << ")";
    indent++;
    DefaultVisitor::visitOrOperator(node);
    indent--;
    qCDebug(KDEV_QMAKE) << getIndent() << "END(or_op)(" << getTokenInfo(node->endToken) << ")";
}

void DebugVisitor::visitScope(ScopeAst* node)
{
    qCDebug(KDEV_QMAKE) << getIndent() << "BEGIN(scope)(" << getTokenInfo(node->startToken) << ")";
    indent++;
    visitNode(node->functionArguments);
    visitNode(node->orOperator);
    visitNode(node->scopeBody);
    indent--;
    qCDebug(KDEV_QMAKE) << getIndent() << "END(scope)(" << getTokenInfo(node->endToken) << ")";
}

void DebugVisitor::visitFunctionArguments(FunctionArgumentsAst* node)
{
    qCDebug(KDEV_QMAKE) << getIndent() << "BEGIN(function_args)(" << getTokenInfo(node->startToken) << ")";
    indent++;
    DefaultVisitor::visitFunctionArguments(node);
    indent--;
    qCDebug(KDEV_QMAKE) << getIndent() << "END(function_args)(" << getTokenInfo(node->endToken) << ")";
}

void DebugVisitor::visitOp(OpAst* node)
{
    qCDebug(KDEV_QMAKE) << getIndent() << "BEGIN(op)(" << getTokenInfo(node->startToken) << ")";
    indent++;
    qCDebug(KDEV_QMAKE) << getIndent() << "optoken =" << getTokenInfo(node->optoken);
    DefaultVisitor::visitOp(node);
    indent--;
    qCDebug(KDEV_QMAKE) << getIndent() << "END(op)(" << getTokenInfo(node->endToken) << "))";
}

void DebugVisitor::visitProject(ProjectAst* node)
{
    qCDebug(KDEV_QMAKE) << getIndent() << "BEGIN(project)(" << getTokenInfo(node->startToken) << ")";
    indent++;
    DefaultVisitor::visitProject(node);
    indent--;
    qCDebug(KDEV_QMAKE) << getIndent() << "END(project)(" << getTokenInfo(node->endToken) << ")";
}

void DebugVisitor::visitScopeBody(ScopeBodyAst* node)
{
    qCDebug(KDEV_QMAKE) << getIndent() << "BEGIN(scope_body)(" << getTokenInfo(node->startToken) << ")";
    indent++;
    DefaultVisitor::visitScopeBody(node);
    indent--;
    qCDebug(KDEV_QMAKE) << getIndent() << "END(scope_body)(" << getTokenInfo(node->endToken) << ")";
}

void DebugVisitor::visitStatement(StatementAst* node)
{
    qCDebug(KDEV_QMAKE) << getIndent() << "BEGIN(stmt)(" << getTokenInfo(node->startToken) << ")";
    indent++;
    qCDebug(KDEV_QMAKE) << getIndent() << "isExclam=" << node->isExclam;
    if (!node->isNewline) {
        qCDebug(KDEV_QMAKE) << getIndent() << "id=" << getTokenInfo(node->id);
    }
    DefaultVisitor::visitStatement(node);
    indent--;
    qCDebug(KDEV_QMAKE) << getIndent() << "END(stmt)(" << getTokenInfo(node->endToken) << ")";
}

void DebugVisitor::visitValue(ValueAst* node)
{
    qCDebug(KDEV_QMAKE) << getIndent() << "BEGIN(value)(" << getTokenInfo(node->startToken) << ")";
    indent++;
    qCDebug(KDEV_QMAKE) << getIndent() << "value=" << getTokenInfo(node->value);
    DefaultVisitor::visitValue(node);
    indent--;
    qCDebug(KDEV_QMAKE) << getIndent() << "END(value)(" << getTokenInfo(node->endToken) << ")";
}

void DebugVisitor::visitValueList(ValueListAst* node)
{
    qCDebug(KDEV_QMAKE) << getIndent() << "BEGIN(value_list)(" << getTokenInfo(node->startToken) << ")";
    indent++;
    DefaultVisitor::visitValueList(node);
    indent--;
    qCDebug(KDEV_QMAKE) << getIndent() << "END(value_list)(" << getTokenInfo(node->endToken) << ")";
}

void DebugVisitor::visitVariableAssignment(VariableAssignmentAst* node)
{
    qCDebug(KDEV_QMAKE) << getIndent() << "BEGIN(variable_assignment)(" << getTokenInfo(node->startToken) << ")";
    indent++;
    DefaultVisitor::visitVariableAssignment(node);
    indent--;
    qCDebug(KDEV_QMAKE) << getIndent() << "END(variable_assignment)(" << getTokenInfo(node->endToken) << ")";
}

void DebugVisitor::visitItem(ItemAst* node)
{

    qCDebug(KDEV_QMAKE) << getIndent() << "BEGIN(item)(" << getTokenInfo(node->startToken) << ")";
    indent++;
    qCDebug(KDEV_QMAKE) << getIndent() << "id=" << getTokenInfo(node->id);
    DefaultVisitor::visitItem(node);
    indent--;
    qCDebug(KDEV_QMAKE) << getIndent() << "END(item)(" << getTokenInfo(node->endToken) << ")";
}
}
