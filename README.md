# Online Chess Application

Network Programming (IT4062E) Project - Group 1

## Setup
1. Install Qt development libraries
```shell
sudo apt install qtbase5-dev qt5-qmake libqt5sql5
```

2. Install mysql connector
```shell
sudo apt install libqt5sql5-mysql  
```

3. Setup MySQL database
```mysql
CREATE DATABASE chess_db;
```

```mysql
CREATE TABLE accounts (
    id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(255) NOT NULL,
    password VARCHAR(255) NOT NULL,
    elo INT DEFAULT 1000
);
```

## Usage
1. Run the server
```bash
cd ChessServer
make
./ChessServer
```

2. Run the client
```bash
cd Client
make
./chessClient
```

3. Register/Login with username, password and the IP address of the server and enjoy the game!