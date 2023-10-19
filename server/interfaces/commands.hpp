#include <string>
#include <vector>
#include "./user.hpp"

using namespace std;

void handleRequest(char *buff, User &user, vector<User> &users);
void signin(User &user);
void login(User &user, vector<User> &users);