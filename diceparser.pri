
INCLUDEPATH += $$PWD/result
INCLUDEPATH += $$PWD/node
INCLUDEPATH += $$PWD/include
INCLUDEPATH += $$PWD

SOURCES += $$PWD/diceparser.cpp \
    $$PWD/result/diceresult.cpp \
    $$PWD/range.cpp \
    $$PWD/highlightdice.cpp \
    $$PWD/booleancondition.cpp \
    $$PWD/validator.cpp \
    $$PWD/die.cpp \
    $$PWD/result/result.cpp \
    $$PWD/result/scalarresult.cpp \
    $$PWD/parsingtoolbox.cpp \
    $$PWD/result/stringresult.cpp \
    $$PWD/compositevalidator.cpp \
    $$PWD/dicealias.cpp \
    $$PWD/operationcondition.cpp \
    $$PWD/node/stringnode.cpp \
    $$PWD/node/filternode.cpp \
    $$PWD/node/allsamenode.cpp \
    $$PWD/node/groupnode.cpp \
    $$PWD/node/dicerollernode.cpp \
    $$PWD/node/executionnode.cpp \
    $$PWD/node/startingnode.cpp \
    $$PWD/node/rerolldicenode.cpp \
    $$PWD/node/scalaroperatornode.cpp \
    $$PWD/node/numbernode.cpp \
    $$PWD/node/sortresult.cpp \
    $$PWD/node/keepdiceexecnode.cpp \
    $$PWD/node/countexecutenode.cpp \
    $$PWD/node/explodedicenode.cpp \
    $$PWD/node/parenthesesnode.cpp \
    $$PWD/node/helpnode.cpp \
    $$PWD/node/jumpbackwardnode.cpp \
    $$PWD/node/mergenode.cpp \
    $$PWD/node/listaliasnode.cpp \
    $$PWD/node/paintnode.cpp \
    $$PWD/node/ifnode.cpp \
    $$PWD/node/splitnode.cpp \
    $$PWD/node/valueslistnode.cpp \
    $$PWD/node/uniquenode.cpp \
    $$PWD/node/repeaternode.cpp \
    $$PWD/node/listsetrollnode.cpp\
    $$PWD/node/variablenode.cpp\
    $$PWD/node/bind.cpp\
    $$PWD/node/occurencecountnode.cpp\
    $$PWD/diceroller.cpp\
    $$PWD/validatorlist.cpp\
    $$PWD/qmltypesregister.cpp


HEADERS += \
    $$PWD/include/diceparser.h \
    $$PWD/result/diceresult.h \
    $$PWD/range.h \
    $$PWD/booleancondition.h \
    $$PWD/include/highlightdice.h \
    $$PWD/validator.h \
    $$PWD/die.h \
    $$PWD/result/result.h \
    $$PWD/result/scalarresult.h \
    $$PWD/include/parsingtoolbox.h \
    $$PWD/result/stringresult.h \
    $$PWD/compositevalidator.h \
    $$PWD/include/dicealias.h \
    $$PWD/operationcondition.h \
    $$PWD/node/stringnode.h \
    $$PWD/node/filternode.h\
    $$PWD/node/allsamenode.h\
    $$PWD/node/groupnode.h \
    $$PWD/node/variablenode.h\
    $$PWD/node/dicerollernode.h \
    $$PWD/node/executionnode.h \
    $$PWD/node/rerolldicenode.h \
    $$PWD/node/startingnode.h \
    $$PWD/node/scalaroperatornode.h \
    $$PWD/node/numbernode.h \
    $$PWD/node/sortresult.h \
    $$PWD/node/keepdiceexecnode.h \
    $$PWD/node/countexecutenode.h \
    $$PWD/node/explodedicenode.h \
    $$PWD/node/valueslistnode.h \
    $$PWD/node/parenthesesnode.h \
    $$PWD/node/helpnode.h \
    $$PWD/node/jumpbackwardnode.h \
    $$PWD/node/mergenode.h \
    $$PWD/node/listaliasnode.h \
    $$PWD/node/ifnode.h \
    $$PWD/node/bind.h\
    $$PWD/node/splitnode.h \
    $$PWD/node/repeaternode.h \
    $$PWD/node/uniquenode.h \
    $$PWD/node/paintnode.h \
    $$PWD/node/listsetrollnode.h \
    $$PWD/node/occurencecountnode.h\
    $$PWD/diceroller.h \
    $$PWD/validatorlist.cpp\
    $$PWD/include/diceparserhelper.h \
    $$PWD/qmltypesregister.h




OTHER_FILES += \
    $$PWD/HelpMe.md
