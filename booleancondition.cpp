/***************************************************************************
 * Copyright (C) 2014 by Renaud Guezennec                                   *
 * https://rolisteam.org/contact                      *
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
#include "booleancondition.h"

Dice::CONDITION_STATE testEqual(bool insideRange, const std::pair<qint64, qint64>& range)
{
    if(!insideRange)
        return Dice::CONDITION_STATE::UNREACHABLE;
    else if(insideRange && (range.first == range.second))
        return Dice::CONDITION_STATE::ALWAYSTRUE;
    else
        return Dice::CONDITION_STATE::REACHABLE;
}

Dice::CONDITION_STATE testGreatherThan(qint64 value, const std::pair<qint64, qint64>& range)
{
    if(value >= std::max(range.first, range.second))
        return Dice::CONDITION_STATE::UNREACHABLE;
    else if(value < std::min(range.first, range.second))
        return Dice::CONDITION_STATE::ALWAYSTRUE;
    else
        return Dice::CONDITION_STATE::REACHABLE;
}

Dice::CONDITION_STATE testLesserThan(qint64 value, const std::pair<qint64, qint64>& range)
{
    if(value <= std::min(range.first, range.second))
        return Dice::CONDITION_STATE::UNREACHABLE;
    else if(value > std::max(range.first, range.second))
        return Dice::CONDITION_STATE::ALWAYSTRUE;
    else
        return Dice::CONDITION_STATE::REACHABLE;
}

Dice::CONDITION_STATE testGreaterOrEqual(qint64 value, const std::pair<qint64, qint64>& range)
{
    if(value > std::max(range.first, range.second))
        return Dice::CONDITION_STATE::UNREACHABLE;
    else if(value <= std::min(range.first, range.second))
        return Dice::CONDITION_STATE::ALWAYSTRUE;
    else
        return Dice::CONDITION_STATE::REACHABLE;
}

Dice::CONDITION_STATE testLesserOrEqual(qint64 value, const std::pair<qint64, qint64>& range)
{
    if(value < std::min(range.first, range.second))
        return Dice::CONDITION_STATE::UNREACHABLE;
    else if(value >= std::max(range.first, range.second))
        return Dice::CONDITION_STATE::ALWAYSTRUE;
    else
        return Dice::CONDITION_STATE::REACHABLE;
}

Dice::CONDITION_STATE testDifferent(bool inside, const std::pair<qint64, qint64>& range)
{
    if(inside && (range.first == range.second))
        return Dice::CONDITION_STATE::UNREACHABLE;
    else if(!inside)
        return Dice::CONDITION_STATE::ALWAYSTRUE;
    else
        return Dice::CONDITION_STATE::REACHABLE;
}

BooleanCondition::BooleanCondition() : m_operator(Equal) {}

BooleanCondition::~BooleanCondition()
{
    if(m_value != nullptr)
    {
        delete m_value;
        m_value= nullptr;
    }
}
qint64 BooleanCondition::hasValid(Die* b, bool recursive, bool unhighlight) const
{
    QList<qint64> listValues;
    if(recursive)
    {
        listValues= b->getListValue();
    }
    else
    {
        listValues.append(b->getLastRolledValue());
    }

    qint64 sum= 0;
    auto valueScalar= valueToScalar();
    for(qint64& value : listValues)
    {
        switch(m_operator)
        {
        case Equal:
            sum+= (value == valueScalar) ? 1 : 0;
            break;
        case GreaterThan:
            sum+= (value > valueScalar) ? 1 : 0;
            break;
        case LesserThan:
            sum+= (value < valueScalar) ? 1 : 0;
            break;
        case GreaterOrEqual:
            sum+= (value >= valueScalar) ? 1 : 0;
            break;
        case LesserOrEqual:
            sum+= (value <= valueScalar) ? 1 : 0;
            break;
        case Different:
            sum+= (value != valueScalar) ? 1 : 0;
            break;
        }
    }
    if((unhighlight) && (sum == 0))
    {
        b->setHighlighted(false);
    }
    else
    {
        b->setHighlighted(true);
    }

    return sum;
}

void BooleanCondition::setOperator(LogicOperator m)
{
    m_operator= m;
}

void BooleanCondition::setValueNode(ExecutionNode* v)
{
    m_value= v;
}
QString BooleanCondition::toString()
{
    QString str("");
    switch(m_operator)
    {
    case Equal:
        str.append(QStringLiteral("="));
        break;
    case GreaterThan:
        str.append(QStringLiteral(">"));
        break;
    case LesserThan:
        str.append(QStringLiteral("<"));
        break;
    case GreaterOrEqual:
        str.append(QStringLiteral(">="));
        break;
    case LesserOrEqual:
        str.append(QStringLiteral("<="));
        break;
    case Different:
        str.append(QStringLiteral("!="));
        break;
    }
    return QStringLiteral("[%1%2]").arg(str).arg(valueToScalar());
}

Dice::CONDITION_STATE BooleanCondition::isValidRangeSize(const std::pair<qint64, qint64>& range) const
{
    Dice::CONDITION_STATE state;
    auto valueScalar= valueToScalar();
    qint64 boundValue= qBound(range.first, valueScalar, range.second);
    bool isInsideRange= (boundValue == valueScalar);
    switch(m_operator)
    {
    case Equal:
        state= testEqual(isInsideRange, range); //(isInsideRange && (range.first != range.second)) ? ;
        break;
    case GreaterThan:
        state= testGreatherThan(valueScalar, range);
        break;
    case LesserThan:
        state= testLesserThan(valueScalar, range);
        break;
    case GreaterOrEqual:
        state= testGreaterOrEqual(valueScalar, range);
        break;
    case LesserOrEqual:
        state= testLesserOrEqual(valueScalar, range);
        break;
    case Different:
        state= testDifferent(isInsideRange, range);
        break;
    }
    return state;
}
Validator* BooleanCondition::getCopy() const
{
    BooleanCondition* val= new BooleanCondition();
    val->setOperator(m_operator);
    val->setValueNode(m_value->getCopy());
    return val;
}
qint64 BooleanCondition::valueToScalar() const
{
    if(m_value == nullptr)
        return 0;

    m_value->run(nullptr);
    auto result= m_value->getResult();
    if(result)
        return result->getResult(Dice::RESULT_TYPE::SCALAR).toInt();
    else
        return 0;
}
