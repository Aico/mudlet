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


TPythonInterpreter::TPythonInterpreter(Host * pH)
:mpHost( pH )
,mpInitialized(false)
{
    // init PythonQt and Python
    PythonQt::init();
    mainModule = PythonQt::self()->createUniqueModule();
}

void TPythonInterpreter::init()
{   
    if (mpHost->pythonEnabled())
    {
        connect(PythonQt::self(),SIGNAL(pythonStdOut(const QString&)),this, SLOT(slotEchoMessage(const QString&)));
        connect(PythonQt::self(),SIGNAL(pythonStdErr(const QString&)),this, SLOT(slotEchoMessage(const QString&)));
        
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
    return QString("def %1():\n    try:\n        'main python code'\n        %2\n    except:\n        printFixedStackTrace(traceback.format_exc(),'%3')").arg(funcName).arg(code.replace("\n","\n        ")).arg(name);
}

void TPythonInterpreter::slotEchoMessage(const QString & msg)
{
    mpHost->mpConsole->echo( const_cast<QString&>(msg) );
}

    
