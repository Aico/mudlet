
/***************************************************************************
 *   Copyright (C) 2008-2009 by Heiko Koehn   *
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


#include "TLabel.h"
#include <QDebug>

TLabel::TLabel( QWidget * pW )
: QLabel( pW )
, mpHost( 0 )
{
    setMouseTracking( true );
}

QString nothing = "";

void TLabel::mousePressEvent( QMouseEvent * event )
{
    if( event->button() == Qt::LeftButton )
    {
        if( mpHost )
        {
            if(mScriptLanguage == "LUA")
            {
                mpHost->getLuaInterpreter()->callEventHandler( mScript, mpParameters );
            }
            if(mScriptLanguage == "PYTHON")
            {
                mpHost->getPythonInterpreter()->callEventHandler( mScript, mpParameters );
            }
        }
        event->accept();
        return;
    }

    QWidget::mousePressEvent( event );
}
