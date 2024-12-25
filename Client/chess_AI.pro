#-------------------------------------------------
#
# Project created by QtCreator 2018-06-07T18:32:17
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = chessClient
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    main.cpp \
    game.cpp \
    gameitems/gameboard.cpp \
    gameitems/boardbox.cpp \
    gameitems/piece.cpp \
    gameitems/king.cpp \
    gameitems/queen.cpp \
    gameitems/bishop.cpp \
    gameitems/rook.cpp \
    gameitems/pawn.cpp \
    gameitems/knight.cpp \
    button.cpp \
    AI_files/possible_boxnpiece.cpp \
    AI_files/stupid_ai.cpp \
    AI_files/findallmovess.cpp \
    onlineGame/chessroom.cpp \
    onlineGame/chatroom.cpp \
    onlineGame/gamelobby.cpp \
    cJSON/cJSON.c

HEADERS += \
    game.h \
    gameitems/gameboard.h \
    gameitems/boardbox.h \
    gameitems/piece.h \
    gameitems/king.h \
    gameitems/queen.h \
    gameitems/bishop.h \
    gameitems/rook.h \
    gameitems/pawn.h \
    gameitems/knight.h \
    button.h \
    AI_files/possible_boxnpiece.h \
    AI_files/stupid_ai.h \
    AI_files/findallmovess.h \
    AI_files/moves.h \
    AI_files/positioncalcuation.h \
    onlineGame/chessroom.h \
    onlineGame/chatroom.h \
    onlineGame/message.h \
    onlineGame/onlinemove.h \
    onlineGame/gamelobby.h \
    cJSON/cJSON.h 

FORMS += \
    mainwindow.ui

RESOURCES += \
    res.qrc
