#include "../interfaces/heartbeat.hpp"

void heartbeat(list<User> &users) {
    while (true) {
        auto it = users.begin();
        while (it != users.end()) {
            string message = "security heartbeat";
            transmit(*it, message);
            it++;
        }
        this_thread::sleep_for(chrono::seconds(20));

        it = users.begin();
        while (it != users.end()) {
            if (!it->isAlive) {
                cout << "Client is died" << endl;
                string level, event, details;
                level = "WARNING";
                event = "Client is died";
                details = "IP: " + getIp(*it);
                report(level, event, details);
                it = users.erase(it);
            }
            
            else {
                cout << "Client is alive" << endl;
                it->isAlive = false;
                it++;
            }
        }
    }
}