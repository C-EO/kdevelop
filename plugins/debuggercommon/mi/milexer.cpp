/*
    SPDX-FileCopyrightText: 2004 Roberto Raggi <roberto@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "milexer.h"
#include "tokens.h"
#include <cctype>
#include <iostream>

using namespace KDevMI::MI;

bool MILexer::s_initialized = false;
scan_fun_ptr MILexer::s_scan_table[];


MILexer::MILexer()
{
    if (!s_initialized)
        setupScanTable();
}

MILexer::~MILexer()
{
}

void MILexer::setupScanTable()
{
    s_initialized = true;

    for (int i=0; i<128; ++i) {
        switch (i) {
        case '\n':
            s_scan_table[i] = &MILexer::scanNewline;
            break;

        case '"':
            s_scan_table[i] = &MILexer::scanStringLiteral;
            break;

        default:
            if (isspace(i))
                s_scan_table[i] = &MILexer::scanWhiteSpaces;
            else if (isalpha(i) || i == '_')
                s_scan_table[i] = &MILexer::scanIdentifier;
            else if (isdigit(i))
                s_scan_table[i] = &MILexer::scanNumberLiteral;
            else
                s_scan_table[i] = &MILexer::scanChar;
        }
    }

    s_scan_table[128] = &MILexer::scanUnicodeChar;
}

/*

    m_firstToken = m_tokens.data();
    m_currentToken = 0;

    m_firstToken = m_tokens.data();
    m_currentToken = m_firstToken;
 */

TokenStream *MILexer::tokenize(const FileSymbol *fileSymbol)
{
    m_tokensCount = 0;
    m_tokens.resize(64);

    m_contents = fileSymbol->contents;
    m_length = m_contents.length();
    m_ptr = 0;

    m_lines.resize(8);
    m_line = 0;

    m_lines[m_line++] = 0;

    m_cursor = 0;

    // tokenize
    int pos, len;

    for (;;) {
        if (m_tokensCount == (int)m_tokens.size())
            m_tokens.resize(m_tokensCount * 2);

        Token &tk = m_tokens[m_tokensCount++];
        tk.kind = nextToken(pos, len);
        tk.position = pos;
        tk.length = len;

        if (tk.kind == 0)
            break;
    }

    auto *tokenStream = new TokenStream;
    tokenStream->m_contents = m_contents;

    tokenStream->m_lines = m_lines;
    tokenStream->m_line = m_line;

    tokenStream->m_tokens = m_tokens;
    tokenStream->m_tokensCount = m_tokensCount;

    tokenStream->m_firstToken = tokenStream->m_tokens.data();
    tokenStream->m_currentToken = tokenStream->m_firstToken;

    tokenStream->m_cursor = m_cursor;

    return tokenStream;
}

int MILexer::nextToken(int &pos, int &len)
{
    while (m_ptr < m_length) {
        const int start = m_ptr;

        const char ch = m_contents[m_ptr];
        Q_ASSERT(ch >= 0);
        int kind = 0;
        (this->*s_scan_table[static_cast<uchar>(ch)])(&kind);

        switch (kind) {
            case Token_whitespaces:
            case '\n':
                break;

            default:
                pos = start;
                len = m_ptr - start;
                return kind;
        }
    }

    return 0;
}

void MILexer::scanChar(int *kind)
{
    *kind = m_contents[m_ptr++];
}

void MILexer::scanWhiteSpaces(int *kind)
{
    *kind = Token_whitespaces;

    while (m_ptr < m_length) {
        char ch = m_contents[m_ptr];
        if (!(isspace(ch) && ch != '\n'))
            break;

        ++m_ptr;
    }
}

void MILexer::scanNewline(int *kind)
{
    if (m_line == (int)m_lines.size())
        m_lines.resize(m_lines.size() * 2);

    if (m_lines.at(m_line) < m_ptr)
        m_lines[m_line++] = m_ptr;

    *kind = m_contents[m_ptr++];
}

void MILexer::scanUnicodeChar(int *kind)
{
    *kind = m_contents[m_ptr++];
}

void MILexer::scanStringLiteral(int *kind)
{
    ++m_ptr;
    while (char c = m_contents[m_ptr]) {
        switch (c) {
        case '\n':
            // ### error
            *kind = Token_string_literal;
            return;
        case '\\':
            {
                char next = m_contents.at(m_ptr+1);
                if (next == '"' || next == '\\')
                    m_ptr += 2;
                else
                    ++m_ptr;
            }
            break;
        case '"':
            ++m_ptr;
            *kind = Token_string_literal;
            return;
        default:
            ++m_ptr;
            break;
        }
    }

    // ### error
    *kind = Token_string_literal;
}

void MILexer::scanIdentifier(int *kind)
{
    while (m_ptr < m_length) {
        const char ch = m_contents[m_ptr];
        if (!(isalnum(ch) || ch == '-' || ch == '_'))
            break;

        ++m_ptr;
    }

    *kind = Token_identifier;
}

void MILexer::scanNumberLiteral(int *kind)
{
    while (m_ptr < m_length) {
        const char ch = m_contents[m_ptr];
        if (!(isalnum(ch) || ch == '.'))
            break;

        ++m_ptr;
    }

    // ### finish to implement me!!
    *kind = Token_number_literal;
}

void TokenStream::positionAt(int position, int *line, int *column) const
{
    if (!(line && column && !m_lines.isEmpty()))
        return;

    int first = 0;
    int len = m_line;

    while (len > 0) {
        const int half = len >> 1;
        const int middle = first + half;

        if (m_lines[middle] < position) {
            first = middle;
            ++first;
            len = len - half - 1;
        }
        else
            len = half;
    }

    *line = qMax(first - 1, 0);
    *column = position - m_lines.at(*line);

    Q_ASSERT( *column >= 0 );
}

QByteArray TokenStream::tokenText(int index) const
{
    Token *t = index < 0 ? m_currentToken : m_firstToken + index;
    const char* data = m_contents.constData();
    return QByteArray(data + t->position, t->length);
}

