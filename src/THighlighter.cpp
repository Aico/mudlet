#include "THighlighter.h"

#include <QtGui>

#define LUA     0
#define PYTHON  1

THighlighter::THighlighter( QTextEdit *parent )
: QSyntaxHighlighter( parent )
, editor((PlainTextEdit*)parent)
{
    //isString = false;
    HighlightingRule rule;

    keywordFormat.setForeground( Qt::darkBlue );
    keywordFormat.setFontWeight( QFont::Bold );
    QStringList keywordPatternsLua, keywordPatternsPython, valuePatternsPython, modulePatternsPython
                , builtinFunctionsPatternsPython, builtinObjectsPatternsPython;
    keywordPatternsLua << "\\btrue\\b" << "\\bfalse\\b" << "\\bnil\\b"
                    << "\\bnot\\b" << "\\band\\b" << "\\bor\\b"
                    << "\\bfunction\\b" << "\\blocal\\b" << "\\end\\b"
                    << "\\bwhile\\b" << "\\bdo\\b" << "\\bif\\b"
                    << "\\bthen\\b" << "\\belse\\b" << "\\bwhile\\b"
                    << "\\brepeat\\b" << "\\bfor\\b" << "\\bpairs\\b"
                    << "\\bipairs\\b" << "\\bin\\b" << "\\buntil\\b"
                    << "\\bbreak\\b" << "\\breturn\\b" << "\\belseif\\b";
                    
    keywordPatternsPython   << "\\band\\b" << "\\bassert\\b" << "\\bbreak\\b" << "\\bclass\\b" << "\\bcontinue\\b" << "\\bdef\\b"
                            << "\\bdel\\b" << "\\belif\\b" << "\\belse\\b" << "\\bexcept\\b" << "\\bexec\\b" << "\\bfinally\\b"
                            << "\\bfor\\b" << "\\bglobal\\b" << "\\bif\\b" << "\\bimport\\b" << "\\bin\\b"
                            << "\\bis\\b" << "\\blambda\\b" << "\\bnot\\b" << "\\bor\\b" << "\\bpass\\b" << "\\bprint\\b"
                            << "\\braise\\b" << "\\breturn\\b" << "\\btry\\b" << "\\bwhile\\b" << "\\byield\\b";
                            
    valuePatternsPython     << "\\bNone\\b" << "\\bTrue\\b" << "\\bFalse\\b" << "\\bEllipsis\\b" << "\\bNotImplemented\\b";
    
    modulePatternsPython   << "\\bfrom\\b" << "\\bimport\\b" << "\\bas\\b";
    
    builtinFunctionsPatternsPython  << "\\b__import__\\b" << "\\babs\\b" << "\\ball\\b" << "\\bany\\b" << "\\bapply\\b" 
                                    << "\\bbasestring\\b" << "\\bbool\\b" << "\\bbuffer\\b" << "\\bcallable\\b" << "\\bchr\\b" 
                                    << "\\bclassmethod\\b" << "\\bcmp\\b" << "\\bcoerce\\b" << "\\bcompile\\b" << "\\bcomplex\\b" 
                                    << "\\bdelattr\\b" << "\\bdict\\b" << "\\bdir\\b" << "\\bdivmod\\b" << "\\benumerate\\b" 
                                    << "\\beval\\b" << "\\bexecfile\\b" << "\\bfile\\b" << "\\bfilter\\b" << "\\bfloat\\b" 
                                    << "\\bfrozenset\\b" << "\\bgetattr\\b" << "\\bglobals\\b" << "\\bhasattr\\b" << "\\bhash\\b" 
                                    << "\\bhex\\b" << "\\bid\\b" << "\\binput\\b" << "\\bint\\b" << "\\bintern\\b" << "\\bisinstance\\b" 
                                    << "\\bissubclass\\b" << "\\biter\\b" << "\\blen\\b" << "\\blist\\b" << "\\blocals\\b" 
                                    << "\\blong\\b" << "\\bmap\\b" << "\\bmax\\b" << "\\bmin\\b" << "\\bobject\\b" << "\\boct\\b" 
                                    << "\\bopen\\b" << "\\bord\\b" << "\\bpow\\b" << "\\bproperty\\b" << "\\brange\\b" 
                                    << "\\braw_input\\b" << "\\breduce\\b" << "\\breload\\b" << "\\brepr\\b" << "\\breversed\\b" 
                                    << "\\bround\\b" << "\\bsetattr\\b" << "\\bset\\b" << "\\bslice\\b" << "\\bsorted\\b" 
                                    << "\\bstaticmethod\\b" << "\\bstr\\b" << "\\bsum\\b" << "\\bsuper\\b" << "\\btuple\\b" 
                                    << "\\btype\\b" << "\\bunichr\\b" << "\\bunicode\\b" << "\\bvars\\b" << "\\bxrange\\b" 
                                    << "\\bzip\\b" << "\\b__self__\\b" << "\\b__name__\\b" << "\\b__debug__\\b";
                                    
    builtinObjectsPatternsPython    << "\\bArithmeticError\\b" << "\\bAssertionError\\b" << "\\bAttributeError\\b" 
                                    << "\\bEnvironmentError\\b" << "\\bEOFError\\b" << "\\bException\\b" << "\\bFloatingPointError\\b" 
                                    << "\\bImportError\\b" << "\\bIndentationError\\b" << "\\bIndexError\\b" << "\\bIOError\\b" 
                                    << "\\bKeyboardInterrupt\\b" << "\\bKeyError\\b" << "\\bLookupError\\b" << "\\bMemoryError\\b" 
                                    << "\\bNameError\\b" << "\\bNotImplementedError\\b" << "\\bOSError\\b" << "\\bOverflowError\\b" 
                                    << "\\bReferenceError\\b" << "\\bRuntimeError\\b" << "\\bStandardError\\b" << "\\bStopIteration\\b" 
                                    << "\\bSyntaxError\\b" << "\\bSystemError\\b" << "\\bSystemExit\\b" << "\\bTabError\\b" 
                                    << "\\bTypeError\\b" << "\\bUnboundLocalError\\b" << "\\bUnicodeDecodeError\\b" 
                                    << "\\bUnicodeEncodeError\\b" << "\\bUnicodeError\\b" << "\\bUnicodeTranslateError\\b" 
                                    << "\\bValueError\\b" << "\\bWindowsError\\b" << "\\bZeroDivisionError\\b" << "\\bWarning\\b" 
                                    << "\\bUserWarning\\b" << "\\bDeprecationWarning\\b" << "\\bPendingDeprecationWarning\\b" 
                                    << "\\bSyntaxWarning\\b" << "\\bOverflowWarning\\b" << "\\bRuntimeWarning\\b" << "\\bFutureWarning\\b";
                            
    foreach( QString pattern, keywordPatternsLua )
    {
        rule.pattern = QRegExp( pattern );
        rule.format = keywordFormat;
        highlightingRulesLua.append( rule );
    }
    
    keywordFormat.setForeground( Qt::darkRed );
    keywordFormat.setFontWeight( QFont::Bold );

    foreach( QString pattern, keywordPatternsPython )
    {
        rule.pattern = QRegExp( pattern );
        rule.format = keywordFormat;
        highlightingRulesPython.append( rule );
    }
    
    keywordFormat.setForeground( Qt::magenta );
    keywordFormat.setFontWeight( QFont::Normal );

    foreach( QString pattern, valuePatternsPython )
    {
        rule.pattern = QRegExp( pattern );
        rule.format = keywordFormat;
        highlightingRulesPython.append( rule );
    }
    
    keywordFormat.setForeground( Qt::darkMagenta );
    keywordFormat.setFontWeight( QFont::Bold );

    foreach( QString pattern, modulePatternsPython )
    {
        rule.pattern = QRegExp( pattern );
        rule.format = keywordFormat;
        highlightingRulesPython.append( rule );
    }
    
    keywordFormat.setForeground( Qt::cyan );
    keywordFormat.setFontWeight( QFont::Normal );

    foreach( QString pattern, builtinFunctionsPatternsPython )
    {
        rule.pattern = QRegExp( pattern );
        rule.format = keywordFormat;
        highlightingRulesPython.append( rule );
    }
    
    keywordFormat.setForeground( Qt::darkGreen );
    keywordFormat.setFontWeight( QFont::Bold );

    foreach( QString pattern, builtinObjectsPatternsPython )
    {
        rule.pattern = QRegExp( pattern );
        rule.format = keywordFormat;
        highlightingRulesPython.append( rule );
    }

    classFormat.setFontWeight( QFont::Bold );
    classFormat.setForeground( Qt::darkMagenta );
    rule.pattern = QRegExp( "\\bQ[A-Za-z]+\\b" );
    rule.format = classFormat;
    highlightingRulesLua.append( rule );

    functionFormat.setFontItalic( false );
    functionFormat.setFontWeight( QFont::Bold );
    functionFormat.setForeground( Qt::black );
    rule.pattern = QRegExp( "\\b[A-Za-z0-9_]+(?=\\()" );
    rule.format = functionFormat;
    highlightingRulesLua.append( rule );

    quotationFormat.setForeground( Qt::darkGreen );
    rule.pattern = QRegExp( "\"[^\"]*\"" );
    rule.format = quotationFormat;
    highlightingRulesLua.append( rule );
    
    quotationFormat.setForeground( Qt::magenta );
    rule.format = quotationFormat;
    highlightingRulesPython.append( rule );

    rule.pattern = QRegExp( "\'[^\']*\'" );
    highlightingRulesPython.append( rule );

    rule.pattern = QRegExp( "\\[\\[[^\\]\\]]*\\]\\]" );
    rule.format = quotationFormat;
    highlightingRulesLua.append( rule );

    singleLineCommentFormat.setForeground( Qt::darkGreen );
    rule.pattern = QRegExp( "--[^\n]*" );//\\[\\]]*" );
    rule.format = singleLineCommentFormat;
    highlightingRulesLua.append( rule );
    
    singleLineCommentFormat.setForeground( Qt::blue );
    rule.pattern = QRegExp( "#[^\n]*" );
    rule.format = singleLineCommentFormat;
    highlightingRulesPython.append( rule );

    multiLineCommentFormat.setForeground( Qt::darkGreen );

    commentStartExpression = QRegExp( "\\[\\[" );
    commentEndExpression = QRegExp( "\\]\\]" );
    
    multiLineCommentFormatPython.setForeground( Qt::magenta );

    commentStartExpressionPython1 = QRegExp( "\"\"\""  );
    commentEndExpressionPython1 = QRegExp( "\"\"\"" );
    
    commentStartExpressionPython2 = QRegExp( "\'\'\'"  );
    commentEndExpressionPython2 = QRegExp( "\'\'\'" );

    mSearchPattern = "MudletTheMUDClient";

    searchFormat.setForeground( QColor(0,0,0) );
    searchFormat.setBackground( QColor(255,255,0) );

    rule.pattern = QRegExp( mSearchPattern );
    rule.format = searchFormat;
    highlightingRulesLua.append( rule );
    highlightingRulesPython.append( rule );
}

void THighlighter::setSearchPattern( QString p )
{
    HighlightingRule rule;
    mSearchPattern = QRegExp::escape( p );
    searchFormat.setForeground( QColor(0,0,0) );
    searchFormat.setBackground( QColor(255,255,0) );

    rule.pattern = QRegExp( mSearchPattern );
    rule.pattern.setCaseSensitivity(Qt::CaseInsensitive);
    rule.format = searchFormat;
    highlightingRulesLua.last() = rule;
}

void THighlighter::highlightBlock( const QString & text )
{
    if( text.size() < 1 ) return;
    QVector<HighlightingRule> highlightingRules = highlightingRulesLua;
    if ( editor->getScriptLanguageCode() == PYTHON)
    {
        highlightingRules = highlightingRulesPython;
    }
    
    foreach( HighlightingRule rule, highlightingRules )
    {
        QRegExp expression( rule.pattern );
        int index = text.indexOf( expression );
        while( index >= 0 )
        {
            int length = expression.matchedLength();
            setFormat( index, length, rule.format );
            index = text.indexOf( expression, index + length );
        }
    }

    setCurrentBlockState( 0 );
    int startIndex = 0;
    if( previousBlockState() != 1 )
    {
        if ( editor->getScriptLanguageCode() == PYTHON)
        {
            startIndex = text.indexOf( commentStartExpressionPython1 );
            if (startIndex == -1)
            {
                startIndex = text.indexOf( commentStartExpressionPython2 );
                commentTypeSingleQuotePython=true;
            }
            else
            {
                commentTypeSingleQuotePython=false;
            }
        }
        else
        {
            startIndex = text.indexOf( commentStartExpression );
        }
    }
    while( startIndex >= 0 )
    {
        int endIndex,commentSyntaxLength;
        if ( editor->getScriptLanguageCode() == PYTHON)
        {
            commentSyntaxLength=3;
            if (commentTypeSingleQuotePython)
            {
                endIndex = text.indexOf( commentEndExpressionPython2, startIndex+1 );
            }
            else
            {
                endIndex = text.indexOf( commentEndExpressionPython1, startIndex+1 );
            }
        }
        else
        {
            commentSyntaxLength=2;
            endIndex = text.indexOf( commentEndExpression, startIndex );
        }
        int commentLength;
        if( endIndex == -1 )
        {
            setCurrentBlockState( 1 );
            commentLength = text.length() - startIndex;
        }
        else
        {
            commentLength = endIndex - startIndex + commentSyntaxLength;
        }
        if ( editor->getScriptLanguageCode() == PYTHON)
        {
            int startIndexTmp;
            setFormat( startIndex, commentLength, multiLineCommentFormatPython );
            startIndexTmp = text.indexOf( commentStartExpressionPython1, startIndex + commentLength );
            startIndex = text.indexOf( commentStartExpressionPython2, startIndex + commentLength );
            if (startIndexTmp != -1 && startIndexTmp<startIndex)
            {
                startIndex=startIndexTmp;
            }            
        }
        else
        {
            setFormat( startIndex, commentLength, multiLineCommentFormat );
            startIndex = text.indexOf( commentStartExpression, startIndex + commentLength );
        }
    }

    QRegExp expression( highlightingRules.last().pattern );
    int index = text.indexOf( mSearchPattern, 0, Qt::CaseInsensitive );//expression );
    while( index >= 0 )
    {
        int length = mSearchPattern.length();//expression.matchedLength();
        setFormat( index, length, highlightingRules.last().format );
        //index = text.indexOf( expression, index + length );
        index = text.indexOf( mSearchPattern, index + length, Qt::CaseInsensitive );
    }
}

