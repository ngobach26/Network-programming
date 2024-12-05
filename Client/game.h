#ifndef GAME_H
#define GAME_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QCloseEvent>
#include "gameitems/gameboard.h"
#include "gameitems/boardbox.h"
#include "AI_files/stupid_ai.h"
#include "button.h"
#include "AI_files/possible_boxnpiece.h"
#include "onlineGame/gamelobby.h"
#include "onlineGame/onlinemove.h"
// #include "tablepagination.h"


class game:public QGraphicsView
{
    Q_OBJECT
public:
    game(QWidget *parent = NULL);
    //~game();
    void placeTheBoard();
    void addToScene(QGraphicsItem *item);
    boardbox* getbox(int i, int j);
    void pickUpPieces(Piece* P);
    void placePieces();

    void mouseMoveEvent(QMouseEvent *event);
    //void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    int getTurn();
    void setTurn(int i);
    void changeTurn();
    void playOffline();
    void SetGamecolor();
    void gameOver(int color);
    stupid_AI *Siri = NULL;
    int AIsSide = -1;
    void delay();
    bool checking = false;
    bool CanYouMove(int yourturn);
    bool onlineGame = false;
    friend class Piece;
    QString hostName = "You";
    QString guestName = "Computer";
    void addMove(QString);

public slots:
    // void startVSblackAI();
    // void startVSwhiteAI();
    void start();
    void register_user();
    void mainmenu();
    void openGameLobby();
    void backToLobby();
    void SHOW();
    void playAsWhiteOnline(QString, QString);
    void playAsBlackOnline(QString, QString);
    void playAsWhiteOnline();
    void playAsBlackOnline();
    void receiveMove(onlineMove*);
    void Draw();
    void askDraw();

protected:
    void closeEvent(QCloseEvent *event);

signals:
    void EndGame(int);

private:
    gameLobby *Lobby = NULL;
    QGraphicsTextItem *check;
    QGraphicsScene* gameScene;
    gameboard *board;
    QPointF originalPos;
    Piece* piece_to_placed;
    int turn; // 1 =black, 0 = white
    QGraphicsTextItem * turnDisplay;
    int playerside = 0;
    void AIsMove();
    QListWidget *historyWidget = NULL;
    int currentMoveIndex = 1;
    QStringList currentMovePair;
};

#endif // GAME_H
