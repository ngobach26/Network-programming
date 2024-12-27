#include "chatroom.h"
#include "gamelobby.h"

#include <QDebug>
#include <string>

#pragma execution_character_set("utf-8")

Chatroom::Chatroom(gameLobby *parent,Qt::WindowFlags f): QDialog(parent,f)
{
    setWindowFlags(this->windowFlags() & ~Qt::WindowCloseButtonHint);
    Parent = parent;
    setWindowTitle(tr("Chat room"));

    contentListWidget = new QListWidget;

    sendLineEdit = new QLineEdit;

    userNameLabel = new QLabel(tr("Your Name:"));
    userNameLineEdit = new QLineEdit;

    sendBtn= new QPushButton(tr("Send"));
    mainLayout = new QGridLayout(this);
    mainLayout->addWidget(contentListWidget,0,0,1,2);
    mainLayout->addWidget(userNameLabel,1,0);
    mainLayout->addWidget(userNameLineEdit,1,1);
    mainLayout->addWidget(sendLineEdit,2,0,1,2);
    mainLayout->addWidget(sendBtn,3,0,1,2);
    userNameLineEdit->setText(parent->id_id);
    connect(sendBtn,SIGNAL(clicked()),this,SLOT(sendMessage()));
}

void Chatroom::Showmessage(char* String)
{
    QString msg= QString(QLatin1String(String));
    contentListWidget->addItem(msg);
}


void Chatroom::sendMessage()
{
    if(sendLineEdit->text()=="") return;
    QString msg= userNameLineEdit->text()+":"+sendLineEdit->text();
    if (msg.length() <= 256)
    {
        std::string user = userNameLineEdit->text().toStdString();
        std::string mess = msg.toStdString();
        bool send = Parent->sendMessage(mess,user);
        if (!send)
        {}
    }
    else
    {}
    sendLineEdit->clear();
}