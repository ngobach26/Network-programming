#ifndef GAMELOBBY_H
#define GAMELOBBY_H

#include "../elotier.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QMutex>
#include <QStandardItemModel>
#include <cstring>
#include <QListView>


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
enum class StatusCode
{
    // Successful
    OK = 200,
    CREATED = 201,

    // Client Errors
    BAD_REQUEST = 400,
    UNAUTHORIZED = 401,
    FORBIDDEN = 403,
    NOT_FOUND = 404,

    CONFLICT = 409,
    SERVER_ERROR = 500,
    SERVICE_UNAVAIABLE = 503
};
std::string statusToString(StatusCode code);

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
    void sendInvite();
    void updateOnlineUserList(const QStringList &users);
    void inviteReceived(const QString &fromUser, int gameID);
    void inviteResponse(const QString &message, bool success);
    void matchFound(QString opponent);


protected:
    void closeEvent(QCloseEvent *event);

private:
    QStringList rankingList;
    QListView *listView;
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
    void sendInvite(const QString &username);
    QList<ChessRoom *> chessroomS;
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
    button* matchRandomBtn;
    button* matchEloBtn;
    //void CancelWaiting(); //need to be done
        //void sendMessage(string message);
    QDialog* matchingDialog = nullptr;
    QDialog* eloMatchingDialog = nullptr;

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
    void handleOnlineUserList(const QStringList &users);
    void handleInvite(const QString &fromUser, int gameID);
    void onInviteResponse(const QString &message, bool success);
    void MatchRandomPlayer();
    void CancelRandomMatch();
    void MatchEloPlayer();
    void CancelEloMatch();
};


#endif // GAMELOBBY_H
