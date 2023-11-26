#include "../interfaces/heartbeat.hpp"

void heartbeat(list<User> &users) {

    cout << "I will start the heartbeat" << endl;
    while (true) {
        auto it = users.begin();
        while (it != users.end()) {
            cout << "I will send heartbeat" << endl;

            string message = "security heartbeat";
            transmit(*it, message);
            it++;
        }
        this_thread::sleep_for(chrono::seconds(20));

        it = users.begin();
        while (it != users.end()) {
            if (!it->isAlive) {
                cout << "I will remove user" << endl;
                it = users.erase(it);
            } else {
                cout << "I will set user to not alive" << endl;
                it->isAlive = false;
                it++;
            }
        }
    }
}