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


    // For Config.json
    bool loadConfig();
    void writeConfig();
    void resetConfig();

    // For Secrets.json
    bool loadSpotifySecrets();
    //void writeSpotifySecrets();
    //void resetSpotifySecrets();

    // For Spotify Tokens
    bool loadSpotifyTokens();
    void writeSpotifyTokens();
    void resetSpotifyTokens();

private:

    SystemManager() {}



    SystemManager(const SystemManager&) = delete;
    void operator=(const SystemManager&) = delete;
};



#endif //SYSTEMMANAGER_H
