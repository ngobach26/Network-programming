#include "game.h"
#include "gameitems/gameboard.h"
#include <QColor>
#include <QDebug>
#include <math.h>
#include "button.h"
#include "gameitems/queen.h"

int Piece::deadBlack = 0;
int Piece::deadWhite = 0;

game::game(QWidget *parent) : QGraphicsView(parent)
{
    board = NULL;
    gameScene = new QGraphicsScene();
    gameScene->setSceneRect(0, 0, 1400, 950);
    piece_to_placed = NULL;
    setFixedSize(1400, 950);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setScene(gameScene);
    SetGamecolor();
}

void game::placeTheBoard()
{
    board = new gameboard(playerside);
    board->placeBoxes();
}

void game::addToScene(QGraphicsItem *item)
{
    gameScene->addItem(item);
}

void game::start()
{
    delete Siri;
    Siri = NULL;
    AIsSide = -1;
    playerside = 0;
    onlineGame = false;
    gameScene->clear();
    playOffline();
    addToScene(turnDisplay);
    addToScene(check);
    placeTheBoard();
    placePieces();
}

void game::register_user()
{
    QDialog dialog;
    dialog.setWindowTitle("Sign Up an Account");

    QLineEdit text1LineEdit;
    QLineEdit text2LineEdit;
    QLineEdit text3LineEdit;
    QLineEdit text4LineEdit;
    text1LineEdit.setText("127.0.0.1");
    QComboBox level;
    level.addItem(tr("New to chess"));
    level.addItem(tr("Beginner"));
    level.addItem(tr("Intermediate"));
    level.addItem(tr("Advanced"));
    level.addItem(tr("Expert"));

    QFormLayout layout(&dialog);
    layout.addRow("Server IP address:", &text1LineEdit);
    layout.addRow("User name:", &text2LineEdit);
    text3LineEdit.setEchoMode(QLineEdit::Password);
    layout.addRow("Password:", &text3LineEdit);
    text4LineEdit.setEchoMode(QLineEdit::Password);
    layout.addRow("Confirm password:", &text4LineEdit);
    layout.addRow("Level", &level);

    QPushButton okButton("Register");
    layout.addRow(&okButton);

    QObject::connect(&okButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    if (dialog.exec() == QDialog::Accepted)
    {
        QString in_txt[4] = {text1LineEdit.text(), text2LineEdit.text(), text3LineEdit.text(),
                             text4LineEdit.text()};
        for (int i = 0; i < 4; i++)
            if (in_txt[i].isEmpty())
            {
                QMessageBox::critical(NULL, "Error", "Missing Input\nPlease try again!");
                return;
            }
        if (in_txt[0].contains("#") || in_txt[0].contains(","))
        {
            QMessageBox::critical(NULL, "Error", "Your ID contains forbidden character");
            return;
        }
        if (in_txt[2] != in_txt[3])
        {
            QMessageBox::critical(NULL, "Error", "The passwords are different!");
            return;
        }
        if (in_txt[1].contains(" ") || in_txt[2].contains(" ") || in_txt[1].length() < 5 || in_txt[2].length() < 6)
        {
            QMessageBox::critical(NULL, "Error", "Your ID or Password does not match syntax!\nMinimum length of ID(5), password(6)\nID and password cannot contains space character");
            return;
        }
        int socfd = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in sAdds;
        sAdds.sin_family = AF_INET;
        sAdds.sin_port = htons(1111);
        sAdds.sin_addr.s_addr = inet_addr(in_txt[0].toUtf8().constData());
        if (::connect(socfd, (struct sockaddr *)&sAdds, sizeof(sAdds)))
        {
            QMessageBox::critical(NULL, "Error", "Failed to Connect");
            return;
        }
        cJSON *Mesg;
        Mesg = cJSON_CreateObject();
        cJSON_AddStringToObject(Mesg, "Type", "REGISTRATION");
        cJSON_AddStringToObject(Mesg, "UN", in_txt[1].toStdString().c_str());
        cJSON_AddStringToObject(Mesg, "PW", in_txt[2].toStdString().c_str());
        cJSON_AddNumberToObject(Mesg, "ELO", (level.currentIndex() + 1) * 400);
        char *JsonToSend = cJSON_Print(Mesg);
        cJSON_Delete(Mesg);
        if (send(socfd, JsonToSend, 512, NULL) < 0)
        {
            QMessageBox::critical(NULL, "Error", "Failed to Register!");
            return;
        }

        char buffer[512] = {0};
        for (int i = 0; i < 50; i++)
        {
            if (recv(socfd, buffer, sizeof(buffer), MSG_DONTWAIT) > 0)
            {
                qDebug() << buffer;
                cJSON *json, *json_type, *json_Status;
                json = cJSON_Parse(buffer);
                json_type = cJSON_GetObjectItem(json, "Type");
                json_Status = cJSON_GetObjectItem(json, "Status");
                std::string type = "";
                std::string status;
                if (json_type != NULL && json_Status != NULL)
                {
                    type = json_type->valuestring;
                    status = json_Status->valuestring;
                }
                cJSON_Delete(json);
                qDebug() << QString::fromStdString(type) << " " << QString::fromStdString(status);
                if (type == "REGISTRATION_RES" && status == "200")
                {
                    QMessageBox::information(NULL, "Congratulation", "Register successfully!");
                    goto exitMesg;
                }
                else if (type == "REGISTRATION_RES" && status == "409")
                {
                    QMessageBox::critical(NULL, "Error", "User already exists!");
                    goto exitMesg;
                }
                else if (status == "500")
                {
                    QMessageBox::critical(NULL, "Error", "Failed to register!");
                    goto exitMesg;
                }
            }
            QThread::msleep(100);
        }

        QMessageBox::critical(NULL, "Error", "Log In timeout!");
    exitMesg:
        Mesg = cJSON_CreateObject();
        cJSON_AddStringToObject(Mesg, "Type", "EXIT");
        JsonToSend = cJSON_Print(Mesg);
        cJSON_Delete(Mesg);
        send(socfd, JsonToSend, 512, NULL);
        ::close(socfd);
    }
}

boardbox *game::getbox(int i, int j)
{
    boardbox *ret = NULL;
    if (board != NULL)
        ret = board->getbox(i, j);
    if (ret)
        return ret;
    else
        return NULL;
}

void game::pickUpPieces(Piece *P)
{
    if (P != NULL)
    {
        piece_to_placed = P;
        originalPos = P->pos();
    }
}

void game::placePieces()
{
    if (board != NULL)
        board->startup();
}

void game::mainmenu()
{
    delete Siri;
    Siri = NULL;
    playerside = 0;
    onlineGame = false;

    gameScene->clear();
    QGraphicsTextItem *titleText = new QGraphicsTextItem("Online Chess Game");
    QFont titleFont("arial", 50);
    titleText->setFont(titleFont);
    int xPos = width() / 2 - titleText->boundingRect().width() / 2;
    int yPos = 100;
    titleText->setPos(xPos, yPos);
    addToScene(titleText);

    button *registerButton = new button("Register Account");
    xPos = width() / 2 - registerButton->boundingRect().width() / 2;
    yPos = 225;
    registerButton->setPos(xPos, yPos);
    connect(registerButton, SIGNAL(clicked()), this, SLOT(register_user()));
    addToScene(registerButton);

    button *Playonline = new button("Play Online");
    int oxPos1 = width() / 2 - registerButton->boundingRect().width() / 2;
    int oyPos1 = 300;
    Playonline->setPos(oxPos1, oyPos1);
    connect(Playonline, SIGNAL(clicked()), this, SLOT(openGameLobby()));
    addToScene(Playonline);

    button *quitButton = new button("Exit");
    int qxPos = width() / 2 - quitButton->boundingRect().width() / 2;
    int qyPos = 375;
    quitButton->setPos(qxPos, qyPos);
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));
    addToScene(quitButton);
}

void game::openGameLobby()
{
    delete Lobby;
    Lobby = NULL;
    Lobby = new gameLobby();
    if (!Lobby->connectError)
    {
        Lobby->show();

        hide();
    }
    else
    {
        delete Lobby;
        Lobby = NULL;
    }
}

void game::backToLobby()
{
    playerside = 0;
    onlineGame = true;
    mainmenu();
    hide();
    Lobby->yourSide = -1;
    Lobby->inRooms = false;
    Lobby->waiting = false;
    Lobby->host = false;
    Lobby->backToLobby();
}

void game::SHOW()
{
    show();
}

void game::mouseMoveEvent(QMouseEvent *event)
{
    if (piece_to_placed)
    {
        piece_to_placed->setPos((event->pos().x() - 50), (event->pos().y() - 50));
        piece_to_placed->setZValue(10);
    }
    QGraphicsView::mouseMoveEvent(event);
}

void game::mouseReleaseEvent(QMouseEvent *event)
{
    bool dieLogHis = false;
    int startX = event->pos().x();
    int startY = event->pos().y();
    QGraphicsView::mouseReleaseEvent(event);
    int finalX = startX / 100 * 100;
    int finalY = (startY - 50) / 100 * 100;
    int x = finalX / 100 - 3;
    int y = finalY / 100;
    QString hisTxt;
    if (playerside)
    {
        x = 7 - x;
        y = 7 - y;
    }
    finalY += 50;

    if (piece_to_placed)
    {
        int Pieceside = piece_to_placed->getside();
        if (startX < 300 || startX >= 1100)
        {
            piece_to_placed->setPos(originalPos);
            piece_to_placed->setZValue(piece_to_placed->origin_zValue);
            piece_to_placed = NULL;
            return;
        }
        else if (startY < 50 || startY >= 850)
        {
            piece_to_placed->setPos(originalPos);
            piece_to_placed->setZValue(piece_to_placed->origin_zValue);
            piece_to_placed = NULL;
            return;
        }
        boardbox *targetBox = getbox(x, y);
        if (piece_to_placed->pawnAttack(x, y) && targetBox->hasPiece() && targetBox->getpiece()->getside() != piece_to_placed->getside())
        {
            piece_to_placed->tryToMoveTo(x, y);
            if (board->checkCanCheck() == turn)
            {
                board->gobackThinking();
                piece_to_placed->setPos(originalPos);
                piece_to_placed->setZValue(piece_to_placed->origin_zValue);
                piece_to_placed = NULL;
                return;
            }
            else if (board->checkCanCheck() == (!turn))
                checking = true;
            else
                checking = false;
            board->gobackThinking();

            int diediediedie = targetBox->getpiece()->die(playerside);
            dieLogHis = true;
            if (diediediedie + 1)
            {
                qDebug() << "Game over!";
                gameOver(diediediedie);
                if (onlineGame)
                {
                    if (!Lobby->sendMove(piece_to_placed->getCol(), piece_to_placed->getRow(), x, y))
                    {
                        piece_to_placed->setPos(originalPos);
                        piece_to_placed->setZValue(piece_to_placed->origin_zValue);
                        piece_to_placed = NULL;
                        return;
                    }
                }
                hisTxt = piece_to_placed->moveTo(x, y, dieLogHis);
                addMove(hisTxt);
                piece_to_placed->setZValue(piece_to_placed->origin_zValue);
                piece_to_placed = NULL;
                return;
            }
            if (onlineGame)
            {
                if (!Lobby->sendMove(piece_to_placed->getCol(), piece_to_placed->getRow(), x, y))
                {
                    piece_to_placed->setPos(originalPos);
                    piece_to_placed->setZValue(piece_to_placed->origin_zValue);
                    piece_to_placed = NULL;
                    return;
                }
            }
            hisTxt = piece_to_placed->moveTo(x, y, dieLogHis);
            addMove(hisTxt);
            if (y == 0 + Pieceside * 7)
            {
                Piece *newPiece = new queen(Pieceside, x, y);
                newPiece->setPos(finalX, finalY);
                board->appendPieces(newPiece);
                piece_to_placed->setdie(true);
                gameScene->removeItem(piece_to_placed);
                addToScene(newPiece);
                piece_to_placed = newPiece;
                if (board->checkCanCheck() == (!turn))
                    checking = true;
                else
                    checking = false;
                if (checking)
                {
                    if (!check->isVisible())
                        check->setVisible(true);
                    if (!CanYouMove(!turn))
                    {
                        piece_to_placed->setZValue(piece_to_placed->origin_zValue);
                        piece_to_placed = NULL;
                        gameOver(!turn);
                        return;
                    }
                }
            }
            piece_to_placed->setZValue(piece_to_placed->origin_zValue);
            piece_to_placed = NULL;
            if (checking)
            {
                if (!check->isVisible())
                    check->setVisible(true);
                if (!CanYouMove(!turn))
                {
                    gameOver(!turn);
                    return;
                }
            }
            else
                check->setVisible(false);
            changeTurn();
            return;
        }
        else if (piece_to_placed->canmove(x, y))
        {
            piece_to_placed->tryToMoveTo(x, y);
            if (board->checkCanCheck() == turn)
            {
                board->gobackThinking();
                piece_to_placed->setPos(originalPos);
                piece_to_placed->setZValue(piece_to_placed->origin_zValue);
                piece_to_placed = NULL;
                return;
            }
            else if (board->checkCanCheck() == (!turn))
                checking = true;
            else
                checking = false;
            board->gobackThinking();
            if (targetBox->hasPiece())
            {
                if (targetBox->getpiece()->getside() == piece_to_placed->getside())
                {
                    piece_to_placed->setPos(originalPos);
                    piece_to_placed->setZValue(piece_to_placed->origin_zValue);
                    piece_to_placed = NULL;
                    return;
                }
                else
                {
                    int diediediedie = targetBox->getpiece()->die(playerside);
                    dieLogHis = true;
                    if (diediediedie + 1)
                    {
                        qDebug() << "Game over!";
                        gameOver(diediediedie);
                        if (onlineGame)
                        {
                            if (!Lobby->sendMove(piece_to_placed->getCol(), piece_to_placed->getRow(), x, y))
                            {
                                piece_to_placed->setPos(originalPos);
                                piece_to_placed->setZValue(piece_to_placed->origin_zValue);
                                piece_to_placed = NULL;
                                return;
                            }
                        }
                        hisTxt = piece_to_placed->moveTo(x, y, dieLogHis);
                        addMove(hisTxt);
                        piece_to_placed->setZValue(piece_to_placed->origin_zValue);
                        piece_to_placed = NULL;
                        return;
                    }
                }
            }
            if (onlineGame)
            {
                if (piece_to_placed->getType() == 9 && piece_to_placed->getCol() == 4 && (x == 2 || x == 6))
                {
                    int castling = -1;
                    if (x == 2)
                        castling = 0;
                    else
                        castling = 7;
                    if (!Lobby->sendMove(piece_to_placed->getCol(), piece_to_placed->getRow(), x, y, castling))
                    {
                        piece_to_placed->setPos(originalPos);
                        piece_to_placed->setZValue(piece_to_placed->origin_zValue);
                        piece_to_placed = NULL;
                        return;
                    }
                }
                else
                {
                    if (!Lobby->sendMove(piece_to_placed->getCol(), piece_to_placed->getRow(), x, y))
                    {
                        piece_to_placed->setPos(originalPos);
                        piece_to_placed->setZValue(piece_to_placed->origin_zValue);
                        piece_to_placed = NULL;
                        return;
                    }
                }
            }
            hisTxt = piece_to_placed->moveTo(x, y, dieLogHis);
            addMove(hisTxt);
            if (y == 0 + Pieceside * 7 && piece_to_placed->getType() == 4)
            {
                Piece *newPiece = new queen(Pieceside, x, y);
                newPiece->setPos(finalX, finalY);
                board->appendPieces(newPiece);
                piece_to_placed->setdie(true);
                gameScene->removeItem(piece_to_placed);
                addToScene(newPiece);
                piece_to_placed = newPiece;
                if (board->checkCanCheck() == (!turn))
                    checking = true;
                else
                    checking = false;
                if (checking)
                {
                    if (!check->isVisible())
                        check->setVisible(true);
                    if (!CanYouMove(!turn))
                    {
                        piece_to_placed->setZValue(piece_to_placed->origin_zValue);
                        piece_to_placed = NULL;
                        gameOver(!turn);
                        return;
                    }
                }
            }
            piece_to_placed->setZValue(piece_to_placed->origin_zValue);
            piece_to_placed = NULL;
            if (checking)
            {
                if (!check->isVisible())
                    check->setVisible(true);
                if (!CanYouMove(!turn))
                {
                    gameOver(!turn);
                    return;
                }
            }
            else
                check->setVisible(false);
            changeTurn();
            return;
        }
        else
        {
            piece_to_placed->setPos(originalPos);
            piece_to_placed->setZValue(piece_to_placed->origin_zValue);
            piece_to_placed = NULL;
            return;
        }
    }
    piece_to_placed = NULL;
}

int game::getTurn()
{
    return turn;
}

void game::setTurn(int i)
{
    turn = i;
}

void game::AIsMove()
{
    delay();
    bool dieLogHis = false;
    std::shared_ptr<possible_boxNpiece> lol = board->findGoodMovesOneTrun(AIsSide, Siri);
    if (lol == NULL)
    {
        qDebug() << "Game over!";
        gameOver(AIsSide);
        return;
    }
    if (lol->possibleMove->hasPiece())
    {
        Piece *willdie = lol->possibleMove->getpiece();
        int diediediedie = willdie->die(playerside);
        dieLogHis = true;
        if (diediediedie + 1)
        {
            qDebug() << "Game over!";
            gameOver(diediediedie);
            QString hisTxt = lol->targetPiece->moveTo(lol->BoxCol, lol->BoxRow, dieLogHis);
            addMove(hisTxt);
            lol = NULL;
            return;
        }
    }
    QString hisTxt = lol->targetPiece->moveTo(lol->BoxCol, lol->BoxRow, dieLogHis);
    addMove(hisTxt);

    if (board->checkCanCheck() == (!AIsSide))
        checking = true;
    else
        checking = false;
    if (checking)
    {
        if (!check->isVisible())
            check->setVisible(true);
        if (!CanYouMove(!AIsSide))
        {
            gameOver(!AIsSide);
            return;
        }
    }
    else
        check->setVisible(false);
    changeTurn();
}

void game::changeTurn()
{
    if (turn)
    {
        turn = 0;
        turnDisplay->setPlainText("Turn : WHITE");
        if (turn == AIsSide)
            AIsMove();
    }
    else
    {
        turn = 1;
        turnDisplay->setPlainText("Turn : BLACK");
        if (turn == AIsSide)
            AIsMove();
    }
}

void game::addMove(QString move)
{
    currentMovePair.append(move);

    if (currentMovePair.size() == 1)
    {
        QString moveText = QString::number(currentMoveIndex) + ". " + currentMovePair[0];
        historyWidget->addItem(moveText);
    }
    else if (currentMovePair.size() == 2)
    {
        QString moveText = QString::number(currentMoveIndex) + ". " + currentMovePair.join(" ");
        QListWidgetItem *lastItem = historyWidget->item(historyWidget->count() - 1);
        lastItem->setText(moveText);

        currentMoveIndex++;
        currentMovePair.clear();
    }
}

void game::playOffline()
{
    piece_to_placed = NULL;

    turn = 0;
    turnDisplay = new QGraphicsTextItem();
    turnDisplay->setPos(80, 10);
    turnDisplay->setZValue(1);
    turnDisplay->setDefaultTextColor(Qt::white);
    turnDisplay->setFont(QFont("", 20));
    turnDisplay->setPlainText("Turn : WHITE");

    check = new QGraphicsTextItem();
    check->setPos(width() - 250, 10);
    check->setZValue(5);
    check->setDefaultTextColor(Qt::red);
    check->setFont(QFont("", 35));
    check->setPlainText("CHECK!");
    check->setVisible(false);

    QFont titleFont("arial", 15);
    QLabel *guestLabel = new QLabel("");
    guestLabel->setFont(titleFont);

    historyWidget = new QListWidget();
    QVBoxLayout *layout1 = new QVBoxLayout;
    layout1->addWidget(historyWidget);

    QWidget *widget1 = new QWidget;
    widget1->setLayout(layout1);

    QGraphicsProxyWidget *proxyWidget1 = new QGraphicsProxyWidget();
    proxyWidget1->setWidget(widget1);

    proxyWidget1->setPos(width() - 250, 70);
    layout1->addWidget(guestLabel);

    proxyWidget1->resize(200, 350);

    if (onlineGame)
    {
        button *drawButton = new button("Draw");
        connect(drawButton, SIGNAL(clicked()), Lobby, SLOT(I_wannaDraw()));
        drawButton->setZValue(5);
        drawButton->setPos(width() - 250, 450);
        addToScene(drawButton);
    }
    gameScene->addItem(proxyWidget1);
}

void game::SetGamecolor()
{
    QBrush brush;
    brush.setStyle(Qt::SolidPattern);
    QColor gray(Qt::gray);
    brush.setColor(gray);
    setBackgroundBrush(brush);
}

void game::gameOver(int color)
{
    turn = 2;
    connect(this, SIGNAL(EndGame(int)), Lobby, SLOT(EndGame(int)));
    QGraphicsRectItem *rect(new QGraphicsRectItem());
    rect->setRect(0, 0, 450, 300);
    QBrush Abrush;
    Abrush.setStyle(Qt::SolidPattern);
    Abrush.setColor(QColor(199, 231, 253));
    rect->setBrush(Abrush);
    rect->setZValue(4);
    int pxPos = width() / 2 - rect->boundingRect().width() / 2;
    int pyPos = 250;
    rect->setPos(pxPos, pyPos);
    addToScene(rect);

    QGraphicsTextItem *whowin;
    if (color == 0)
        whowin = new QGraphicsTextItem("Black Wins!");
    else if (color == 1)
        whowin = new QGraphicsTextItem("White Wins!");
    else if (color == 2)
        whowin = new QGraphicsTextItem("Draw!");
    if (onlineGame)
    {
        emit EndGame(color);
    }

    QFont titleFont("arial", 30);
    whowin->setFont(titleFont);
    int axPos = width() / 2 - whowin->boundingRect().width() / 2;
    int ayPos = 300;
    whowin->setPos(axPos, ayPos);
    whowin->setZValue(5);
    addToScene(whowin);

    button *replayButton = new button("Play Again");
    int qxPos = width() / 2 - rect->boundingRect().width() / 2 + 10;
    int qyPos = 400;
    replayButton->setPos(qxPos, qyPos);
    replayButton->setZValue(5);
    if (onlineGame)
        connect(replayButton, SIGNAL(clicked()), Lobby, SLOT(I_wannaPlayAgain()));
    else if (AIsSide == -1)
        connect(replayButton, SIGNAL(clicked()), this, SLOT(start()));
    addToScene(replayButton);

    if (onlineGame)
    {
        button *ReturnButton = new button("Return to the Lobby");
        int rxPos = width() / 2 + 15;
        int ryPos = 400;
        ReturnButton->setPos(rxPos, ryPos);
        ReturnButton->setZValue(5);
        connect(ReturnButton, SIGNAL(clicked()), this, SLOT(backToLobby()));
        addToScene(ReturnButton);
    }
    else
    {
        button *ReturnButton = new button("Return to Mainmenu");
        int rxPos = width() / 2 + 15;
        int ryPos = 400;
        ReturnButton->setPos(rxPos, ryPos);
        ReturnButton->setZValue(5);
        connect(ReturnButton, SIGNAL(clicked()), this, SLOT(mainmenu()));
        addToScene(ReturnButton);
    }
    QThread::msleep(500);
    if (onlineGame)
    {
        QGraphicsTextItem *eloGained = new QGraphicsTextItem();
        eloGained->setPlainText("You have gained: " + QString::number(Lobby->recent_elo) + " ELO points!");
        QFont titleFont("arial", 20);
        eloGained->setFont(titleFont);
        eloGained->setPos(width() / 2 - eloGained->boundingRect().width() / 2, 350);
        eloGained->setZValue(5);
        addToScene(eloGained);
    }

    if (onlineGame && Lobby)
    {
        Lobby->waiting = false;

        playerside = 0;
        onlineGame = false;

        if (Lobby->matchingDialog)
        {
            Lobby->matchingDialog->close();
            delete Lobby->matchingDialog;
            Lobby->matchingDialog = nullptr;
        }
    }
}

void game::delay()
{
    QTime dieTime = QTime::currentTime().addSecs(1);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
}

bool game::CanYouMove(int yourturn)
{
    std::vector<std::vector<int>> map = board->createloaclmap();
    findallmovess *yourturnsmove = new findallmovess(yourturn, map);
    if (yourturnsmove->allmoves.length() == 0)
    {
        delete yourturnsmove;
        return false;
    }
    else
    {
        delete yourturnsmove;
        return true;
    }
}

void game::receiveMove(onlineMove *move)
{
    bool dieLogHis = false;
    int fromX = move->fromX;
    int fromY = move->fromY;
    int x = move->ToX;
    int y = move->ToY;
    int Cast = move->Castling;
    delete move;
    boardbox *Piecesbox = board->getbox(fromX, fromY);

    boardbox *targetBox = board->getbox(x, y);
    Piece *targetPiece = Piecesbox->getpiece();

    if (targetBox->hasPiece())
    {
        int diediediedie = targetBox->getpiece()->die(playerside);
        dieLogHis = true;
        if (diediediedie + 1)
        {
            qDebug() << "Game over!";
            gameOver(diediediedie);
            QString hisTxt = targetPiece->moveTo(x, y, dieLogHis);
            addMove(hisTxt);
        }
    }

    if (Cast >= 0)
    {
        Piece *rook = board->getbox(Cast, fromY)->getpiece();
        int rookX;
        if (Cast == 0)
            rookX = 3;
        else
            rookX = 5;
        QString hisTxt = rook->moveTo(rookX, fromY, dieLogHis);
        if (rookX = 3)
            hisTxt = "0-0";
        else
            hisTxt = "0-0-0";
        addMove(hisTxt);
    }
    QString hisTxt = targetPiece->moveTo(x, y, dieLogHis);
    if (Cast < 0)
        addMove(hisTxt);

    if (board->checkCanCheck() == (!turn))
        checking = true;
    else
        checking = false;

    int Pieceside = targetPiece->getside();
    if (y == 0 + Pieceside * 7 && targetPiece->getType() == 4)
    {
        Piece *newPiece = new queen(Pieceside, x, y);
        newPiece->setPos(targetPiece->pos().x(), targetPiece->pos().y());
        board->appendPieces(newPiece);
        targetPiece->setdie(true);
        gameScene->removeItem(targetPiece);
        addToScene(newPiece);
        if (board->checkCanCheck() == (!turn))
            checking = true;
        else
            checking = false;
    }
    if (checking)
    {
        if (!check->isVisible())
            check->setVisible(true);
        if (!CanYouMove(!turn))
        {
            gameOver(!turn);
            return;
        }
    }
    else
        check->setVisible(false);
    changeTurn();
}

void game::closeEvent(QCloseEvent *event)
{
    if (Lobby)
        Lobby->close();
}

void game::playAsWhiteOnline(QString player1, QString player2)
{
    delete Siri;
    Siri = NULL;
    AIsSide = -1;
    playerside = 0;
    onlineGame = true;
    gameScene->clear();
    hostName = player1;
    guestName = player2;
    playOffline();
    addToScene(turnDisplay);
    addToScene(check);
    placeTheBoard();
    placePieces();
}

void game::playAsBlackOnline(QString player1, QString player2)
{
    delete Siri;
    Siri = NULL;
    AIsSide = -1;
    playerside = 1;
    onlineGame = true;
    gameScene->clear();
    hostName = player2;
    guestName = player1;
    playOffline();
    addToScene(turnDisplay);
    addToScene(check);
    placeTheBoard();
    placePieces();
}
void game::playAsWhiteOnline()
{
    try
    {
        if (Siri)
        {
            delete Siri;
            Siri = nullptr;
        }

        AIsSide = -1;
        playerside = 0;
        onlineGame = true;
        gameScene->clear();
        currentMovePair.clear();

        playOffline();

        if (turnDisplay)
            addToScene(turnDisplay);
        if (check)
            addToScene(check);

        placeTheBoard();
        placePieces();

        setTurn(true);
        turnDisplay->setPlainText("Your Turn");
        check->setPlainText("");
    }
    catch (const std::exception &e)
    {
        qDebug() << "Error in playAsWhiteOnline:" << e.what();
    }
}

void game::playAsBlackOnline()
{
    try
    {
        if (Siri)
        {
            delete Siri;
            Siri = nullptr;
        }

        AIsSide = -1;
        playerside = 1;
        onlineGame = true;
        gameScene->clear();
        currentMovePair.clear();

        playOffline();

        if (turnDisplay)
            addToScene(turnDisplay);
        if (check)
            addToScene(check);

        placeTheBoard();
        placePieces();

        setTurn(false);
        turnDisplay->setPlainText("Opponent's Turn");
        check->setPlainText("");
    }
    catch (const std::exception &e)
    {
        qDebug() << "Error in playAsBlackOnline:" << e.what();
    }
}

void game::Draw()
{
    gameOver(2);
}

void game::askDraw()
{
    int reply = QMessageBox::question(NULL, "Draw", "Your opponent request for a Draw\nAccepted?", QMessageBox::Yes | QMessageBox::No);
    Lobby->sendDraw(reply == QMessageBox::Yes);
}