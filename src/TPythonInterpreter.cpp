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
        const QVariant& varHostValue = hashString;
        add_python_variable(varHostHash,varHostValue);
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
    return QString("def %1():\n    try:\n        'main python code'\n        %2\n    except:\n        printFixedStackTrace(traceback.format_exc(),'%3')").arg(funcName).arg(code.replace("\n","\n        ").replace("\t","    ")).arg(name);
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

