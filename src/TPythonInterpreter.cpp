/***************************************************************************
 *   Copyright (C) 2011 by Kenneth Chik                                    *
 *   kenneth.chik@gmail.com                                                *
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

#include "PythonQt.h"
#include "TPythonInterpreter.h"
#include "Host.h"
#include "TEvent.h"
#include "mudlet.h"
#include <QString>
#include <QStringList>
#include <QList>
#include <QDateTime>
#include <QHash>
#include <string>
#include <list>

QHash<QString, Host *> TPythonInterpreter::pythonHostMap;

TPythonInterpreter::TPythonInterpreter(Host * pH)
:mpHost( pH )
,mpInitialized(false)
{
    try
    {
        // init PythonQt and Python
        PythonQt::init();
        mainModule = PythonQt::self()->createUniqueModule();
        mpHost->mPythonWorking = true;
    }
    catch(...)
    {
        mpHost->mPython=false;
        mpHost->mPythonWorking = false;
    }    
}

void TPythonInterpreter::init()
{   
    if (mpHost->pythonEnabled())
    {
        connect(PythonQt::self(),SIGNAL(pythonStdOut(const QString&)),this, SLOT(slotEchoMessage(const QString&)));
        connect(PythonQt::self(),SIGNAL(pythonStdErr(const QString&)),this, SLOT(slotEchoMessage(const QString&)));
        
        QString hashString = QDateTime::currentDateTime().toString() + QString(mpHost->mHostID);
        pythonHostMap[hashString]=mpHost;
        const QString& varHostHash = "HOST_HASH";
        const QVariant& varHostHashValue = hashString;
        add_python_variable(varHostHash,varHostHashValue);
        const QString& varHostName = "HOST_NAME";
        const QVariant& varHostNameValue = mpHost->getName();
        add_python_variable(varHostName,varHostNameValue);
        const QString& varHostLogin = "HOST_LOGIN";
        const QVariant& varHostLoginValue = mpHost->getLogin();
        add_python_variable(varHostLogin,varHostLoginValue);
        PythonQt::self()->registerCPPClass("MudletObject", "","mudlet", PythonQtCreateObject<MudletObjectWrapper>);
        
        QString dirPath = QCoreApplication::applicationDirPath();
        mainModule.evalFile(dirPath + "/PythonGlobal.py");
        mpInitialized = true;
    }
}

void TPythonInterpreter::add_python_variable( const QString & varName, const QVariant & var) {
    if (mpHost->pythonEnabled())
    {
        mainModule.addVariable(varName,var);
    }
}

void TPythonInterpreter::callEventHandler( const QString & function, const TEvent * pE ) {
    if (mpHost->pythonEnabled())
    {
        QVariantList vl = QVariantList();
        for( int i=0; i<pE->mArgumentList.size(); i++ )
        {
            if( pE->mArgumentTypeList[i] == ARGUMENT_TYPE_NUMBER )
            {
                vl << pE->mArgumentList[i].toInt();
            }
            else
            {
                vl << pE->mArgumentList[i];
            }
        }
        mainModule.call(function,vl);
    }
}

void TPythonInterpreter::executeScript( const QString & code ) {
    if (mpHost->pythonEnabled())
    {
        mainModule.evalScript(code);
    }
}

void TPythonInterpreter::call(const QString &callable) {
    if (mpHost->pythonEnabled())
    {
        std::list<std::string> captureGroupList = (mpHost->getLuaInterpreter())->mCaptureGroupList;
        if( captureGroupList.size() > 0 )
        {
            QStringList qCaptureGroupList;
            std::list< std::string >::iterator mit = captureGroupList.begin();
            for (; mit!=captureGroupList.end(); mit++)
            {
                qCaptureGroupList << QString((*mit).data());
            }

            add_python_variable("matches",QVariant(qCaptureGroupList));
        }
        mainModule.call(callable);
    }
}

void TPythonInterpreter::callMulti( QString & function)
{
    if (mpHost->pythonEnabled())
    {
        std::list< std::list<std::string> > multiCaptureGroupList = (mpHost->getLuaInterpreter())->mMultiCaptureGroupList;
        if( multiCaptureGroupList.size() > 0 )
        {
            QList<QVariant> mmatches = QList<QVariant>();
            QStringList sl;
            std::list< std::list<std::string> >::iterator mit = multiCaptureGroupList.begin();
            for( ; mit!=multiCaptureGroupList.end(); mit++)
            {
                sl = QStringList();
                std::list<std::string>::iterator it = (*mit).begin();
                for( ; it!=(*mit).end(); it++)
                {
                    sl << QString((*it).data());
                }
                mmatches << QVariant(sl);
            }

            add_python_variable("multimatches",QVariant(mmatches));
        }

        mainModule.call(function);
    }
}

QString TPythonInterpreter::wrapCode(QString funcName, QString code, QString name)
{
    return QString("def %1():\n    try:\n        'main python code'\n        %2\n    except:\n        printFixedStackTrace(traceback.format_exc(),'%3')").arg(funcName).arg(code.replace("\n","\n        ")).arg(name);
}

void TPythonInterpreter::setAtcpTable( QString & var, QString & value )
{
    if (mpHost->pythonEnabled())
    {
        QString pyscript = QString("atcp['%1']='%2'").arg(var).arg(value);
        executeScript(pyscript);
    }
}

void TPythonInterpreter::setChannel102Table( int & var, int & value )
{
    if (mpHost->pythonEnabled())
    {
        QString pyscript = QString("channel102['%1']='%2'").arg(var).arg(value);
        executeScript(pyscript);
    }
}

void TPythonInterpreter::setGMCPTable(QString & key, QString & string_data)
{
    if (mpHost->pythonEnabled())
    {
        QStringList tokenList = key.split(".");
        QString pKey = "";
        QString keyTemplate = "['%1']";
        QString pyscript;
        for( int i = 0; i<tokenList.size(); i++ )
        {
            pKey += keyTemplate.arg(tokenList[i]);
        }
        if (string_data.isEmpty() || string_data.isNull())
        {
            pyscript = QString("try:\n    gmcp%1={}\nexcept SyntaxError:\n    ''").arg(pKey);        
            executeScript(pyscript);
        }
        else if (string_data[0] == '{')
        {
            pyscript = QString("try:\n    gmcp%1.update(%2)\nexcept SyntaxError:\n    ''").arg(pKey).arg(string_data);
            executeScript(pyscript);
        }
        else if (!string_data[0].isLetter())
        {
            pyscript = QString("try:\n    gmcp%1=%2\nexcept SyntaxError:\n    ''").arg(pKey).arg(string_data);
            executeScript(pyscript);
        }
    }
}

void TPythonInterpreter::slotEchoMessage(const QString & msg)
{
    mpHost->mpConsole->echo( const_cast<QString&>(msg) );
}

MudletObject::MudletObject(const QString& hash)
{
    mpHost = TPythonInterpreter::pythonHostMap[hash];
}

int MudletObjectWrapper::selectString(MudletObject* o, QString& text, int numMatch, QString& console)
{
    if (console == "main")
    {
        return o->mpHost->mpConsole->select( text, numMatch );
    }
    else
    {
        return mudlet::self()->selectString( o->mpHost, console, text, numMatch );
    }
}

void MudletObjectWrapper::send(MudletObject* o, QString& command, bool echo)
{
    o->mpHost->send( command,echo, true );
}

void MudletObjectWrapper::expandAlias( MudletObject* o, QString& command, bool print )
{
    o->mpHost->send( command,print, false );
}

void MudletObjectWrapper::resetFormat( MudletObject* o, QString& console )
{
    if (console == "main")
    {
        o->mpHost->mpConsole->reset();
    }
    else
    {
        mudlet::self()->resetFormat( o->mpHost, console );
    }
}

void MudletObjectWrapper::setBgColor( MudletObject* o, int r,int g,int b, QString& console )
{
    if (console == "main")
    {
        o->mpHost->mpConsole->setBgColor( r, g, b );
    }
    else
    {
        mudlet::self()->setBgColor( o->mpHost, console, r, g, b );
    }
}

void MudletObjectWrapper::setFgColor( MudletObject* o, int r,int g,int b, QString& console )
{
    if (console == "main")
    {
        o->mpHost->mpConsole->setFgColor( r, g, b );
    }
    else
    {
        mudlet::self()->setFgColor( o->mpHost, console, r, g, b );
    }
}

bool MudletObjectWrapper::enableTimer( MudletObject* o, QString& name )
{
    return o->mpHost->getTimerUnit()->enableTimer( name );
}

bool MudletObjectWrapper::enableKey( MudletObject* o, QString& name )
{
    return o->mpHost->getKeyUnit()->enableKey( name );
}

bool MudletObjectWrapper::enableTrigger( MudletObject* o, QString& name )
{
    return o->mpHost->getTriggerUnit()->enableTrigger( name );
}

bool MudletObjectWrapper::enableAlias( MudletObject* o, QString& name )
{
    return o->mpHost->getAliasUnit()->enableAlias( name );
}

bool MudletObjectWrapper::disableTimer( MudletObject* o, QString& name )
{
    return o->mpHost->getTimerUnit()->disableTimer( name );
}

bool MudletObjectWrapper::disableKey( MudletObject* o, QString& name )
{
    return o->mpHost->getKeyUnit()->disableKey( name );
}

bool MudletObjectWrapper::disableTrigger( MudletObject* o, QString& name )
{
    return o->mpHost->getTriggerUnit()->disableTrigger( name );
}

bool MudletObjectWrapper::disableAlias( MudletObject* o, QString& name )
{
    return o->mpHost->getAliasUnit()->disableAlias( name );
}

int MudletObjectWrapper::selectCaptureGroup( MudletObject* o, int groupNumber )
{
    int luaNumOfMatch = groupNumber;
    luaNumOfMatch--; //we want capture groups to start with 1 instead of 0
    if( luaNumOfMatch < static_cast<int>(o->mpHost->getLuaInterpreter()->mCaptureGroupList.size()) )
    {
        TLuaInterpreter * pL = o->mpHost->getLuaInterpreter();
        std::list<std::string>::iterator its = pL->mCaptureGroupList.begin();
        std::list<int>::iterator iti = pL->mCaptureGroupPosList.begin();

        for( int i=0; iti!=pL->mCaptureGroupPosList.end(); ++iti,++i )
        {
            if( i >= luaNumOfMatch ) break;
        }
        for( int i=0; its!=pL->mCaptureGroupList.end(); ++its,++i)
        {
            if( i >= luaNumOfMatch ) break;
        }

        int begin = *iti;
        std::string & s = *its;
        int length = s.size();
        return o->mpHost->mpConsole->selectSection( begin, length );
    }
}

void MudletObjectWrapper::replace( MudletObject* o, QString& with, QString& console )
{
    if (console == "main")
    {
        o->mpHost->mpConsole->replace( with );
    }
    else
    {
        mudlet::self()->replace( o->mpHost, console, with );
    }
}

void MudletObjectWrapper::replaceAll( MudletObject* o, QString& what, QString& with, QString& console )
{
    if (console == "main")
    {
        while (o->mpHost->mpConsole->select( what, 1) > -1)
        {
            o->mpHost->mpConsole->replace( with);
        }
    }
    else
    {
        while (mudlet::self()->selectString( o->mpHost, console, what, 1 ) > -1)
        {
            mudlet::self()->replace( o->mpHost, console, with );
        }
    }
}

void MudletObjectWrapper::deleteLine( MudletObject* o, QString& console )
{
    if (console == "main")
    {
        o->mpHost->mpConsole->skipLine();
    }
    else
    {
        mudlet::self()->deleteLine( o->mpHost, console );
    }
}

bool MudletObjectWrapper::selectSection( MudletObject* o, int from, int length_of_string, QString& console )
{
    if (console == "main")
    {
        return o->mpHost->mpConsole->selectSection( from, length_of_string );
    }
    else
    {
        return mudlet::self()->selectSection( o->mpHost, console, from, length_of_string );
    }
}

void MudletObjectWrapper::raiseEvent( MudletObject* o, QVariantList args )
{
    TEvent * pE = new TEvent;
    for (int i=0;i<args.size();++i)
    {
        if (args[i].type() == QVariant::String)
        {
            pE->mArgumentList.append( args[i].toString() );
            pE->mArgumentTypeList.append( ARGUMENT_TYPE_STRING );
        }
        else
        {
            pE->mArgumentList.append( args[i].toString() );
            pE->mArgumentTypeList.append( ARGUMENT_TYPE_NUMBER );
        }
    }
    o->mpHost->raiseEvent( pE );
}

void MudletObjectWrapper::reconnect( MudletObject* o )
{
    o->mpHost->mTelnet.connectIt( o->mpHost->getUrl(), o->mpHost->getPort() );
}

void MudletObjectWrapper::disconnect( MudletObject* o )
{
    o->mpHost->mTelnet.disconnect();
}


int MudletObjectWrapper::sendGMCP( MudletObject* o, QString& themsg )
{
    QString _h;
    _h += TN_IAC;
    _h += TN_SB;
    _h += GMCP;
    _h += themsg;
    _h += TN_IAC;
    _h += TN_SE;

    std::string toSend = _h.toStdString();

    o->mpHost->mTelnet.socketOutRaw( toSend );
    return 0;
}

int MudletObjectWrapper::echo( MudletObject* o, QString& themsg, QString& console)
{
    if (console == "main")
    {
        o->mpHost->mpConsole->echo( themsg );
    }
    else
    {
        mudlet::self()->echoWindow( o->mpHost, console, themsg );
    }
    return 0;
}

int MudletObjectWrapper::echoLink( MudletObject* o, QString& themsg, QString& thefunc, QString& tooltip, QString& console, bool customFormat)
{
    QStringList func;
    QStringList tip;
    func <<  thefunc;
    tip << tooltip;
    func.prepend("PYTHON");

    if (console == "main")
    {
        o->mpHost->mpConsole->echoLink( themsg, func, tip, customFormat );
    }
    else
    {
        mudlet::self()->echoLink( o->mpHost, console, themsg, func, tip, customFormat  );
    }
    return 0;
}

int MudletObjectWrapper::insertLink(MudletObject* o, QString& themsg, QString& thefunc, QString& tooltip, QString& console)
{
    QStringList func;
    QStringList tip;
    func <<  thefunc;
    tip << tooltip;
    func.prepend("PYTHON");

    if (console == "main")
    {
        o->mpHost->mpConsole->insertLink( themsg, func, tip);
    }
    else
    {
        mudlet::self()->insertLink( o->mpHost, console, themsg, func, tip  );
    }
    return 0;
}

int MudletObjectWrapper::setLink( MudletObject* o,QString& thefunc, QString& tooltip, QString& console)
{
    QStringList func;
    QStringList tip;
    func <<  thefunc;
    tip << tooltip;
    func.prepend("PYTHON");
    QString linkTxt="";

    if (console == "main")
    {
        o->mpHost->mpConsole->setLink( linkTxt, func, tip );
    }
    else
    {
        mudlet::self()->setLink( o->mpHost, console, linkTxt, func, tip  );
    }
    return 0;
}

int MudletObjectWrapper::createBuffer(MudletObject* o, QString& name)
{
    mudlet::self()->createBuffer( o->mpHost, name );
    return 0;
}

int MudletObjectWrapper::appendBuffer(MudletObject* o, QString& console)
{
    {
        if (console == "main")
        {
            o->mpHost->mpConsole->appendBuffer();
        }
        else
        {
            mudlet::self()->appendBuffer( o->mpHost, console );
        }
        return 0;
    }
}

int MudletObjectWrapper::getLineNumber(MudletObject* o)
{
    return o->mpHost->mpConsole->getLineNumber();
}

int MudletObjectWrapper::copy(MudletObject* o, QString& console)
{
    {
        if (console == "main")
        {
            o->mpHost->mpConsole->copy();;
        }
        else
        {
            mudlet::self()->copy( o->mpHost, console );
        }
        return 0;
    }
}

int MudletObjectWrapper::cut(MudletObject* o)
{
    o->mpHost->mpConsole->cut();
    return 0;
}


int MudletObjectWrapper::paste(MudletObject* o, QString& console)
{
    {
        if (console == "main")
        {
            o->mpHost->mpConsole->paste();;
        }
        else
        {
            mudlet::self()->pasteWindow( o->mpHost, console );
        }
        return 0;
    }
}

int MudletObjectWrapper::feedTriggers(MudletObject* o, QString& txt)
{
    std::string toShow = txt.toStdString();
    o->mpHost->mpConsole->printOnDisplay( toShow );
    return 0;
}

int MudletObjectWrapper::setBold(MudletObject* o, QString& console, bool active)
{
    {
        if (console == "main")
        {
            o->mpHost->mpConsole->setBold(active);
        }
        else
        {
            mudlet::self()->setBold( o->mpHost, console, active );
        }
        return 0;
    }
}

int MudletObjectWrapper::setItalics(MudletObject* o, QString& console, bool active)
{
    {
        if (console == "main")
        {
            o->mpHost->mpConsole->setItalics(active);
        }
        else
        {
            mudlet::self()->setItalics( o->mpHost, console, active );
        }
        return 0;
    }
}

int MudletObjectWrapper::setUnderline(MudletObject* o, QString& console, bool active)
{
    {
        if (console == "main")
        {
            o->mpHost->mpConsole->setUnderline(active);
        }
        else
        {
            mudlet::self()->setUnderline( o->mpHost, console, active );
        }
        return 0;
    }
}

int MudletObjectWrapper::echoPopup(MudletObject* o, QString& themsg, QStringList& func, QStringList& tooltip, QString& console, bool customFormat)
{
    func.prepend("PYTHON");
    tooltip.prepend("");
    if (console == "main")
    {
        o->mpHost->mpConsole->echoLink( themsg, func, tooltip, customFormat );
    }
    else
    {
        mudlet::self()->echoLink( o->mpHost, console, themsg, func, tooltip, customFormat  );
    }
    return 0;
}

int MudletObjectWrapper::setPopup(MudletObject* o, QStringList& func, QStringList& tip, QString& console)
{
    QString linkTxt="";
    func.prepend("PYTHON");
    tip.prepend("");

    if (console == "main")
    {
        o->mpHost->mpConsole->setLink( linkTxt, func, tip );
    }
    else
    {
        mudlet::self()->setLink( o->mpHost, console, linkTxt, func, tip  );
    }
    return 0;
}

int MudletObjectWrapper::insertPopup(MudletObject* o, QString& themsg, QStringList& func, QStringList& tooltip, QString& console)
{
    func.prepend("PYTHON");
    tooltip.prepend("");

    if (console == "main")
    {
        o->mpHost->mpConsole->insertLink( themsg, func, tooltip);
    }
    else
    {
        mudlet::self()->insertLink( o->mpHost, console, themsg, func, tooltip  );
    }
    return 0;
}

int MudletObjectWrapper::moveCursor(MudletObject* o, int xpos, int ypos, QString& console)
{
    if (console == "main")
    {
        o->mpHost->mpConsole->moveCursor( xpos, ypos);
    }
    else
    {
        mudlet::self()->moveCursor( o->mpHost, console, xpos, ypos  );
    }
    return 1;
}

int MudletObjectWrapper::moveCursorEnd(MudletObject* o, QString& console)
{
    if (console == "main")
    {
        o->mpHost->mpConsole->moveCursorEnd();
    }
    else
    {
        mudlet::self()->moveCursorEnd( o->mpHost, console );
    }
    return 0;
}

int MudletObjectWrapper::pasteWindow(MudletObject* o, QString& console)
{
    mudlet::self()->pasteWindow( o->mpHost, console );
    return 0;
}

int MudletObjectWrapper::selectCurrentLine(MudletObject* o, QString& console)
{
    std::string _console = console.toStdString();
    o->mpHost->mpConsole->selectCurrentLine( _console);
    return 0;
}

int MudletObjectWrapper::wrapLine(MudletObject* o, int linenum, QString& console)
{
    std::string _console = console.toStdString();
    o->mpHost->mpConsole->luaWrapLine(_console, linenum);
    return 0;
}

QList<int> MudletObjectWrapper::getFgColor(MudletObject* o, QString& console)
{
    std::string _console = console.toStdString();
    std::list<int> result;
    result = o->mpHost->mpConsole->getFgColor( _console );
    QList<int> _result = QList<int>::fromStdList(result);
    return _result;
}

QList<int> MudletObjectWrapper::getBgColor(MudletObject* o, QString& console)
{
    std::string _console = console.toStdString();
    std::list<int> result;
    result = o->mpHost->mpConsole->getBgColor( _console );
    QList<int> _result = QList<int>::fromStdList(result);
    return _result;
}

QString MudletObjectWrapper::getCurrentLine(MudletObject* o, QString& console)
{
    std::string _console = console.toStdString();
    QString line = o->mpHost->mpConsole->getCurrentLine( _console );
    return line;
}

int MudletObjectWrapper::insertHTML(MudletObject* o, QString& themsg )
{
    o->mpHost->mpConsole->insertHTML( themsg);
    return 0;
}

int MudletObjectWrapper::insertText(MudletObject* o, QString& themsg, QString& console )
{
    if (console == "main")
    {
        o->mpHost->mpConsole->insertText( themsg);
    }
    else
    {
        mudlet::self()->insertText( o->mpHost, console, themsg );
    }
    return 0;
}

bool MudletObjectWrapper::isAnsiFgColor(MudletObject* o, int ansiFg, QString& console)
{
    std::string _console = console.toStdString();
    std::list<int> result;
    result = o->mpHost->mpConsole->getFgColor( _console );
    typedef std::list<int>::iterator IT;
    IT it=result.begin();
    if( result.size() < 3 ) return false;
    if( ansiFg < 0 ) return false;
    if( ansiFg > 16 ) return false;


    QColor c;
    switch( ansiFg )
    {
        case 0: c = o->mpHost->mFgColor;  break;
        case 1: c = o->mpHost->mLightBlack; break;
        case 2: c = o->mpHost->mBlack; break;
        case 3: c = o->mpHost->mLightRed; break;
        case 4: c = o->mpHost->mRed; break;
        case 5: c = o->mpHost->mLightGreen; break;
        case 6: c = o->mpHost->mGreen; break;
        case 7: c = o->mpHost->mLightYellow; break;
        case 8: c = o->mpHost->mYellow; break;
        case 9: c = o->mpHost->mLightBlue; break;
        case 10: c = o->mpHost->mBlue; break;
        case 11: c = o->mpHost->mLightMagenta; break;
        case 12: c = o->mpHost->mMagenta; break;
        case 13: c = o->mpHost->mLightCyan; break;
        case 14: c = o->mpHost->mCyan; break;
        case 15: c = o->mpHost->mLightWhite; break;
        case 16: c = o->mpHost->mWhite; break;
    }

    int val = *it;
    if( val == c.red() )
    {
        it++;
        val = *it;
        if( val == c.green() )
        {
            it++;
            val = *it;
            if( val == c.blue() )
            {
                return true;
            }
        }
    }
    return false;
}

bool MudletObjectWrapper::isAnsiBgColor(MudletObject* o, int ansiBg, QString& console)
{
    std::string _console = console.toStdString();
    std::list<int> result;
    result = o->mpHost->mpConsole->getBgColor( _console );
    typedef std::list<int>::iterator IT;
    IT it=result.begin();
    if( result.size() < 3 ) return false;
    if( ansiBg < 0 ) return false;
    if( ansiBg > 16 ) return false;


    QColor c;
    switch( ansiBg )
    {
        case 0: c = o->mpHost->mFgColor;  break;
        case 1: c = o->mpHost->mLightBlack; break;
        case 2: c = o->mpHost->mBlack; break;
        case 3: c = o->mpHost->mLightRed; break;
        case 4: c = o->mpHost->mRed; break;
        case 5: c = o->mpHost->mLightGreen; break;
        case 6: c = o->mpHost->mGreen; break;
        case 7: c = o->mpHost->mLightYellow; break;
        case 8: c = o->mpHost->mYellow; break;
        case 9: c = o->mpHost->mLightBlue; break;
        case 10: c = o->mpHost->mBlue; break;
        case 11: c = o->mpHost->mLightMagenta; break;
        case 12: c = o->mpHost->mMagenta; break;
        case 13: c = o->mpHost->mLightCyan; break;
        case 14: c = o->mpHost->mCyan; break;
        case 15: c = o->mpHost->mLightWhite; break;
        case 16: c = o->mpHost->mWhite; break;
    }

    int val = *it;
    if( val == c.red() )
    {
        it++;
        val = *it;
        if( val == c.green() )
        {
            it++;
            val = *it;
            if( val == c.blue() )
            {
                return true;
            }
        }
    }
    return false;
}
