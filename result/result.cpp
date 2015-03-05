/***************************************************************************
* Copyright (C) 2014 by Renaud Guezennec                                   *
* http://renaudguezennec.homelinux.org/accueil,3.html                      *
*                                                                          *
*  This file is part of DiceParser                                         *
*                                                                          *
* DiceParser is free software; you can redistribute it and/or modify       *
* it under the terms of the GNU General Public License as published by     *
* the Free Software Foundation; either version 2 of the License, or        *
* (at your option) any later version.                                      *
*                                                                          *
* This program is distributed in the hope that it will be useful,          *
* but WITHOUT ANY WARRANTY; without even the implied warranty of           *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the             *
* GNU General Public License for more details.                             *
*                                                                          *
* You should have received a copy of the GNU General Public License        *
* along with this program; if not, write to the                            *
* Free Software Foundation, Inc.,                                          *
* 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.                 *
***************************************************************************/
#include "result.h"

Result::Result()
    : m_previous(NULL)
{
}

Result* Result::getPrevious()
{
    return m_previous;
}

void Result::setPrevious(Result* p)
{
    m_previous = p;
}

bool Result::isStringResult()
{
    return false;
}

bool Result::hasResultOfType(RESULT_TYPE type) const
{
    return (m_resultTypes & type);
}
void Result::generateDotTree(QString& s)
{
    s.append(toString());

}
QString Result::toString()
{
    QString();
}