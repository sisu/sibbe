#ifndef HIGHSCORE_HPP
#define HIGHSCORE_HPP
#include <string>
///Top 10 players
///    ^^
///Ja kyllä, koko paska ykkösindeksoitu.

class HighScore{
public:
    HighScore();
    HighScore(std::string file_name);

    void loadFromFile(std::string file_name);
    void writeToFile(std::string file_name);

    std::string getNameByRank(int rank);
    long long getPointsByRank(int rank);

    int getRankForPoints(long long points);

    void addPlayer(std::string name, long long points);
private:
    std::string names[10];
    long long points[10];
};

#endif
