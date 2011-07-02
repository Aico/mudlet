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

class Host;

class TPythonInterpreter {

public:
    TPythonInterpreter(Host* pH);
    void add_python_variable( const QString & varName, const QVariant & var);
    void callEventHandler( const QString & function, const TEvent * pE );
    void executeScript( const QString & code );
    void call(const QString &callable);
    void callMulti( QString & function);

private:
    PythonQtObjectPtr mainModule;
    Host * mpHost;
};
#endif  
