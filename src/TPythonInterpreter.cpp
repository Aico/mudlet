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
#include <string>
#include <list>

const QString METHOD_DELIMIT = "^^MUDLET_DELIMIT^^";
const QString METHOD_SUBDELIMIT = "^^MUDLET_SUBDELIMIT^^";

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
        
        const QString& varNameString = "ARGUMENT_TYPE_STRING";
        const QString& varNameNumber = "ARGUMENT_TYPE_NUMBER";
        const QVariant& varValueString = ARGUMENT_TYPE_STRING;
        const QVariant& varValueNumber = ARGUMENT_TYPE_NUMBER;
        add_python_variable(varNameString,varValueString);
        add_python_variable(varNameNumber,varValueNumber);
        mainModule.evalFile("PythonGlobal.py");
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

void TPythonInterpreter::runMethod(const QString& msg)
{
    QStringList list = msg.split(METHOD_DELIMIT);
    try
    {    
        QString method = list[1];
        std::string s;
        if (method == "send")
        {
            //s = list[2].toStdString();
            mpHost->send( list[2], list[3].toInt(), true );
        }
        else if (method == "expandAlias")
        {
            mpHost->send( list[2], list[3].toInt(), false );
        }
        else if (method == "selectString")
        {
            if (list[4] == "main")
            {
                mpHost->mpConsole->select( list[2], list[3].toInt() );
            }
            else
            {
                mudlet::self()->selectString( mpHost, list[4], list[2], list[3].toInt() );
            }
        }
        else if (method == "resetFormat")
        {
            if (list[2] == "main")
            {
                mpHost->mpConsole->reset();
            }
            else
            {
                mudlet::self()->resetFormat( mpHost, list[2] );
            }
        }
        else if (method == "setBgColor")
        {
            if (list[5] == "main")
            {
                mpHost->mpConsole->setBgColor( list[2].toInt(), list[3].toInt(), list[4].toInt() );
            }
            else
            {
                mudlet::self()->setBgColor( mpHost, list[5], list[2].toInt(), list[3].toInt(), list[4].toInt() );
            }
        }
        else if (method == "setFgColor")
        {
            if (list[5] == "main")
            {
                mpHost->mpConsole->setFgColor( list[2].toInt(), list[3].toInt(), list[4].toInt() );
            }
            else
            {
                mudlet::self()->setFgColor( mpHost, list[5], list[2].toInt(), list[3].toInt(), list[4].toInt() );
            }
        }
        else if (method == "enableTimer")
        {
            mpHost->getTimerUnit()->enableTimer( list[2] );
        }
        else if (method == "enableKey")
        {
            mpHost->getKeyUnit()->enableKey( list[2] );
        }
        else if (method == "enableTrigger")
        {
            mpHost->getTriggerUnit()->enableTrigger( list[2] );
        }
        else if (method == "enableAlias")
        {
            mpHost->getAliasUnit()->enableAlias( list[2] );
        }
        else if (method == "disableTimer")
        {
            mpHost->getTimerUnit()->disableTimer( list[2] );
        }
        else if (method == "disableKey")
        {
            mpHost->getKeyUnit()->disableKey( list[2] );
        }
        else if (method == "disableTrigger")
        {
            mpHost->getTriggerUnit()->disableTrigger( list[2] );
        }
        else if (method == "disableAlias")
        {
            mpHost->getAliasUnit()->disableAlias( list[2] );
        }
        else if (method == "selectCaptureGroup")
        {
            int luaNumOfMatch = list[2].toInt();
            luaNumOfMatch--; //we want capture groups to start with 1 instead of 0
            if( luaNumOfMatch < static_cast<int>(mpHost->getLuaInterpreter()->mCaptureGroupList.size()) )
            {
                TLuaInterpreter * pL = mpHost->getLuaInterpreter();
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
                mpHost->mpConsole->selectSection( begin, length );
            }
        }
        else if (method == "replace")
        {
            if (list[3] == "main")
            {
                mpHost->mpConsole->replace( list[2] );
            }
            else
            {
                mudlet::self()->replace( mpHost, list[3], list[2] );
            }
        }
        else if (method == "replaceAll")
        {
            if (list[4] == "main")
            {
                while (mpHost->mpConsole->select( list[2], 1) > -1)
                {
                    mpHost->mpConsole->replace( list[3]);
                }
            }
            else
            {
                while (mudlet::self()->selectString( mpHost, list[4], list[2], 1 ) > -1)
                {
                    mudlet::self()->replace( mpHost, list[4], list[3] );
                }
            }
        }
        else if (method == "selectSection")
        {
            if (list[4] == "main")
            {
                mpHost->mpConsole->selectSection( list[2].toInt(), list[3].toInt() );
            }
            else
            {
                mudlet::self()->selectSection( mpHost, list[4], list[2].toInt(), list[3].toInt() );
            }
        }
        else if (method == "deleteLine")
        {
            if (list[2] == "main")
            {
                mpHost->mpConsole->skipLine();
            }
            else
            {
                mudlet::self()->deleteLine( mpHost, list[2] );
            }
        }
        else if (method == "raiseEvent")
        {
            TEvent * pE = new TEvent;
            QStringList l;
            for (int i=2;i<list.length();++i)
            {
                l=list[i].split(METHOD_SUBDELIMIT);
                if (l[1].toInt() == ARGUMENT_TYPE_NUMBER)
                {
                    pE->mArgumentList.append( l[0] );
                    pE->mArgumentTypeList.append( ARGUMENT_TYPE_NUMBER );
                }
                else if (l[1].toInt() == ARGUMENT_TYPE_STRING)
                {
                    pE->mArgumentList.append( l[0] );
                    pE->mArgumentTypeList.append( ARGUMENT_TYPE_STRING );
                }
                else
                {
                    QString err1 = "[ ERROR ] Event " + list[2][0] + " argument value not of valid type";
                    mpHost->mpConsole->print( err1, 150, 0, 0, 0, 0, 0 );
                }
            }
            mpHost->raiseEvent( pE );
        }
        else
        {
            QString err = "[ ERROR ] " + method + " is not a valid python method!";
            mpHost->mpConsole->print( err, 150, 0, 0, 0, 0, 0 );
        }
    }
    catch(...)
    {
        QString err;
        if (list.size()>1)
        {
            err = "[ ERROR ] Error occured executing python method " + list[1];
        }
        else
        {
            err = "[ ERROR ] Python method call " + msg + " mangled.";
        }
        mpHost->mpConsole->print( err, 150, 0, 0, 0, 0, 0 );
    }
}

void TPythonInterpreter::slotEchoMessage(const QString & msg)
{
    if (msg.startsWith(METHOD_DELIMIT))
    {
        runMethod(msg);
    }
    else
    {
        mpHost->mpConsole->echo( const_cast<QString&>(msg) );
    }
}
   
