/***************************************************************************
 *   Copyright (C) 2008 by Heiko Koehn   *
 *   KoehnHeiko@googlemail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

//#include <Qsci/qsciscintilla.h>
//#include <Qsci/qscilexerlua.h>

#include <QWidget>
#include <iostream>

#include "dlgSourceEditorArea.h"
#include "THighlighter.h"

#define LUA     0
#define PYTHON  1

dlgSourceEditorArea::dlgSourceEditorArea(QWidget * pF) : QWidget(pF)
{
    // init generated dialog
    setupUi(this);
    editor = new PlainTextEdit(this);
    editor->setObjectName(QString::fromUtf8("editor"));
    verticalLayout->addWidget(editor);
    highlighter = new THighlighter((QTextEdit*)editor);
    editor->setTabStopWidth(25);

}

PlainTextEdit::PlainTextEdit(QWidget* parent) 
: QPlainTextEdit(parent) 
,mScriptLanguage(LUA)
{}

void PlainTextEdit::keyPressEvent(QKeyEvent * event)
{
    if (mScriptLanguage == LUA)
    {
        QPlainTextEdit::keyPressEvent(event);
    }
    else if (event->key() == Qt::Key_Tab)
    {
        int nl = textCursor().selectedText().count(QString::fromUtf8("\u2029"));
        if (nl>0)
        {
            QString sText = textCursor().selectedText();
            textCursor().removeSelectedText();
            if (event->modifiers() == Qt::ControlModifier) {
                if (sText.startsWith(QString("    "))) {
                    sText.remove(0,4);
                }
                sText.replace(QString::fromUtf8("\u2029")+QString("    "),QString::fromUtf8("\u2029"));
            }
            else
            {
                sText.insert(0,QString("    "));
                sText.replace(QString::fromUtf8("\u2029"),QString::fromUtf8("\u2029")+QString("    "));
            }
            textCursor().insertText(sText);
        }
        else
        {
            textCursor().insertText("    ");
        }
    }
    else if (event->key() == Qt::Key_Return)
    {
        QString line = textCursor().block().text();
        int count = 0;
        QString::iterator i;
        for (i = line.begin();i != line.end(); i++)
        {
            if (*i != ' ') break;
            count++;
        }
        QString replace = "\n";
        for (int j=0; j<count; ++j)
        {
            replace += " ";
        }
        textCursor().insertText(replace);
    }
        
    else
    {
        QPlainTextEdit::keyPressEvent(event);
    }
}

void PlainTextEdit::setScriptLanguage(int lang)
{
    mScriptLanguage = lang;
}
