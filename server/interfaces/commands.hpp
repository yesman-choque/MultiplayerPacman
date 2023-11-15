#include <string>
#include <list>
#include "./user.hpp"

using namespace std;

void handleRequest(char *buff, User &user, list<User> &users);
void signin(User &user);
void login(User &user, list<User> &users);
void transmit(User &user, string message);