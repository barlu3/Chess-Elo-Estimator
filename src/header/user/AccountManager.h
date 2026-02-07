#ifndef ACCOUNT_MANAGER_H
#define ACCOUNT_MANAGER_H

#include <string> 
#include <vector> 

class AccountManager {
    private: 
        std::string currentPath = "../saves/guest/";
        std::string currentUsername = "guest";

        std::string getPassword();
        void savePassword(const std::string& passsword);

    public:
        AccountManager();

        std::string getCurrentPath() const;
        std::string getCurrentUsername() const;
        bool createAccount(const std::string& username, const std::string& password);
        bool login(const std::string& username, const std::string& password);
        void logout();
        std::vector<std::string> usernameList() const;
        bool deleteAccount(const std::string& username, const std::string& password);

        //for elo saving
        int getElo(const std::string& name) const;
        void setElo(const std::string& name,int rating);
        void saveStats(const std::string& name,int& wins, int& losses);
        void loadStats(const std::string& name,int& wins, int& losses);
};

#endif