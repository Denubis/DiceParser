/***************************************************************************
 * Copyright (C) 2014 by Renaud Guezennec                                   *
 * http://www.rolisteam.org/contact                      *
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
#include <QString>

#include "node/numbernode.h"
#include "node/sortresult.h"
#include "node/stringnode.h"
#include "node/variablenode.h"
#include "parsingtoolbox.h"

QHash<QString, QString> ParsingToolBox::m_variableHash;
std::vector<ExecutionNode*>* ParsingToolBox::m_startNodes= nullptr;

ParsingToolBox::ParsingToolBox()
    : m_logicOp(new QMap<QString, BooleanCondition::LogicOperator>())
    , m_logicOperation(new QMap<QString, CompositeValidator::LogicOperation>())
    , m_conditionOperation(new QMap<QString, OperationCondition::ConditionOperator>())
    , m_arithmeticOperation(new QHash<QString, Die::ArithmeticOperator>())
{
    // m_logicOp = ;
    m_logicOp->insert(">=", BooleanCondition::GreaterOrEqual);
    m_logicOp->insert("<=", BooleanCondition::LesserOrEqual);
    m_logicOp->insert("<", BooleanCondition::LesserThan);
    m_logicOp->insert("=", BooleanCondition::Equal);
    m_logicOp->insert(">", BooleanCondition::GreaterThan);
    m_logicOp->insert("!=", BooleanCondition::Different);

    // m_logicOperation = ;
    m_logicOperation->insert("|", CompositeValidator::OR);
    m_logicOperation->insert("^", CompositeValidator::EXCLUSIVE_OR);
    m_logicOperation->insert("&", CompositeValidator::AND);

    // m_conditionOperation = ;
    m_conditionOperation->insert("%", OperationCondition::Modulo);

    // m_arithmeticOperation = new QHash<QString,ScalarOperatorNode::ArithmeticOperator>();
    m_arithmeticOperation->insert(QStringLiteral("+"), Die::PLUS);
    m_arithmeticOperation->insert(QStringLiteral("-"), Die::MINUS);
    m_arithmeticOperation->insert(QStringLiteral("*"), Die::MULTIPLICATION);
    m_arithmeticOperation->insert(QStringLiteral("x"), Die::MULTIPLICATION);
    m_arithmeticOperation->insert(QStringLiteral("|"), Die::INTEGER_DIVIDE);
    m_arithmeticOperation->insert(QStringLiteral("/"), Die::DIVIDE);
    m_arithmeticOperation->insert(QStringLiteral("÷"), Die::DIVIDE);
    m_arithmeticOperation->insert(QStringLiteral("**"), Die::POW);
}

ParsingToolBox::ParsingToolBox(const ParsingToolBox&) {}
ParsingToolBox::~ParsingToolBox()
{
    if(nullptr != m_logicOp)
    {
        delete m_logicOp;
        m_logicOp= nullptr;
    }
    if(nullptr != m_logicOperation)
    {
        delete m_logicOperation;
        m_logicOperation= nullptr;
    }
    if(nullptr != m_conditionOperation)
    {
        delete m_conditionOperation;
        m_conditionOperation= nullptr;
    }
    if(nullptr != m_arithmeticOperation)
    {
        delete m_arithmeticOperation;
        m_arithmeticOperation= nullptr;
    }
}
ExecutionNode* ParsingToolBox::addSort(ExecutionNode* e, bool b)
{
    SortResultNode* nodeSort= new SortResultNode();
    nodeSort->setSortAscending(b);
    e->setNextNode(nodeSort);
    return nodeSort;
}

bool ParsingToolBox::readDiceLogicOperator(QString& str, OperationCondition::ConditionOperator& op)
{
    QString longKey;
    auto const& keys= m_conditionOperation->keys();
    for(const QString& tmp : keys)
    {
        if(str.startsWith(tmp))
        {
            if(longKey.size() < tmp.size())
            {
                longKey= tmp;
            }
        }
    }
    if(longKey.size() > 0)
    {
        str= str.remove(0, longKey.size());
        op= m_conditionOperation->value(longKey);
        return true;
    }

    return false;
}

bool ParsingToolBox::readArithmeticOperator(QString& str, Die::ArithmeticOperator& op)
{
    bool found= false;
    // QHash<QString,ScalarOperatorNode::ArithmeticOperator>::Iterator

    for(auto i= m_arithmeticOperation->begin(); i != m_arithmeticOperation->end() && !found; ++i)
    {
        if(str.startsWith(i.key()))
        {
            op= i.value();
            str= str.remove(0, i.key().size());
            found= true;
        }
    }
    return found;
}

bool ParsingToolBox::readLogicOperator(QString& str, BooleanCondition::LogicOperator& op)
{
    QString longKey;
    auto const& keys= m_logicOp->keys();
    for(const QString& tmp : keys)
    {
        if(str.startsWith(tmp))
        {
            if(longKey.size() < tmp.size())
            {
                longKey= tmp;
            }
        }
    }
    if(longKey.size() > 0)
    {
        str= str.remove(0, longKey.size());
        op= m_logicOp->value(longKey);
        return true;
    }

    return false;
}

bool ParsingToolBox::readOperand(QString& str, ExecutionNode*& node)
{
    qint64 intValue= 1;
    QString resultStr;
    if(readDynamicVariable(str, intValue))
    {
        VariableNode* variableNode= new VariableNode();
        variableNode->setIndex(static_cast<quint64>(intValue - 1));
        variableNode->setData(m_startNodes);
        node= variableNode;
        return true;
    }
    else if(readNumber(str, intValue))
    {
        NumberNode* numberNode= new NumberNode();
        numberNode->setNumber(intValue);
        node= numberNode;
        return true;
    }
    else if(readString(str, resultStr))
    {
        StringNode* strNode= new StringNode();
        strNode->setString(resultStr);
        node= strNode;
        return true;
    }
    return false;
}

Validator* ParsingToolBox::readValidator(QString& str, bool hasSquare)
{
    Validator* returnVal= nullptr;
    BooleanCondition::LogicOperator myLogicOp= BooleanCondition::Equal;
    readLogicOperator(str, myLogicOp);

    OperationCondition::ConditionOperator condiOp= OperationCondition::Modulo;
    bool hasDiceLogicOperator= readDiceLogicOperator(str, condiOp);
    ExecutionNode* operandNode= nullptr;
    if(hasDiceLogicOperator)
    {
        if(readOperand(str, operandNode))
        {
            OperationCondition* condition= new OperationCondition();
            condition->setValueNode(operandNode);
            Validator* valid= readValidator(str, hasSquare);
            BooleanCondition* boolC= dynamic_cast<BooleanCondition*>(valid);
            if(nullptr != boolC)
            {
                condition->setBoolean(boolC);
                returnVal= condition;
            }
            else
            {
                delete condition;
            }
        }
    }
    else if(readOperand(str, operandNode))
    {
        bool isRange= false;
        if(str.startsWith("..") && hasSquare)
        {
            str= str.remove(0, 2);
            qint64 end= 0;
            if(readNumber(str, end))
            {
                str= str.remove(0, 1);
                qint64 start= operandNode->getScalarResult();
                Range* range= new Range();
                range->setValue(start, end);
                returnVal= range;
                isRange= true;
            }
        }

        if(!isRange)
        {
            BooleanCondition* condition= new BooleanCondition();
            condition->setValueNode(operandNode);
            condition->setOperator(myLogicOp);
            returnVal= condition;
        }
    }
    return returnVal;
}
IfNode::ConditionType ParsingToolBox::readConditionType(QString& str)
{
    IfNode::ConditionType type= IfNode::OnEach;
    if(str.startsWith('.'))
    {
        str= str.remove(0, 1);
        type= IfNode::OneOfThem;
    }
    else if(str.startsWith('*'))
    {
        str= str.remove(0, 1);
        type= IfNode::AllOfThem;
    }
    else if(str.startsWith(':'))
    {
        str= str.remove(0, 1);
        type= IfNode::OnScalar;
    }
    return type;
}

Validator* ParsingToolBox::readCompositeValidator(QString& str)
{
    bool expectSquareBrasket= false;
    if((str.startsWith("[")))
    {
        str= str.remove(0, 1);
        expectSquareBrasket= true;
    }
    Validator* tmp= readValidator(str, expectSquareBrasket);
    CompositeValidator::LogicOperation opLogic;

    QVector<CompositeValidator::LogicOperation> operators;
    QList<Validator*> validatorList;

    while(nullptr != tmp)
    {
        bool hasOperator= readLogicOperation(str, opLogic);
        if(hasOperator)
        {
            operators.append(opLogic);
            validatorList.append(tmp);
            tmp= readValidator(str, expectSquareBrasket);
        }
        else
        {
            if((expectSquareBrasket) && (str.startsWith("]")))
            {
                str= str.remove(0, 1);
                // isOk=true;
            }

            if(!validatorList.isEmpty())
            {
                validatorList.append(tmp);
            }
            else
            {
                operators.clear();
                return tmp;
            }
            tmp= nullptr;
        }
    }
    if(!validatorList.isEmpty())
    {
        CompositeValidator* validator= new CompositeValidator();
        validator->setOperationList(operators);
        validator->setValidatorList(validatorList);
        return validator;
    }
    else
    {
        return nullptr;
    }
}
bool ParsingToolBox::readLogicOperation(QString& str, CompositeValidator::LogicOperation& op)
{
    QString longKey;
    auto const& keys= m_logicOperation->keys();
    for(auto& tmp : keys)
    {
        if(str.startsWith(tmp))
        {
            if(longKey.size() < tmp.size())
            {
                longKey= tmp;
            }
        }
    }
    if(longKey.size() > 0)
    {
        str= str.remove(0, longKey.size());
        op= m_logicOperation->value(longKey);
        return true;
    }

    return false;
}

bool ParsingToolBox::readNumber(QString& str, qint64& myNumber)
{
    if(str.isEmpty())
        return false;

    QString number;
    int i= 0;
    while(i < str.length() && ((str[i].isNumber()) || ((i == 0) && (str[i] == '-'))))
    {
        number+= str[i];
        ++i;
    }

    if(number.isEmpty())
    {
        QString reason;
        return readVariable(str, myNumber, reason);
    }

    bool ok;
    myNumber= number.toLongLong(&ok);
    if(ok)
    {
        str= str.remove(0, number.size());
        return true;
    }

    return false;
}
bool ParsingToolBox::readDynamicVariable(QString& str, qint64& index)
{
    if(str.isEmpty())
        return false;
    if(str.startsWith('$'))
    {
        QString number;
        int i= 1;
        while(i < str.length() && (str[i].isNumber()))
        {
            number+= str[i];
            ++i;
        }

        bool ok;
        index= number.toLongLong(&ok);
        if(ok)
        {
            str= str.remove(0, number.size() + 1);
            return true;
        }
    }
    return false;
}

ExecutionNode* ParsingToolBox::getLatestNode(ExecutionNode* node)
{
    if(nullptr == node)
        return nullptr;

    ExecutionNode* next= node;
    while(nullptr != next->getNextNode())
    {
        next= next->getNextNode();
    }
    return next;
}

std::vector<ExecutionNode*>* ParsingToolBox::getStartNodes()
{
    return m_startNodes;
}

void ParsingToolBox::setStartNodes(std::vector<ExecutionNode*>* startNodes)
{
    m_startNodes= startNodes;
}

bool ParsingToolBox::readString(QString& str, QString& strResult)
{
    if(str.isEmpty())
        return false;

    if(str.startsWith('"'))
    {
        str= str.remove(0, 1);

        int i= 0;
        int j= 0;
        bool previousEscape= false;
        QString result;
        /*&&
                (((!previousEscape) && !(str[i]=='"')) || (previousEscape) && !(str[i]=='"'))
                    || (str[i]=='\\'))*/
        while(i < str.length() && (!(!previousEscape && (str[i] == '"')) || (previousEscape && str[i] != '"')))
        {
            if(str[i] == '\\')
            {
                previousEscape= true;
            }
            else
            {
                if(previousEscape && str[i] != '\"')
                {
                    result+= '\\';
                    ++j;
                }
                result+= str[i];
                previousEscape= false;
            }
            ++i;
        }

        if(!result.isEmpty())
        {
            str= str.remove(0, i);
            strResult= result;
            if(str.startsWith('"'))
            {
                str= str.remove(0, 1);
                return true;
            }
        }
    }

    return false;
}

bool ParsingToolBox::readVariable(QString& str, qint64& myNumber, QString& reasonFail)
{
    if(str.isEmpty())
        return false;

    if(str.startsWith("${"))
    {
        str= str.remove(0, 2);
    }
    QString key;
    int post= str.indexOf('}');
    key= str.left(post);

    if(!m_variableHash.isEmpty())
    {
        if(m_variableHash.contains(key))
        {
            QString value= m_variableHash.value(key);
            bool ok;
            int valueInt= value.toInt(&ok);
            if(ok)
            {
                myNumber= valueInt;
                str= str.remove(0, post + 1);
                return true;
            }
            else
            {
                reasonFail= QStringLiteral("Variable value is %1, not a number").arg(value);
            }
        }
        else
        {
            reasonFail= QStringLiteral("Variable not found");
        }
    }
    else
    {
        reasonFail= QStringLiteral("No Variables are defined");
    }

    return false;
}
bool ParsingToolBox::readComma(QString& str)
{
    if(str.startsWith(","))
    {
        str= str.remove(0, 1);
        return true;
    }
    else
        return false;
}
bool ParsingToolBox::readOpenParentheses(QString& str)
{
    if(str.startsWith("("))
    {
        str= str.remove(0, 1);
        return true;
    }
    else
        return false;
}

bool ParsingToolBox::readCloseParentheses(QString& str)
{
    if(str.startsWith(")"))
    {
        str= str.remove(0, 1);
        return true;
    }
    else
        return false;
}

int ParsingToolBox::findClosingCharacterIndexOf(QChar open, QChar closing, const QString& str, int offset)
{
    int counter= offset;
    int i= 0;
    for(auto const& letter : str)
    {
        if(letter == open)
            ++counter;
        else if(letter == closing)
            --counter;

        if(counter == 0)
            return i;

        ++i;
    }
    return -1;
}

bool ParsingToolBox::readList(QString& str, QStringList& list, QList<Range>& ranges)
{
    if(str.startsWith("["))
    {
        str= str.remove(0, 1);
        int pos= findClosingCharacterIndexOf('[', ']', str, 1); // str.indexOf("]");
        if(-1 != pos)
        {
            QString liststr= str.left(pos);
            list= liststr.split(",");
            str= str.remove(0, pos + 1);
            readProbability(list, ranges);
            return true;
        }
    }
    return false;
}
bool ParsingToolBox::readAscending(QString& str)
{
    if(str.isEmpty())
    {
        return false;
    }
    else if(str.at(0) == 'l')
    {
        str= str.remove(0, 1);
        return true;
    }
    return false;
}
Dice::CONDITION_STATE ParsingToolBox::isValidValidator(ExecutionNode* previous, Validator* val)
{
    DiceRollerNode* node= getDiceRollerNode(previous);
    if(nullptr == node)
        return Dice::CONDITION_STATE::ERROR_STATE;

    return val->isValidRangeSize(node->getRange());
}
DiceRollerNode* ParsingToolBox::getDiceRollerNode(ExecutionNode* previous)
{
    while(nullptr != previous)
    {
        DiceRollerNode* node= dynamic_cast<DiceRollerNode*>(previous);
        if(nullptr != node)
        {
            return node;
        }
        previous= previous->getPreviousNode();
    }
    return nullptr;
}
bool ParsingToolBox::readDiceRange(QString& str, qint64& start, qint64& end)
{
    bool expectSquareBrasket= false;

    if((str.startsWith("[")))
    {
        str= str.remove(0, 1);
        expectSquareBrasket= true;
    }
    if(readNumber(str, start))
    {
        if(str.startsWith("-"))
        {
            str= str.remove(0, 1);
            if(readNumber(str, end))
            {
                if(expectSquareBrasket)
                {
                    if(str.startsWith("]"))
                    {
                        str= str.remove(0, 1);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}
ParsingToolBox::LIST_OPERATOR ParsingToolBox::readListOperator(QString& str)
{
    if(str.startsWith('u'))
    {
        str= str.remove(0, 1);
        return UNIQUE;
    }
    return NONE;
}

bool ParsingToolBox::readPainterParameter(PainterNode* painter, QString& str)
{
    if(!str.startsWith('['))
        return false;

    str= str.remove(0, 1);
    int pos= str.indexOf(']');

    if(pos == -1)
        return false;

    QString data= str.left(pos);
    str= str.remove(0, pos + 1);
    QStringList duos= data.split(',');
    bool result= false;
    for(QString& duoStr : duos)
    {
        QStringList keyValu= duoStr.split(':');
        if(keyValu.size() == 2)
        {
            painter->insertColorItem(keyValu[1], keyValu[0].toInt());
            result= true;
        }
    }

    return result;
}

QHash<QString, QString> ParsingToolBox::getVariableHash()
{
    return m_variableHash;
}

void ParsingToolBox::setVariableHash(const QHash<QString, QString>& variableHash)
{
    m_variableHash= variableHash;
}

void ParsingToolBox::readProbability(QStringList& str, QList<Range>& ranges)
{
    quint64 totalDistance= 0;
    quint64 undefDistance= 0;
    int undefCount= 0;
    int maxValue= 0;
    int i= 0;
    int j= 0;
    bool hasPercentage= false;
    for(QString line : str)
    {
        int pos= line.indexOf('[');
        if(-1 != pos)
        {
            QString rangeStr= line.right(line.length() - pos);
            line= line.left(pos);
            str[j]= line;
            qint64 start= 0;
            qint64 end= 0;
            if(readDiceRange(rangeStr, start, end))
            {
                Range range;
                range.setValue(start, end);
                ranges.append(range);
                totalDistance+= static_cast<quint64>(end - start + 1);
                ++i;
            }
            else // percentage
            {
                hasPercentage= true;
                Range range;
                range.setStart(start);
                ranges.append(range);
                ++undefCount;
                undefDistance+= static_cast<quint64>(start);
            }
            if((end > maxValue) || (i == 1))
            {
                maxValue= static_cast<int>(end);
            }
        }
        else
        {
            Range range;
            range.setEmptyRange(true);
            ranges.append(range);
        }
        ++j;
    }

    if((hasPercentage) && (undefDistance != 0))
    {
        qreal ratio= 100.0 / static_cast<qreal>(undefDistance);
        qint64 realStart= 0;
        for(int i= 0; i < ranges.size(); ++i)
        {
            Range tmp= ranges.at(i);
            if(!tmp.isFullyDefined())
            {
                int dist= static_cast<int>(tmp.getStart());
                tmp.setStart(realStart + 1);
                double truc= dist * ratio;

                tmp.setEnd(static_cast<int>(realStart + truc));
                realStart= tmp.getEnd();
                ranges[i]= tmp;
            }
        }
    }
    else
    {
        int limitUp= 1;
        for(int i= 0; i < ranges.size(); ++i)
        {
            Range range= ranges.at(i);
            if(range.isEmptyRange())
            {
                range.setStart(limitUp);
                range.setEnd(limitUp);
                range.setEmptyRange(false);
            }
            else
            {
                qint64 sizeRange= range.getEnd() - range.getStart();
                range.setStart(limitUp);
                limitUp+= sizeRange;
                range.setEnd(limitUp);
            }
            ++limitUp;
            ranges[i]= range;
        }
    }
}
bool ParsingToolBox::readComment(QString& str, QString& result, QString& comment)
{
    QString left= str;
    str= str.trimmed();
    if(str.startsWith("#"))
    {
        comment= left;
        str= str.remove(0, 1);
        result= str.trimmed();
        str= "";
        return true;
    }
    return false;
}

QString ParsingToolBox::replaceVariableToValue(const QString& source, QStringList values)
{
    QString result= source;

    int start= source.size() - 1;
    bool valid= true;
    do
    {
        auto ref= readVariableFromString(source, start);
        if(ref.isValid())
        {
            result.remove(ref.position(), ref.length());
            auto val= values[ref.resultIndex() - 1];
            if(ref.digitNumber() != 0)
            {
                auto realVal= QString("%1").arg(val, ref.digitNumber(), QChar('0'));
                result.insert(ref.position(), realVal);
            }
            else
            {
                result.insert(ref.position(), val);
            }
        }
        else
        {
            valid= false;
        }
    } while(valid);

    return result;
}

QString ParsingToolBox::replacePlaceHolderToValue(const QString& source, const QList<ExportedDiceResult>& list)
{
    QStringList resultList;
    std::transform(
        std::begin(list), std::end(list), std::back_inserter(resultList), [](const ExportedDiceResult& dice) {
            QStringList valuesStr;
            if(dice.size() == 1)
            {
                auto values= dice.values();
                std::transform(std::begin(values), std::end(values), std::back_inserter(valuesStr),
                               [](const ListDiceResult& dice) {
                                   QStringList textList;
                                   std::transform(std::begin(dice), std::end(dice), std::back_inserter(textList),
                                                  [](const HighLightDice& dice) { return dice.getResultString(); });
                                   return textList.join(",");
                               });
            }
            else if(dice.size() > 1)
            {
                for(auto key : dice.keys())
                {
                    auto values= dice[key];
                    QStringList textVals;
                    std::transform(std::begin(values), std::end(values), std::back_inserter(textVals),
                                   [](const HighLightDice& dice) { return dice.getResultString(); });
                    valuesStr.append(QString("d%1 [%2]").arg(key).arg(textVals.join(",")));
                }
            }
            return valuesStr.join(",");
        });

    QString result= source;
    int start= source.size() - 1;
    bool valid= true;
    do
    {
        auto ref= readPlaceHolderFromString(source, start);
        if(ref.isValid())
        {
            result.remove(ref.position(), ref.length());
            auto val= resultList[ref.resultIndex() - 1];
            result.insert(ref.position(), val);
            /*            if(ref.digitNumber() != 0)
                        {
                            auto realVal= QString("%1").arg(val, ref.digitNumber(), QChar('0'));
                            result.insert(ref.position(), realVal);
                        }
                        else
                        {
                        }*/
        }
        else
        {
            valid= false;
        }
    } while(valid);

    return result;

    // return source;
}

void ParsingToolBox::readSubtitutionParameters(SubtituteInfo& info, QString& rest)
{
    auto sizeS= rest.size();
    if(rest.startsWith("{"))
    {
        rest= rest.remove(0, 1);
        qint64 number;
        if(readNumber(rest, number))
        {
            if(rest.startsWith("}"))
            {
                rest= rest.remove(0, 1);
                info.setDigitNumber(static_cast<int>(number));
            }
        }
    }
    info.setLength(info.length() + sizeS - rest.size());
}

SubtituteInfo ParsingToolBox::readVariableFromString(const QString& source, int& start)
{
    bool found= false;
    SubtituteInfo info;
    int i= start;
    for(; i >= 0 && !found; --i)
    {
        if(source.at(i) == '$')
        {
            auto rest= source.mid(i + 1, 1 + start - i);
            qint64 number;
            if(readNumber(rest, number))
            {
                auto len= QString::number(number).size() - 1;
                readSubtitutionParameters(info, rest);
                info.setLength(info.length() + len);
                info.setResultIndex(static_cast<int>(number));
                info.setPosition(i);
                found= true;
            }
        }
    }
    start= i;
    return info;
}

SubtituteInfo ParsingToolBox::readPlaceHolderFromString(const QString& source, int& start)
{
    bool found= false;
    SubtituteInfo info;
    int i= start;
    for(; i >= 0 && !found; --i)
    {
        if(source.at(i) == '@')
        {
            auto rest= source.mid(i + 1, 1 + start - i);
            qint64 number;
            if(readNumber(rest, number))
            {
                auto len= QString::number(number).size() - 1;
                readSubtitutionParameters(info, rest);
                info.setLength(info.length() + len);
                info.setResultIndex(static_cast<int>(number));
                info.setPosition(i);
                found= true;
            }
        }
    }
    start= i;
    return info;
}

SubtituteInfo::SubtituteInfo() {}

bool SubtituteInfo::isValid() const
{
    return !(m_position + m_resultIndex < 0);
}

int SubtituteInfo::length() const
{
    return m_length;
}

void SubtituteInfo::setLength(int length)
{
    m_length= length;
}

int SubtituteInfo::resultIndex() const
{
    return m_resultIndex;
}

void SubtituteInfo::setResultIndex(int valueIndex)
{
    m_resultIndex= valueIndex;
}

int SubtituteInfo::position() const
{
    return m_position;
}

void SubtituteInfo::setPosition(int position)
{
    m_position= position;
}

int SubtituteInfo::digitNumber() const
{
    return m_digitNumber;
}

void SubtituteInfo::setDigitNumber(int digitNumber)
{
    m_digitNumber= digitNumber;
}
