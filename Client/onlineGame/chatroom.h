#ifndef CHATROOM_H
#define CHATROOM_H

#include <QDialog>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include "message.h"

class gameLobby;
class Chatroom: public QDialog
{
    Q_OBJECT
public:
    friend class gameLobby;
    Chatroom(gameLobby *parent,Qt::WindowFlags f=Qt::Widget);
    QListWidget *contentListWidget;
    QLabel *userNameLabel;
    QLineEdit *userNameLineEdit;
    QLineEdit *sendLineEdit;
    QPushButton *sendBtn;
    QGridLayout *mainLayout;
    bool status;
    QString userName;
    void Showmessage(char* String);
    friend class gameLobby;
    friend class ChessRoom;

private slots:
    void sendMessage();

private:
    gameLobby* Parent;
};

#endif 
