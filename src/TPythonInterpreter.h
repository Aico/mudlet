#ifndef PYTHON_INTERPRETER_H
#define PYTHON_INTERPRETER_H

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
#include "TEvent.h"
#include <QString>
#include <QObject>
#include <QHash>

class Host;

class TPythonInterpreter : public QObject
{
Q_OBJECT

public:
    TPythonInterpreter(Host* pH);
    void init();
    void add_python_variable( const QString & varName, const QVariant & var);
    void callEventHandler( const QString & function, const TEvent * pE );
    void executeScript( const QString & code );
    void call(const QString &callable);
    void callMulti( QString & function);
    QString wrapCode(QString funcName, QString code, QString name);
    void setAtcpTable( QString & var, QString & value );
    void setChannel102Table( int & var, int & value );
    void setGMCPTable(QString & key, QString & string_data);
    bool isInitialized() {return mpInitialized;}
    static QHash<QString, Host *> pythonHostMap;
    
public slots:
    void slotEchoMessage(const QString & msg);

private:
    PythonQtObjectPtr mainModule;
    Host * mpHost;
    bool mpInitialized;
    void runMethod(const QString& msg);
};

class MudletObject {
public:
  MudletObject() {}
  MudletObject(const QString& hash);

  Host * mpHost;
};


class MudletObjectWrapper : public QObject {

  Q_OBJECT

public slots:
  MudletObject* new_MudletObject(const QString& hash) { return new MudletObject(hash); }

  void delete_MudletObject(MudletObject* o) { delete o; }

  // Python methods.
  int selectString(MudletObject* o, QString& text, int numMatch, QString& console);
  void send(MudletObject* o, QString& command, bool echo);
  void expandAlias( MudletObject* o, QString& command, bool print );
  void resetFormat( MudletObject* o, QString& console );
  void setBgColor( MudletObject* o, int r,int g,int b, QString& console );
  void setFgColor( MudletObject* o, int r,int g,int b, QString& console );
  bool enableTimer( MudletObject* o, QString& name );
  bool enableKey( MudletObject* o, QString& name );
  bool enableTrigger( MudletObject* o, QString& name );
  bool enableAlias( MudletObject* o, QString& name );
  bool disableTimer( MudletObject* o, QString& name );
  bool disableKey( MudletObject* o, QString& name );
  bool disableTrigger( MudletObject* o, QString& name );
  bool disableAlias( MudletObject* o, QString& name );
  int selectCaptureGroup( MudletObject* o, int groupNumber );
  void replace( MudletObject* o, QString& with, QString& console );
  void replaceAll( MudletObject* o, QString& what, QString& with, QString& console );
  void deleteLine( MudletObject* o, QString& console );
  bool selectSection( MudletObject* o, int from, int length_of_string, QString& console );
  void raiseEvent( MudletObject* o, QVariantList args );
};

#endif  
