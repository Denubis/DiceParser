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

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QStringList>
#include <QTextStream>

#ifdef PAINTER_OP
#include <QGuiApplication>
#else
#include <QCoreApplication>
#endif

#include "dicealias.h"
#include "diceparser.h"
#include "displaytoolbox.h"
#include "highlightdice.h"
#include "parsingtoolbox.h"

/**
 * @page Dice
 * The cli for DiceParser the new dice system from rolisteam.
 * @section Build and install
 * To build this program, type these command:
 * - mkdir build
 * - cd build
 * - cmake ../
 * - make
 * - make install
 * @return
 */

QTextStream out(stdout, QIODevice::WriteOnly);
QTextStream err(stderr, QIODevice::WriteOnly);
bool markdown= false;
#ifdef PAINTER_OP
enum EXPORTFORMAT
{
    TERMINAL,
    SVG,
    IMAGE,
    MARKDOWN,
    JSON,
    BOT,
    TEXT
};
#else
enum EXPORTFORMAT
{
    TERMINAL,
    SVG,
    MARKDOWN,
    JSON,
    BOT,
    TEXT
};
#endif
int returnValue= 0;

QString diceToMarkdown(QJsonArray array, bool withColor, bool allSameColor, bool allSameFaceCount)
{
    if(allSameFaceCount)
    {
        QStringList result;
        for(auto item : array)
        {
            auto obj= item.toObject();
            auto values= obj["values"].toArray();
            for(auto val : values)
            {
                result.append(val.toString());
            }
        }
        return result.join(',');
    }
    else
    {
        QStringList result;
        for(auto item : array)
        {
            QStringList subResult;
            auto obj= item.toObject();
            auto values= obj["values"].toArray();
            for(auto val : values)
            {
                subResult.append(val.toString());
            }
            result.append(QStringLiteral("d%1:(").arg(obj["face"].toString()));
            result.append(subResult.join(','));
            result.append(QStringLiteral(")"));
        }
        return result.join(' ');
    }
}

void displayJSon(QString scalarText, QString resultStr, QJsonArray array, bool withColor, QString cmd, QString error,
                 QString warning, QString comment, bool allSameFaceCount, bool allSameColor)
{
    Q_UNUSED(withColor);
    QJsonDocument doc;
    QJsonObject obj;
    obj["values"]= array;
    obj["comment"]= comment;
    obj["error"]= error;
    obj["scalar"]= scalarText;
    obj["string"]= resultStr;
    obj["allSameFace"]= allSameFaceCount;
    obj["allSameColor"]= allSameColor;
    obj["warning"]= warning;
    obj["command"]= cmd;
    doc.setObject(obj);
    out << doc.toJson() << "\n";
}
void displayMarkdown(QString scalarText, QString resultStr, QJsonArray array, bool withColor, QString cmd,
                     QString error, QString warning, QString comment, bool allSameFaceCount, bool allSameColor)
{
    Q_UNUSED(withColor);
    QString str("```Markdown\n");
    if(!error.isEmpty())
    {
        str.append(QStringLiteral("Error: %1\n").arg(error));
    }
    else
    {
        if(!warning.isEmpty())
            str.append(QStringLiteral("Warning: %1\n").arg(warning));

        if(!comment.isEmpty())
        {
            str.prepend(QStringLiteral("%1\n").arg(comment));
        }
        auto diceList= DisplayToolBox::diceToText(array, false, allSameFaceCount, allSameColor);
        if(resultStr.isEmpty())
        {
            str.append(QStringLiteral("# %1\nDetails:[%3 (%2)]\n").arg(scalarText).arg(diceList).arg(cmd));
        }
        else if(!resultStr.isEmpty())
        {
            resultStr.replace("%2", diceList.trimmed());
            str.append(QStringLiteral("%1\n").arg(resultStr));
        }
    }
    str.append(QStringLiteral("```"));
    out << str;
}
QString displaySVG(QString scalarText, QString resultStr, QJsonArray array, bool withColor, QString cmd, QString error,
                   QString warning, QString comment, bool allSameFaceCount, bool allSameColor)
{
    QString str(
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<svg version=\"1.1\"  xmlns=\"http://www.w3.org/2000/svg\" "
        "xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n");
    if(!error.isEmpty())
    {
        str.append(
            QStringLiteral("<text font-size=\"16\" x=\"0\" y=\"20\"><tspan fill=\"red\">%1</tspan></text>").arg(error));
    }
    else
    {
        if(!warning.isEmpty())
            str.append(
                QStringLiteral("<text font-size=\"16\" x=\"0\" y=\"20\"><tspan fill=\"orange\">%1</tspan></text>")
                    .arg(warning));

        int y= 20;
        if(!comment.isEmpty())
        {
            str.append(QStringLiteral("<text font-size=\"16\" x=\"0\" y=\"%2\"><tspan fill=\"blue\">%1</tspan></text>")
                           .arg(comment)
                           .arg(y));
            y+= 20;
        }
        auto diceList= DisplayToolBox::diceToSvg(array, withColor, allSameColor, allSameFaceCount);
        if(resultStr.isEmpty())
        {
            if(withColor)
                str.append(QStringLiteral("<text font-size=\"16\" x=\"0\" y=\"%4\"><tspan fill=\"red\">%1</tspan>\n"
                                          "<tspan x=\"0\" y=\"%5\">details:</tspan>[%3 (%2)]</text>")
                               .arg(scalarText)
                               .arg(diceList)
                               .arg(cmd)
                               .arg(y)
                               .arg(y * 2));
            else
                str.append(QStringLiteral("<text font-size=\"16\" x=\"0\" y=\"%4\"><tspan>%1</tspan>\n"
                                          "<tspan x=\"0\" y=\"%5\">details:</tspan>[%3 (%2)]</text>")
                               .arg(scalarText)
                               .arg(diceList)
                               .arg(cmd)
                               .arg(y)
                               .arg(y * 2));
        }
        else if(!resultStr.isEmpty())
        {
            resultStr.replace("%2", diceList.trimmed());
            str.append(QStringLiteral("<text font-size=\"16\" x=\"0\" y=\"%2\">%1</text>").arg(resultStr).arg(y));
        }
    }
    str.append(QStringLiteral("</svg>\n"));
    return str;
}

#ifdef PAINTER_OP
void displayImage(QString scalarText, QString resultStr, QJsonArray array, bool withColor, QString cmd, QString error,
                  QString warning, QString comment, bool allSameFaceCount, bool allSameColor)
{
    auto svg= displaySVG(scalarText, resultStr, array, withColor, cmd, error, warning, comment, allSameFaceCount,
                         allSameColor);
    out << DisplayToolBox::makeImage(svg.toUtf8());
}
#endif

void displayCommandResult(QString scalarText, QString resultStr, QJsonArray array, bool withColor, QString cmd,
                          QString error, QString warning, QString comment, bool allSameFaceCount, bool allSameColor)
{
    // TODO display warning
    if(!error.isEmpty())
    {
        err << "Error" << error << "\n";
        return;
    }

    if(!warning.isEmpty())
        err << "Warning: " << warning << "\n";

    QString str;

    auto diceList= DisplayToolBox::diceToText(array, withColor, allSameFaceCount, allSameColor);

    if(withColor)
        str= QString("Result: \e[0;31m%1\e[0m - details:[%3 (%2)]").arg(scalarText).arg(diceList).arg(cmd);
    else
        str= QString("Result: %1 - details:[%3 (%2)]").arg(scalarText).arg(diceList).arg(cmd);

    if(!resultStr.isEmpty())
    {
        resultStr.replace("%2", diceList.trimmed());
        str= resultStr;
    }

    if(!comment.isEmpty())
    {
        out << "\033[1m" << comment << "\033[0m\n";
    }
    out << str << "\n";
}

int startDiceParsing(QStringList& cmds, QString& treeFile, bool withColor, EXPORTFORMAT format, QJsonArray array)
{
    DiceParser parser;
    parser.insertAlias(new DiceAlias("L5R5R", QStringLiteral("L[-,⨀,⨀⬢,❂⬢,❁,❁⬢]")), 0);
    parser.insertAlias(new DiceAlias("L5R5S", QStringLiteral("L[-,-,⨀,⨀,⨀❁,⨀⬢,⨀⬢,❂,❂⬢,❁,❁,❁]")), 1);
    int i= 2;
    for(auto alias : array)
    {
        auto objAlias= alias.toObject();
        auto dice
            = new DiceAlias(objAlias["pattern"].toString(), objAlias["cmd"].toString(), !objAlias["regexp"].toBool());
        dice->setComment(objAlias["comment"].toString());
        parser.insertAlias(dice, i++);
    }

    int rt= 0;
    bool in_markdown= true;
    for(QString cmd : cmds)
    {
        if(cmd.startsWith('&') && format == BOT)
        {
            cmd= cmd.remove(0, 1);
            in_markdown= false;
        }

        if(parser.parseLine(cmd))
        {
            parser.start();
            QList<ExportedDiceResult> list;
            QList<ExportedDiceResult> listFull;
            bool homogeneous= true;
            parser.getLastDiceResult(list, homogeneous);
            parser.getDiceResultFromAllInstruction(listFull);
            bool allSameFaceCount= true;
            bool allSameColor= true;
            auto array= DisplayToolBox::diceToJson(list, allSameFaceCount, allSameColor);
            QString resultStr;
            QString scalarText;
            QString lastScalarText;
            QString comment= parser.getComment();
            QString error= parser.humanReadableError();
            QString warnings= parser.humanReadableWarning();
            QStringList strLst;
            QStringList listOfDiceResult;
            QString cmdRework= parser.getDiceCommand();

            if(parser.hasIntegerResultNotInFirst())
            {
                auto values= parser.getLastIntegerResults();
                for(auto val : values)
                {
                    strLst << QString::number(val);
                }
                scalarText= QString("%1").arg(strLst.join(','));
                lastScalarText= strLst.last();
            }
            else if(!list.isEmpty())
            {
                auto values= parser.getSumOfDiceResult();
                for(auto val : values)
                {
                    strLst << QString::number(val);
                }
                scalarText= QString("%1").arg(strLst.join(','));
            }

            for(auto map : list)
            {
                for(auto key : map.keys())
                {
                    auto dice= map[key];
                    QString stringVal;
                    for(auto val : dice)
                    {
                        qint64 total= 0;
                        QStringList dicelist;
                        for(auto score : val.getResult())
                        {
                            total+= score;
                            dicelist << QString::number(score);
                        }
                        if(val.getResult().size() > 1)
                        {
                            stringVal= QString("%1 [%2]").arg(total).arg(dicelist.join(','));
                            listOfDiceResult << stringVal;
                        }
                        else
                        {
                            listOfDiceResult << QString::number(total);
                        }
                    }
                }
            }

            if(parser.hasStringResult())
            {
                bool ok;
                QStringList allStringlist= parser.allFirstResultAsString(ok);

                QStringList resultWithPlaceHolder;
                std::for_each(allStringlist.begin(), allStringlist.end(), [&resultWithPlaceHolder](const QString& sub) {
                    QRegularExpression ex("%[1-3]?|\\$[1-9]+|@[1-9]+");
                    if(sub.contains(ex))
                        resultWithPlaceHolder.append(sub);
                });
                auto stringResult
                    = resultWithPlaceHolder.isEmpty() ? allStringlist.join(" ; ") : resultWithPlaceHolder.join(" ; ");

                stringResult.replace("%1", scalarText);
                stringResult.replace("%2", listOfDiceResult.join(",").trimmed());
                stringResult.replace("%3", lastScalarText);
                stringResult.replace("\\n", "\n");

                QMap<Dice::ERROR_CODE, QString> errorMap;
                stringResult= ParsingToolBox::replaceVariableToValue(stringResult, allStringlist, errorMap);
                stringResult= ParsingToolBox::replacePlaceHolderToValue(stringResult, listFull);

                error= errorMap.values().join(", ");

                int i= strLst.size();
                for(auto it= strLst.rbegin(); it != strLst.rend(); ++it)
                {
                    stringResult.replace(QStringLiteral("µ%1").arg(i), (*it));
                    --i;
                }
                bool isInt= true;
                stringResult.toInt(&isInt);
                if(!isInt)
                    resultStr= stringResult;
            }
            if(format == BOT)
            {
                if(allSameColor)
                {
                    format= in_markdown ? MARKDOWN : TEXT;
                }
                else
                {
#ifdef PAINTER_OP
                    format= IMAGE;
#else
                    format= MARKDOWN;
#endif
                }
                if(!error.isEmpty())
                {
                    format= MARKDOWN;
                }
            }

            switch(format)
            {
            case TERMINAL:
                displayCommandResult(scalarText, resultStr, array, withColor, cmdRework, error, warnings, comment,
                                     allSameFaceCount, allSameColor);
                break;
            case SVG:
                out << displaySVG(scalarText, resultStr, array, withColor, cmdRework, error, warnings, comment,
                                  allSameFaceCount, allSameColor)
                    << "\n";
                break;
            case BOT:
            case MARKDOWN:
                displayMarkdown(scalarText, resultStr, array, withColor, cmdRework, error, warnings, comment,
                                allSameFaceCount, allSameColor);
                break;
            case TEXT:
                displayCommandResult(scalarText, resultStr, array, false, cmdRework, error, warnings, comment,
                                     allSameFaceCount, allSameColor);
                break;
            case JSON:
                displayJSon(scalarText, resultStr, array, withColor, cmdRework, error, warnings, comment,
                            allSameFaceCount, allSameColor);
                break;
#ifdef PAINTER_OP
            case IMAGE:
                displayImage(scalarText, resultStr, array, withColor, cmdRework, error, warnings, comment,
                             allSameFaceCount, allSameColor);
                break;
#endif
            }
            if(!treeFile.isEmpty())
            {
                parser.writeDownDotTree(treeFile);
            }

            if(!error.isEmpty())
            {
                rt= 1;
            }
        }
        else
        {
            QString error= parser.humanReadableError();
            err << "Error: " << error << "\n";
            rt= 1;
        }
    }
    return rt;
}
#include <QTextCodec>
int main(int argc, char* argv[])
{
#ifdef PAINTER_OP
    QGuiApplication a(argc, argv);
#else
    QCoreApplication a(argc, argv);
#endif

    QStringList commands;
    QString cmd;
    QString dotFileStr;
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    bool colorb= true;
    out.setCodec("UTF-8");
    EXPORTFORMAT format= TERMINAL;

    QCommandLineParser optionParser;
    QCommandLineOption color(QStringList() << "c"
                                           << "color-off",
                             "Disable color to highlight result");
    QCommandLineOption version(QStringList() << "v"
                                             << "version",
                               "Show the version and quit.");
    QCommandLineOption reset(QStringList() << "reset-settings", "Erase the settings and use the default parameters");
    QCommandLineOption alias(QStringList() << "a"
                                           << "alias",
                             "path to alias json files: <aliasfile>", "aliasfile");

    QCommandLineOption aliasData(QStringList() << "alias-data", "alias in json data <aliasdata>", "aliasdata");

    QCommandLineOption character(QStringList() << "s"
                                               << "charactersheet",
                                 "set Parameters to simulate character sheet: <sheetfile>", "sheetfile");
    QCommandLineOption markdown(QStringList() << "m"
                                              << "markdown",
                                "The output is formatted in markdown.");
    QCommandLineOption bot(QStringList() << "b"
                                         << "bot",
                           "Discord bot.");
    QCommandLineOption svg(QStringList() << "g"
                                         << "svg",
                           "The output is formatted in svg.");
    QCommandLineOption json(QStringList() << "j"
                                          << "json",
                            "The output is formatted in json.");
    QCommandLineOption dotFile(QStringList() << "d"
                                             << "dot-file",
                               "Instead of rolling dice, generate the execution tree and write it in <dotfile>",
                               "dotfile");
    QCommandLineOption translation(QStringList() << "t"
                                                 << "translation",
                                   "path to the translation file: <translationfile>", "translationfile");
    QCommandLineOption help(QStringList() << "h"
                                          << "help",
                            "Display this help");

    optionParser.addOption(color);
    optionParser.addOption(version);
    optionParser.addOption(reset);
    optionParser.addOption(dotFile);
    optionParser.addOption(alias);
    optionParser.addOption(aliasData);
    optionParser.addOption(character);
    optionParser.addOption(markdown);
    optionParser.addOption(bot);
    optionParser.addOption(svg);
    optionParser.addOption(json);
    optionParser.addOption(translation);
    optionParser.addOption(help);
    for(int i= 0; i < argc; ++i)
    {
        commands << QString::fromUtf8(argv[i]);
    }

    optionParser.process(commands);

    if(optionParser.isSet(color))
    {
        commands.removeAt(0);
        colorb= false;
    }
    else if(optionParser.isSet(version))
    {
        out << "Rolisteam DiceParser v1.0.0"
            << "\n";
        out << "More Details: www.rolisteam.org"
            << "\n";
        return 0;
    }
    else if(optionParser.isSet(reset))
    {
        return 0;
    }
    else if(optionParser.isSet(dotFile))
    {
        dotFileStr= optionParser.value(dotFile);
    }
    if(optionParser.isSet(markdown))
    {
        format= MARKDOWN;
    }
    else if(optionParser.isSet(bot))
    {
        format= BOT;
    }
    else if(optionParser.isSet(svg))
    {
        format= SVG;
    }
    else if(optionParser.isSet(json))
    {
        format= JSON;
    }
    if(optionParser.isSet(help))
    {
        cmd= "help";
    }
    QStringList cmdList= optionParser.positionalArguments();

    if(!cmdList.isEmpty())
    {
        if(cmdList[0].startsWith('&'))
        {
            colorb= false;
        }
    }
    // cmdList << "8d10;\$1c[>6];\$1c[=1];\$2-\$3i:[>0]{\"%3 Success[%2]\"}{i:[<0]{\"Critical fail %3 [%2]\"}{\"Fail %3
    // [%2]\"}}";
    QJsonArray aliases;
    if(optionParser.isSet(alias))
    {
        auto aliasstr= optionParser.value(alias);

        QFile file(aliasstr);

        if(file.open(QIODevice::ReadOnly))
        {
            QJsonDocument doc= QJsonDocument::fromJson(file.readAll());
            aliases= doc.array();
        }
    }
    else if(optionParser.isSet(aliasData))
    {
        auto aliasstr= optionParser.value(aliasData);
        QJsonDocument doc= QJsonDocument::fromJson(aliasstr.toUtf8());
        aliases= doc.array();
    }

    returnValue= startDiceParsing(cmdList, dotFileStr, colorb, format, aliases);
    if(optionParser.isSet(help))
    {
        out << optionParser.helpText();
    }
    return returnValue;
}
