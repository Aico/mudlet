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
#include <QColor>
#include <QHash>
#include <QNetworkAccessManager>
#include <QNetworkReply>


class Host;
class TMapLabel;
class TRoom;

class TPythonInterpreter : public QObject
{
Q_OBJECT

public:
    TPythonInterpreter(Host* pH);
    QNetworkAccessManager * mpFileDownloader;
    QMap<QNetworkReply *, QString> downloadMap;
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
    QMap<QString,QVariant> convertQMap(const QMap<int,int> map);
    QMap<QString,QVariant> convertQMap(const QMap<int,QString> map);
    QMap<QString,QVariant> convertQMap(const QMap<int,QColor> map);
    QMap<QString,QVariant> convertQMap(const QMap<QString,int> map);
    QMap<QString,QVariant> mapLabelsToQVariant(const QMap<qint32,QMap<qint32,TMapLabel> > map);
    QMap<QString,QVariant> mapLabelToQVariant(const TMapLabel label);
    QMap<QString,QVariant> convertQMap(const QMap<int,TRoom *> map);
    QMap<QString,QVariant> convertQMap(const QMap<QString,QString> map);
    QList<QVariant> convertQList(const QList<int> list);
    QMap<QString,QVariant> roomToQVariant(const TRoom room);
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
  void reconnect( MudletObject* o );
  void disconnect( MudletObject* o );
  int sendGMCP( MudletObject* o, QString& themsg );
  int sendATCP( MudletObject* o, QString& themsg );
  int sendTelnetChannel102( MudletObject* o, QString& themsg );
  int sendIrc( MudletObject* o, QString& channel, QString& message);
  int echo( MudletObject* o, QString& themsg, QString& console);
  int echoLink( MudletObject* o, QString& themsg, QString& thefunc, QString& tooltip, QString& console, bool customFormat);
  int insertLink( MudletObject* o, QString& themsg, QString& thefunc, QString& tooltip, QString& console);
  int setLink( MudletObject* o, QString& thefunc, QString& tooltip, QString& console);
  int createBuffer(MudletObject* o, QString& name);
  int appendBuffer(MudletObject* o, QString& console);
  int getLineNumber(MudletObject* o);
  int copy(MudletObject* o, QString& console);
  int cut(MudletObject* o);
  int paste(MudletObject* o, QString& console);
  int feedTriggers(MudletObject* o, QString& txt);
  int setBold(MudletObject* o, QString& console, bool active);
  int setItalics(MudletObject* o, QString& console, bool active);
  int setUnderline(MudletObject* o, QString& console, bool active);
  int echoPopup(MudletObject* o, QString& themsg, QStringList& func, QStringList& tooltip, QString& console, bool customFormat);
  int setPopup(MudletObject* o, QStringList& func, QStringList& tip, QString& console);
  int insertPopup(MudletObject* o, QString& themsg, QStringList& func, QStringList& tooltip, QString& console);
  int moveCursor(MudletObject* o, int xpos, int ypos, QString& console);
  int moveCursorEnd(MudletObject* o, QString& console);
  int pasteWindow(MudletObject* o, QString& console);
  int selectCurrentLine(MudletObject* o, QString& console);
  int wrapLine(MudletObject* o, int linenum, QString& console);
  QList<int> getFgColor(MudletObject* o, QString& console);
  QList<int> getBgColor(MudletObject* o, QString& console);
  QString getCurrentLine(MudletObject* o, QString& console);
  int insertHTML(MudletObject* o, QString& themsg );
  int insertText(MudletObject* o, QString& themsg, QString& console );
  bool isAnsiFgColor(MudletObject* o, int ansiFg, QString& console);
  bool isAnsiBgColor(MudletObject* o, int ansiFg, QString& console);
  //Start of new commit
  int appendCmdLine(MudletObject* o, QString& appendtxt);
  int denyCurrentSend(MudletObject*o);
  int getLastLineNumber(MudletObject*o, QString& console);
  int getLineCount(MudletObject*o, QString& console);
  QStringList getLines(MudletObject*o, int From, int To);
  QStringList getTime(MudletObject*o, bool return_string, QString& fmt);
  QString getTimeStamp(MudletObject*o, int line, QString& console);
  bool isPrompt(MudletObject*o);
  int startLogging(MudletObject*o, bool logOn);
  bool isActive(MudletObject*o, QString& obj, QString& type);
  bool killAlias(MudletObject*o, QString& obj);
  bool killTimer(MudletObject*o, QString& obj);
  bool killTrigger(MudletObject*o, QString& obj);
  int exists(MudletObject*o, QString& obj, QString& type);
  int startTempTimer(MudletObject*o, double timeout, QString & function );
  int startPermTimer(MudletObject*o, QString & name, QString & parent, double timeout, QString & function );
  int startPermAlias(MudletObject*o, QString & name, QString & parent, QString & regex, QString & function );
  int startTempAlias( MudletObject*o, QString & regex, QString & function );
  int startTempExactMatchTrigger( MudletObject*o, QString & regex, QString & function );
  int startTempBeginOfLineTrigger( MudletObject*o, QString & regex, QString & function );
  int startTempTrigger( MudletObject*o, QString & regex, QString & function );
  int startTempLineTrigger( MudletObject*o,  int from, int howmany, QString & function );
  int startTempColorTrigger( MudletObject*o, int fg, int bg, QString & function );
  int startTempRegexTrigger( MudletObject*o, QString & regex, QString & function );
  int startPermRegexTrigger( MudletObject*o, QString & name, QString & parent, QStringList & regexList, QString & function );
  int startPermSubstringTrigger( MudletObject*o, QString & name, QString & parent, QStringList & regexList, QString & function );
  int startPermBeginOfLineStringTrigger(  MudletObject*o, QString & name, QString & parent, QStringList & regexList, QString & function );
  int setTriggerStayOpen( MudletObject*o, QString& name, double lines );
  double createStopWatch( MudletObject*o );
  double stopStopWatch( MudletObject*o, int watchID );
  bool resetStopWatch( MudletObject*o, int watchID );
  bool startStopWatch( MudletObject*o, int watchID );
  double getStopWatchTime( MudletObject*o, int watchID );
  QString getMudletHomeDir( MudletObject*o );
  double getNetworkLatency( MudletObject*o );
  int resetProfile( MudletObject*o );
  int connectToServer( MudletObject*o, int port, QString&  url);
  int downloadFile( MudletObject*o, QString& path, QString& url );
  QString invokeFileDialog( MudletObject*o, bool dir,  QString& title);
  int loadRawFile( MudletObject*o, QString& SendTxt );
  int playSoundFile( MudletObject*o, QString& sound );
  int sendSocket( MudletObject*o, QString& txt );
  //Mapper Functions.
  int setCustomEnvColor( MudletObject* o, int id, QColor c);
  int removeCustomEnvColor( MudletObject* o, int id);
  int updateRoom( MudletObject* o, QMap<QString, QVariant> map);
  int deleteRoom( MudletObject* o, int id);
  int setRoomUserData( MudletObject* o, int roomID, QString key, QString value );
  int update2DMapperNow( MudletObject* o);
  int toggleHighlight( MudletObject* o, int roomID, bool highlight);
  int setAreaName( MudletObject* o, int id, QString name );
  int deleteArea( MudletObject* o, int id );
  int updateMapLabel( MudletObject* o, int area, QString text, float x, float y, QColor fg, QColor bg, int id );
  int deleteMapLabel( MudletObject* o, int area, int labelID  );
};

#endif  
