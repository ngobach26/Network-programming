#ifndef GAMELOBBY_H
#define GAMELOBBY_H


#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QMutex>
#include <QStandardItemModel>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>

#include "cJSON/cJSON.h"
#include "onlinemove.h"
#include "button.h"
// #include "tablepagination.h"

class ChessRoom;
class Chatroom;

class gameLobby : public QGraphicsView
{
    Q_OBJECT
public:
    gameLobby(QWidget *parent =0);
    ~gameLobby();
    bool connectError = false;
    void connectToServer(QString serverIP);
    static bool is_opened;
    bool CloseConnection();
    bool sendMove(int FromX, int FromY, int ToX, int ToY);
    bool sendMove(int FromX, int FromY, int ToX, int ToY, int castling);
    friend class Chatroom;
    friend class ChessRoom;
    friend class game;
    int yourSide = -1;
    //SignaL_XXXX means send signal for XXXX
    void Signal_socketClosed();
    void Signal_socketClosedfailed();
    void Signal_TimeoutJoin();
    bool backToLobby();
    QString id_id;
    int id_elo;
    int recent_elo = 0;
    void sendDraw(int);
signals:
    void updateRooms(cJSON *Lists);
    void socketClosed();
    void socketClosedfailed();
    void TimeoutJoin();
    void someoneLeave();
    void ShowGame();
    void PlayWhite(QString, QString);
    void PlayBlack(QString, QString);
    void PlayWhiteAgain();
    void PlayBlackAgain();
    void moveTo(onlineMove*); // need to be done
    void Full();
    void RoomClose(); // need to be done;
    void ListFull();
    void Draw();
    void askDraw();



protected:
    void closeEvent(QCloseEvent *event);

private:
    QStringList rankingList;
    int prot = 1111;
    static void ClientThread();
    static void WaitforResponseThread();
    int Connection = -1;
    int numOfRooms = 0;
    bool inRooms = false;
    bool host = false;
    bool waiting = false;
    bool connection = false;
    QGraphicsScene* OnlineScene;
    QGraphicsTextItem *titleText;
    button * playButton;
    bool sendMessage(const std::string& message, const std::string& username);
    bool CreateRoom(const std::string& user);
    bool GetString();
    Chatroom* chRoom;
    void SendRequestForJoining(int ID);
    QList <ChessRoom*> chessroomS;
    void exitLobby();
    void showRooms();
    void LobbySUI();
    void waitingForJoin(); //nedd to be done
    void hostWindow();
    QGraphicsRectItem *rect;
    QGraphicsTextItem *WindowTitle;
    button * CancelBotton;
    std::thread t1;
    std::thread t2;
    void hostWindow_hide();
    void hostWindow_show();
    bool requestLogIn(QString id, QString pw);
    QStandardItemModel* onlineUserList = NULL;
    button* getOnUserBtn;
    button* createRoomBtn;
    button* showChatBtn;
    //void CancelWaiting(); //need to be done
        //void sendMessage(string message);

public slots:
    void createRoomsList(cJSON *Lists);
    void sendJointRequest(int ID);
    void ServerClose();
    void SocketBugs();
    void JoinTimeOut();
    void Leave();
    void List_is_full();
    void This_Game_isFull();
    void I_wannaPlayAgain();
    void ReturnToMenu();
    void CancelHost();
    void GetOnlineUser();
    void CreateAGameRoom();
    void ShowChatRoom();
    void getTopRanking();
    QGraphicsProxyWidget *createRankingWidget();
    void EndGame(int);
    void I_wannaDraw();

};


#endif // GAMELOBBY_H
