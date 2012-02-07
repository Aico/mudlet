#ifndef THIGHLIGHTER_H
#define THIGHLIGHTER_H

#include <QSyntaxHighlighter>

#include <QHash>
#include <QTextCharFormat>
#include "dlgSourceEditorArea.h"

class QTextDocument;

class THighlighter : public QSyntaxHighlighter
{
     Q_OBJECT

 public:
                               THighlighter(QTextEdit *parent = 0);
     void                      setSearchPattern( QString p );


 protected:
     void                      highlightBlock(const QString &text);

 private:
     struct HighlightingRule
     {
         QRegExp pattern;
         QTextCharFormat format;
     };
     QString                   mSearchPattern;
     QVector<HighlightingRule> highlightingRulesLua;
     QVector<HighlightingRule> highlightingRulesPython;
     QRegExp                   commentStartExpression;
     QRegExp                   commentEndExpression;
     QRegExp                   stringStart;
     QRegExp                   stringEnd;
     QTextCharFormat           keywordFormat;
     QTextCharFormat           searchFormat;
     QTextCharFormat           classFormat;
     QTextCharFormat           singleLineCommentFormat;
     QTextCharFormat           multiLineCommentFormat;
     QTextCharFormat           quotationFormat;
     QTextCharFormat           functionFormat;
     PlainTextEdit *           editor;
     //bool isString;
 };

#endif // THIGHLIGHTER_H
