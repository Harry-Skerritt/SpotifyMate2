//
// Created by Harry Skerritt on 06/01/2026.
//

#ifndef SYSTEMMANAGER_H
#define SYSTEMMANAGER_H



class SystemManager {
public:
    static SystemManager& getInstance() {
        static SystemManager instance;
        return instance;
    }

    void init();


    void writeConfig();
    void resetConfig();

private:

    SystemManager() {}


    bool loadConfig();


    SystemManager(const SystemManager&) = delete;
    void operator=(const SystemManager&) = delete;
};



#endif //SYSTEMMANAGER_H
