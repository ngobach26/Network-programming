﻿#include "Server.h"

#include <arpa/inet.h>
#include <cmath>

#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>

void log(const std::string &message)
{
    std::ofstream logFile("../ChessServer/server.log", std::ios::app);

    if (!logFile.is_open())
    {
        std::cerr << "Error opening log file: " << strerror(errno) << std::endl;
        return;
    }

    // Get current time
    std::time_t currentTime = std::time(nullptr);
    std::tm *localTime = std::localtime(&currentTime);

    // Format time for the log entry
    char timeBuffer[1024];
    std::strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", localTime);

    // Write log entry
    logFile << "[" << timeBuffer << "] " << message << std::endl;
    logFile.flush(); // Flush the buffer to ensure the message is written to file
    logFile.close(); // Close the file stream

    if (logFile.fail())
    {
        std::cerr << "Error writing to log file: " << strerror(errno) << std::endl;
    }
}

Server::Server(int PORT, bool BroadcastPublically)
{
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);

    if (BroadcastPublically)
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // Listen on all interfaces
    else
        servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Only local connections

    sListen = socket(AF_INET, SOCK_STREAM, 0); // Create a TCP socket
    if ((::bind(sListen, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
    {
        std::string ErrorMsg = "Failed to bind the address to our listening socket.";
        std::cout << ErrorMsg << std::endl;
        log(ErrorMsg + " " + std::to_string(PORT) + "\nError code: " + std::to_string(errno));
        exit(1);
    }

    if ((listen(sListen, SOMAXCONN)) != 0)
    {
        std::string ErrorMsg = "Failed to listen on listening socket.";
        std::cout << ErrorMsg << std::endl;
        log(ErrorMsg + " " + std::to_string(PORT) + "\nError code: " + std::to_string(errno));
        exit(1);
    }

    std::cout << "Server is listening on port " << PORT << "..." << std::endl;

    serverptr = this;
    accList.clear();
    GetAllAccounts();
}


bool Server::Signup(QString username, QString password, int elo)
{
    std::lock_guard<std::mutex> guard(mutexLock);
    SqlConnector connector;
    if (connector.openConnection())
    {
        qDebug() << "Connected to the database!";
    }
    else
    {
        qDebug() << "Cannot connect to the database!";
        exit(66);
    }
    QSqlQuery query;
    QString sQuery = "INSERT INTO accounts (user_name, password, elo) VALUES (:username, :password,:elo)";
    query.prepare(sQuery);
    query.bindValue(":username", username);
    query.bindValue(":password", password);
    query.bindValue(":elo", elo);
    if (query.exec())
    {
        qDebug() << "Data inserted successfully.";
        return true;
    }
    else
    {
        qDebug() << "Error executing query:";
        qDebug() << query.lastError().text();
        return false;
    }
    connector.closeConnection();
}

void Server::GetAllAccounts()
{
    SqlConnector connector;
    if (connector.openConnection())
    {
        qDebug() << "Connected to the database!";
    }
    else
    {
        qDebug() << "Cannot connect to the database!";
        exit(66);
    }
    QSqlQuery query = connector.executeQuery("SELECT * FROM accounts");
    while (query.next())
    {
        QString user_name = query.value(0).toString();
        QString password = query.value(1).toString();
        int elo = query.value(2).toInt();

        qDebug() << "USER NAME: " << user_name;
        qDebug() << "PW: " << password;
        qDebug() << "ELO: " << elo;

        Account tmpAcc;
        tmpAcc.ID = user_name;
        tmpAcc.PassW = password;
        tmpAcc.elo = elo;
        accList.push_back(tmpAcc);
    }
    connector.closeConnection();
}

bool Server::SendString(int ID, string &_string)
{
    // Log the message being sent to the terminal
    std::cout << "Sending to client ID " << ID << ": " << _string << std::endl;

    int RetnCheck = send(Connections[ID], _string.c_str(), 512, 0); // Send string buffer
    if (RetnCheck < 0)                                              // If failed to send string buffer
    {
        std::cerr << "Failed to send message to client ID " << ID << std::endl;
        return false; // Return false: Failed to send string buffer
    }
    return true; // Return true: string successfully sent
}

bool Server::GetString(int ID, string &_string)
{
    char buffer[512];
    int RetnCheck = recv(Connections[ID], buffer, 512, 0); // nhan thong diep tu 1 ket noi voi ID cu the
    _string = buffer;
    // TO DO:
    // when recv failed ,delete it!
    if (RetnCheck < 0)
        return false;
    cout << "get:" << endl
         << _string << endl;
    return true;
}

void Server::sendMessToClients(string Message)
{
    cout << "send:" << endl
         << Message << endl;
    unordered_map<int, int>::iterator it;
    for (it = Connections.begin(); it != Connections.end(); ++it)
    {
        if (!SendString(it->first, Message))
        { // Send message to connection at index i, if message fails to be sent...
            cout << "Failed to send message to client ID: " << it->first << endl;
            log("Failed to send message to client ID: " + std::to_string(it->first));
        }
    }
}

bool Server::ListenForNewConnection()
{
    int newConnection = accept(sListen, (struct sockaddr *)&servaddr, &addrlen); // Accept a new connection
    if (newConnection < 0)                                                       // If accepting the client connection failed
    {
        cout << "Failed to accept the client's connection." << endl;
        log("Failed to accept the client's connection.");
        return false;
    }
    else // If client connection properly accepted
    {
        cout << "Client Connected! ID:" << allID << endl;
        log("Client Connected! ID:" + std::to_string(allID));
        Connections.insert(pair<int, int>(allID, newConnection));
        threadList[allID] = std::thread(&Server::ClientHandlerThread, this, allID);
        player newPlayer(new Player(allID));
        PlayerList.insert(pair<int, player>(allID, newPlayer));
        allID++;
        TotalConnections += 1; // Incremenent total # of clients that have connected
        return true;
    }
}

bool Server::Processinfo(int ID)
{
    string Message;              // string to store our message we received
    if (!GetString(ID, Message)) // Get the chat message and store it in variable: Message
        return false;            // If we do not properly get the chat message, return false
                                 // Next we need to send the message out to each user
    cJSON *json, *json_type;
    json = cJSON_Parse(Message.c_str());
    json_type = cJSON_GetObjectItem(json, "Type"); // kiem tra loai thong diep
    if (json_type == NULL)
    {
        sendMessToClients(Message);
        cout << "Processed chat message packet from user ID: " << ID << endl;
        log("Processed chat message packet from user ID: " + std::to_string(ID));
        cJSON_Delete(json);
        return true;
    }
    else
    {
        string type = json_type->valuestring;
        if (type == "Message")
            sendMessToClients(Message);
        else if (type == "REGISTRATION")
        {
            cJSON *user_ID_Json;
            user_ID_Json = cJSON_GetObjectItem(json, "UN");
            cJSON *user_PW_Json;
            user_PW_Json = cJSON_GetObjectItem(json, "PW");
            cJSON *user_Elo_Json;
            user_Elo_Json = cJSON_GetObjectItem(json, "ELO");
            QString reg_ID = user_ID_Json->valuestring;
            QString reg_PW = user_PW_Json->valuestring;
            int reg_ELO = user_Elo_Json->valueint;
            bool userExists = false;
            for (const auto &acc : accList)
            {
                if (acc.ID == reg_ID)
                {
                    userExists = true;
                    break;
                }
            }

            if (userExists)
            {
                sendResponse(ID, "REGISTRATION_RES", StatusCode::CONFLICT);
            }
            else
            {
                if (Signup(reg_ID, reg_PW, reg_ELO))
                {
                    sendResponse(ID, "REGISTRATION_RES", StatusCode::OK);
                    accList.push_back(Account(reg_ID, reg_PW, reg_ELO));
                }
                else
                {
                    sendResponse(ID, "REGISTRATION_RES", StatusCode::SERVER_ERROR);
                }
            }
        }
        else if (type == "LOGIN")
        {
            // su dung cSJON de lay thong tin user
            cJSON *user_UN_Json;
            user_UN_Json = cJSON_GetObjectItem(json, "UN");
            cJSON *user_PW_Json;
            user_PW_Json = cJSON_GetObjectItem(json, "PW");
            // chuyen sang kieu QString de tich hop vao cac ham cua QT
            QString user_UN = user_UN_Json->valuestring;
            QString user_PW = user_PW_Json->valuestring;
            int flag = -1, isLogin = false;
            for (int i = 0; i < accList.size(); i++)
            {
                if (accList[i].ID == user_UN && accList[i].PassW == user_PW)
                {
                    if (accList[i].login)
                    {
                        isLogin = true; // tai khoan da duoc dang nhap va khong can update lai trang thai dang nhap
                        break;
                    }
                    flag = i;
                    accList[i].login = true;
                    break;
                }
            }
            if (flag > -1)
            {
                sendResponse(ID, "LOGIN_RES", StatusCode::OK, "elo", std::to_string(accList[flag].elo));
                sendGameList(ID);
                OnlineUserList[ID] = user_UN;
            }
            else
            {
                if (!isLogin)
                    sendResponse(ID, "LOGIN_RES", StatusCode::BAD_REQUEST);
                else
                    sendResponse(ID, "LOGIN_RES", StatusCode::CONFLICT); // tai khoan da duoc dang nhap
            }
        }
        else if (type == "CREATEROOM")
        {
            if (GameList.size() >= 6)
            {
                cJSON_Delete(json);
                sendResponse(ID, "CREATEROOM_RES", StatusCode::SERVICE_UNAVAIABLE);
                return true;
            }
            if (PlayerList[ID]->isFree())
            {
                cJSON *Username;
                Username = cJSON_GetObjectItem(json, "User");
                int gameID = GameNum;
                GameNum++;
                onlineGame newGame(new Game(gameID, ID, Username->valuestring)); // tao game moi
                PlayerList[ID]->hostGame(gameID, newGame);                       // cho nguoi choi vao game
                newGame->hostIs(PlayerList[ID]);                                 // host la nguoi choi vua tao game
                GameList.insert(pair<int, onlineGame>(gameID, newGame));
                sendResponse(ID, "CREATEROOM_RES", StatusCode::OK);
                sendGameList(-1);
            }
            else
            {
                cout << "The player is already in game!" << endl;
                cJSON_Delete(json);
                log("The player is already in game!");
                return true;
            }
        }
        else if (type == "PLAY_AGAIN")
        {
            if (PlayerList[ID]->AreYouInGame() >= 0)
            {                                                           // kiem tra xem nguoi choi co dang choi game nao khong
                int HID = PlayerList[ID]->AreYouInGame();               // lay ID cua game dang choi
                GameList[HID]->booltest();                              // debug
                GameList[HID]->playAgain(ID);                           // player gui thong diep PlayAgain
                int anotherPlayer = GameList[HID]->anotherPlayerID(ID); // lay ID cua doi thu
                if (anotherPlayer >= 0)
                { // neu doi thu trong Game
                    if (GameList[HID]->can_Play_again())
                    { // kiem tra xem co the choi lai khong
                        if (systemSend(ID, "SEND_PLAY_AGAIN") && systemSend(anotherPlayer, "SEND_PLAY_AGAIN"))
                            GameList[HID]->reset_play_again();
                    }
                }
                else
                    GameList[HID]->reset_play_again();
            }
        }
        else if (type == "MOVE")
        {
            if (PlayerList[ID]->AreYouInGame() >= 0)
            {
                int HID = PlayerList[ID]->AreYouInGame();               // lay ID cua game dang choi
                int anotherPlayer = GameList[HID]->anotherPlayerID(ID); // lay ID cua doi thu
                if (anotherPlayer >= 0)
                {                                       // neu doi thu trong Game
                    SendString(anotherPlayer, Message); // gui thong diep toi doi thu
                }
            }
        }
        else if (type == "INVITE")
        {
            cJSON *recipientJson = cJSON_GetObjectItem(json, "User");
            QString recipientName = recipientJson->valuestring;
            int senderID = ID;
            QString senderName = OnlineUserList[senderID];

            // Check if the sender is hosting a room
            if (PlayerList[senderID]->isOnlyInRoom())
            {
                // Check if the recipient exists and is online
                auto recipientIt = std::find_if(OnlineUserList.begin(), OnlineUserList.end(),
                                                [&recipientName](const std::pair<int, QString> &pair)
                                                {
                                                    return pair.second == recipientName;
                                                });

                if (recipientIt != OnlineUserList.end())
                {
                    int recipientID = recipientIt->first;

                    // Check if recipient is not in a game
                    if (PlayerList[recipientID]->isFree())
                    {
                        // Send the invite to the recipient
                        systemSend(
                            recipientID, "INVITE_RECEIVED",
                            "Data", senderName.toStdString() + "#" + std::to_string(PlayerList[senderID]->AreYouInGame()));

                        // Optionally acknowledge the sender
                        sendResponse(senderID, "INVITE_RES", StatusCode::OK);
                    }
                    else
                    {
                        // Recipient is busy
                        sendResponse(senderID, "INVITE_RES", StatusCode::CONFLICT);
                    }
                }
                else
                {
                    // Recipient not found
                    sendResponse(senderID, "INVITE_RES", StatusCode::NOT_FOUND);
                }
            }
            else
            {
                // Sender is not hosting a room
                sendResponse(senderID, "INVITE_RES", StatusCode::FORBIDDEN);
            }
        }
        else if (type == "JOIN_ROOM")
        {
            cJSON *Games_ID;
            Games_ID = cJSON_GetObjectItem(json, "ID");
            cJSON *Username;
            Username = cJSON_GetObjectItem(json, "User");
            int gameID = Games_ID->valueint;
            string p2name = Username->valuestring;
            cout << "got Join room request from ID: " << Games_ID->valueint << endl;
            log("got Join room request from ID: " + std::to_string(Games_ID->valueint));
            if (GameList[gameID]->isFull())
            {
                sendResponse(ID, "JOIN_ROOM_RES", StatusCode::CONFLICT);
                cJSON_Delete(json);
                return true;
            }
            else if (GameList[gameID]->hostsID() == ID)
            {
                cJSON_Delete(json);
                return true;
            }
            else
            {
                auto game = GameList[gameID];
                auto player = PlayerList[ID];

                game->Joinin(ID, player, p2name);
                player->JoininGame(gameID, game);
                sendGameList(-1);
                int hostID = game->hostsID();
                std::string hostName = game->hostName;
                std::string opponentName = game->p2Name;
                if (sendResponse(ID, "JOIN_ROOM_RES", StatusCode::OK, "Name_Info", hostName))
                {
                    sendResponse(hostID, "SEND_OPPONENT_JOINED", StatusCode::OK, "Name_Info", opponentName);
                }
            }
        }
        else if (type == "BACK_TO_LOBBY")
        {
            if (PlayerList[ID]->AreYouInGame() >= 0)
            {
                int HID = PlayerList[ID]->AreYouInGame();
                int anotherPlayer = GameList[HID]->anotherPlayerID(ID);
                if (anotherPlayer >= 0)
                {
                    systemSend(anotherPlayer, "SEND_OPPONENT_LEAVED");
                    PlayerList[anotherPlayer]->returnToLobby();
                }
                PlayerList[ID]->returnToLobby();
                GameList.erase(HID);
                sendGameList(-1);
            }
            else
                PlayerList[ID]->returnToLobby();
        }
        else if (type == "CANCEL_HOST")
        {
            if (PlayerList[ID]->AreYouInGame() >= 0)
            {
                int HID = PlayerList[ID]->AreYouInGame();
                int anotherPlayer = GameList[HID]->anotherPlayerID(ID);
                if (anotherPlayer >= 0)
                {
                    systemSend(anotherPlayer, "SEND_OPPONENT_CANCELED_HOST");
                    PlayerList[anotherPlayer]->returnToLobby();
                }
                PlayerList[ID]->returnToLobby();
                GameList.erase(HID);
                sendGameList(-1);
            }
            else
                PlayerList[ID]->returnToLobby();
        }
        else if (type == "GET_ONLINE_USERS")
        {
            string res = "";
            for (auto it = OnlineUserList.begin(); it != OnlineUserList.end(); ++it)
            {
                res += it->second.toStdString();
                if (std::next(it) != OnlineUserList.end())
                {
                    res += ","; // Add a comma only if this is not the last element
                }
            }
            sendResponse(ID, "GET_ONLINE_USERS_RES", StatusCode::OK, "Data", res.c_str());
        }
        else if (type == "EXIT")
        {
            return false;
        }
        else if (type == "GET_TOP_RANKING")
        {
            QString res = GetTopRanking();
            sendResponse(ID, "GET_TOP_RANKING_RES", StatusCode::OK, "Response", res.toStdString().c_str());
        }
        else if (type == "EndGame")
        {
            cJSON *json_result = cJSON_GetObjectItem(json, "Winner");
            int eloA, eloB;
            int result = json_result->valueint;
            int HID = PlayerList[ID]->AreYouInGame();
            if (HID >= 0)
            {
                int anotherPlayer = GameList[HID]->anotherPlayerID(ID);
                if (anotherPlayer >= 0)
                {
                    float res;
                    std::string name;

                    if (PlayerList[ID]->ishost)
                    {
                        name = GameList[HID]->hostName;
                        eloA = NameToElo(name);

                        eloB = NameToElo(GameList[HID]->p2Name);
                        res = result;
                    }
                    else
                    {
                        name = GameList[HID]->p2Name;
                        eloA = NameToElo(name);
                        eloB = NameToElo(GameList[HID]->hostName);
                        if (result == 0)
                            res = 1;
                        else if (result == 1)
                            res = 0;
                    }
                    if (result == 2)
                        res = 0.5;
                    int gain = CalculateElo(eloA, eloB, res);
                    cJSON *json = cJSON_CreateObject();
                    cJSON_AddStringToObject(json, "Type", "Result");
                    cJSON_AddNumberToObject(json, "elo", gain);
                    char *JsonToSend = cJSON_Print(json);
                    cJSON_Delete(json);
                    cout << "send:" << endl
                         << JsonToSend << " To: " << ID << endl;
                    string Send(JsonToSend);
                    SendString(ID, Send);
                    UpdateElo(name, gain);
                }
            }
        }
        else if (type == "ASK_DRAW" || type == "DRAW")
        {
            if (PlayerList[ID]->AreYouInGame() >= 0)
            {
                int HID = PlayerList[ID]->AreYouInGame();
                int anotherPlayer = GameList[HID]->anotherPlayerID(ID);
                if (anotherPlayer >= 0)
                {
                    SendString(anotherPlayer, Message);
                }
            }
        }
    }
    cout << "Processed chat message packet from user ID: " << ID << endl;
    cJSON_Delete(json);
    return true;
}

QString Server::GetTopRanking()
{
    std::lock_guard<std::mutex> guard(mutexLock);
    QString res = "";
    SqlConnector connector;
    if (connector.openConnection())
    {
        qDebug() << "Connected to the database!";
    }
    else
    {
        qDebug() << "Cannot connect to the database!";
        exit(66);
    }
    QSqlQuery query = connector.executeQuery("SELECT * FROM accounts ORDER BY elo DESC LIMIT 50;");
    while (query.next())
    {
        QString user_name = query.value(0).toString();
        int elo = query.value(2).toInt();

        res += (user_name + "#" + QString::number(elo) + ",");
    }
    connector.closeConnection();
    return res;
}

bool Server::CreateGameList(string &_string)
{
    cJSON *json = cJSON_CreateObject();
    if (cJSON_AddStringToObject(json, "Type", "List_of_Rooms") == NULL)
    {
        cout << "type add failed." << endl;
        cJSON_Delete(json);
        return false;
    }
    cJSON *Lists = NULL;
    Lists = cJSON_AddArrayToObject(json, "List");
    if (Lists == NULL)
    {
        cout << "List add failed." << endl;
        cJSON_Delete(json);
        return false;
    }
    unordered_map<int, onlineGame>::iterator it;
    for (it = GameList.begin(); it != GameList.end(); ++it)
    {
        cJSON *GAME = cJSON_CreateObject();

        if (cJSON_AddStringToObject(GAME, "name", it->second->hostName.c_str()) == NULL)
        {
            cJSON_Delete(json);
            return false;
        }
        if (cJSON_AddNumberToObject(GAME, "id", it->second->id) == NULL)
        {
            cJSON_Delete(json);
            return false;
        }
        if (cJSON_AddNumberToObject(GAME, "isplay", it->second->isplay) == NULL)
        {
            cJSON_Delete(json);
            return false;
        }
        string p2name;
        if (it->second->isplay)
            p2name = it->second->p2Name;
        else
            p2name = "";
        if (cJSON_AddStringToObject(GAME, "p2name", p2name.c_str()) == NULL)
        {
            cJSON_Delete(json);
            return false;
        }
        cJSON_AddItemToArray(Lists, GAME);
    }

    _string = cJSON_Print(json);
    cJSON_Delete(json);
    return true;
    ;
}

bool Server::sendSystemInfo(int ID, string InfoType, string addKey, string addValue)
{
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "Type", "System");
    cJSON_AddStringToObject(json, "System_Info", InfoType.c_str());

    if (addKey != "")
    {
        cJSON_AddStringToObject(json, addKey.c_str(), addValue.c_str());
    }

    char *JsonToSend = cJSON_Print(json);
    cJSON_Delete(json);
    cout << "send:" << endl
         << JsonToSend << " To: " << ID << endl;
    log("send:" + std::string(JsonToSend) + " To: " + std::to_string(ID));
    string Send(JsonToSend);
    return SendString(ID, Send);
}

std::string statusToString(StatusCode code)
{
    switch (code)
    {
    case StatusCode::OK:
        return "200";
    case StatusCode::CREATED:
        return "201";
    case StatusCode::BAD_REQUEST:
        return "400";
    case StatusCode::UNAUTHORIZED:
        return "401";
    case StatusCode::FORBIDDEN:
        return "403";
    case StatusCode::NOT_FOUND:
        return "404";
    case StatusCode::CONFLICT:
        return "409";
    case StatusCode::SERVER_ERROR:
        return "500";
    case StatusCode::SERVICE_UNAVAIABLE:
        return "503";
    default:
        return "Unknown Status";
    }
}
bool Server::systemSend(int ID, string type, string addKey, string addValue)
{
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "Type", type.c_str());

    if (addKey != "")
    {
        cJSON_AddStringToObject(json, addKey.c_str(), addValue.c_str());
    }

    char *JsonToSend = cJSON_Print(json);
    cJSON_Delete(json);
    // cout << "send:" << endl
    //      << JsonToSend << " To: " << ID << endl;
    // log("send:" + std::string(JsonToSend) + " To: " + std::to_string(ID));
    string Send(JsonToSend);
    return SendString(ID, Send);
}

#include <vector>

bool Server::sendResponse(int ID, string type, StatusCode status, string addKey, string addValue)
{
    cJSON *json = cJSON_CreateObject();
    string str_status = statusToString(status);
    cJSON_AddStringToObject(json, "Type", type.c_str());
    cJSON_AddStringToObject(json, "Status", str_status.c_str());

    // Add additional key-value pairs from the arrays
    if (addKey != "")
    {
        cJSON_AddStringToObject(json, addKey.c_str(), addValue.c_str());
    }

    char *JsonToSend = cJSON_Print(json);
    cJSON_Delete(json);
    // cout << "send:" << endl
    //      << JsonToSend << " To: " << ID << endl;
    // log("send:" + std::string(JsonToSend) + " To: " + std::to_string(ID));
    string Send(JsonToSend);
    return SendString(ID, Send);
}

bool Server::sendGameList(int ID)
{
    string Message;
    if (!CreateGameList(Message))
    {
        cout << "create game list JSON failed!" << endl;
        log("create game list JSON failed!");
        return false;
    }
    if (Message == "NULL")
        return true;
    if (ID < 0)
        sendMessToClients(Message);
    else
        SendString(ID, Message);
    return true;
}

void Server::ClientHandlerThread(int ID) // ID = the index in the SOCKET Connections array
{
    while (true)
    {
        if (!serverptr->Processinfo(ID))
            break;
    }
    cout << "Lost connection to client ID: " << ID << endl;
    log("Lost connection to client ID: " + std::to_string(ID));
    close(serverptr->Connections[ID]);
    for (int i = 0; i < accList.size(); i++)
    {
        if (accList[i].ID == OnlineUserList[ID])
        {
            accList[i].login = false;
            break;
        }
    }
    OnlineUserList.erase(ID);
    if (serverptr->PlayerList[ID]->AreYouInGame() >= 0)
    {
        cout << "Delete its game room: " << endl;
        log("Delete its game room: ");
        int HID = serverptr->PlayerList[ID]->AreYouInGame();
        int anotherPlayer = serverptr->GameList[HID]->anotherPlayerID(ID);
        if (anotherPlayer >= 0 && serverptr->PlayerList[anotherPlayer])
        {
            serverptr->systemSend(anotherPlayer, "LOST_CONNECTION");
            serverptr->PlayerList[anotherPlayer]->returnToLobby();
        }
        serverptr->GameList.erase(HID);
        serverptr->sendGameList(-1);
    }
    serverptr->PlayerList.erase(ID);
    serverptr->Connections.erase(ID);
    serverptr->TotalConnections -= 1;
}

int Server::NameToElo(std::string name)
{
    QString elo = QString::fromStdString(name).split("#").at(1);
    return elo.toInt();
}

int Server::CalculateElo(int playerA, int playerB, float result)
{
    int K;
    if (playerA < 2100)
        K = 32;
    else if (playerA < 2400)
        K = 24;
    else
        K = 16;

    float expectedA = (1 + std::pow(10, (playerB - playerA) / 400));
    expectedA = 1 / expectedA;

    return round(K * (result - expectedA));
}

void Server::UpdateElo(std::string nameElo, int gain)
{
    std::lock_guard<std::mutex> guard(mutexLock);
    QString name = QString::fromStdString(nameElo).split("#").at(0);
    for (int i = 0; i < accList.size(); i++)
    {
        if (accList[i].ID == name)
        {
            accList[i].elo += gain;
            break;
        }
    }
    SqlConnector connector;
    if (connector.openConnection())
    {
        qDebug() << "Connected to the database!";
    }
    else
    {
        qDebug() << "Cannot connect to the database!";
        exit(66);
    }
    QSqlQuery query;
    QString sQuery = "UPDATE accounts SET elo = elo + :value WHERE user_name = :username";
    query.prepare(sQuery);
    query.bindValue(":username", name);
    query.bindValue(":value", gain);
    if (query.exec())
    {
        qDebug() << "Data updated successfully.";
        return;
    }
    else
    {
        qDebug() << "Error executing query:";
        qDebug() << query.lastError().text();
        return;
    }
    connector.closeConnection();
}
