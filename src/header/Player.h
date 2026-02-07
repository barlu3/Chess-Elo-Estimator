#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include "user/AccountManager.h"

class Player {
    private:
        std::string name;
        AccountManager& accountmanager;
        bool isWhite;
    
    public:
        Player(const std::string& name, AccountManager& am, bool white) : name(name), accountmanager(am), isWhite(white)  {}

        std::string getName() const { return name; }
        bool getColor() const { return isWhite; }

        int getRating() { return accountmanager.getElo(name); }
        void setRating(int rating) { accountmanager.setElo(name,rating); }

        void win() {
            int wins, losses;
            accountmanager.loadStats(name,wins, losses);
            wins++;
            accountmanager.saveStats(name,wins,losses);
        }

        void lose() {
            int wins, losses;
            accountmanager.loadStats(name,wins, losses);
            losses++;
            accountmanager.saveStats(name,wins,losses);
        }
};

#endif