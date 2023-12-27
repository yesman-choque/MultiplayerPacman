<div align="center">
  <h3 align="center">Multiplayer Pacman 👾</h3>

  <p align="center">
    Multiplayer LAN Pacman
  </p>
</div>

<!-- ABOUT THE PROJECT -->
## About The Project

This project involves implementing a Pacman-style game with support for multiple players. The game is developed using the C++ programming language version 17 and the Linux operating system. The game consists of a server and a client for the correct execution of the game.

The server is responsible for managing the game sessions, authenticating users, recording player scores, and maintaining a log file with system information. The client is responsible for sending and receiving messages from the server, performing tasks only when allowed by the server. Additionally, the client supports two communication protocols when exchanging messages with the server: TCP and UDP.

During the game session, the client uses the TCP protocol to send and receive messages from the game (if there is an opponent), and the game is exclusively run through the terminal in text mode. The port used for the game is chosen randomly by the operating system (if there is any available).

The source code is divided into two folders:
- client: contains the client's source code
- server: contains the server's source code

The source code for both the client and the server is organized into two subfolders:
- src: contains the implementations
- interfaces: contains the header files

Additionally, the server uses two text files to ensure data persistence:
- users.txt: contains the registered users in the system
- log.txt: contains system information

<div align="center">
   <img src="https://github.com/yesmanic/Multiplayer-Pacman/assets/62268626/8d8d50fb-f591-4d0f-a133-2c9365bd6bd7" alt="screenshot" height="300">
</div>

### Built With

* [C++](https://cplusplus.com/)

## Getting Started

To get a local copy up and running follow these simple steps.

### Prerequisites

* git
* g++
* make

### Installation

1. Clone the repo
    ```sh
    git clone https://github.com/yesmanic/Multiplayer-Pacman.git
    ```
2. Compile the source
    ```sh
    make
    ```
3. Run the server
    ```sh
    ./server/bin/program <port>
    ```
4. Run the client
    ```sh
    ./client/bin/program <ip> <port> <protocol>
    ```

### Tests

The following tests were conducted to verify the server's functionality. For each test, the command executed on the client and the output obtained from the terminal are presented.
(The tests were performed on localhost at port 8080)

Test 1: User creation and login

    Pac-Man> new user 123
    Pac-Man> Success to signin
    Pac-Man> enter user 123
    Pac-Man> Success to login

(For the tests below, it is necessary for the user to be logged in.)

Test 2: Checking connected users

    Pac-Man> list
    Players online: 1
    user

Test 3: Ranking of top players

    Pac-Man> leaders
    Table of leaders:
    user 0

Test 4: Start of the game

    Pac-Man> start
    (An arena is created in the terminal, and the game begins)
    move right
    (The player moves to the right)

## Contact

Yesman - yesman.choque@gmail.com

Project Link: https://github.com/yesmanic/Multiplayer-Pacman

<p align="right">(<a href="#top">back to top</a>)</p>
