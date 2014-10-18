#include "highscore.hpp"
#include <fstream>
#include <iostream>

using namespace std;

HighScore::HighScore(){
    for(int i=0; i<10; i++)
        names[i]="-",points[i]=-1;
}

HighScore::HighScore(string file_name){
    loadFromFile(file_name);
}

void HighScore::loadFromFile(string file_name){
    ifstream fin(file_name);

    if(!fin){
        for(int i=0; i<10; i++)
            names[i]="-",points[i]=-1;
        fin.close();
    }

    for(int i=0; i<10; i++){

        if(!(fin>>names[i])){
            for(; i<10; i++)
                names[i]="-",points[i]=-1;
            fin.close();
            return;
        }

        if(!(fin>>points[i])){
            for(; i<10; i++)
                names[i]="-",points[i]=-1;
            fin.close();
            return;
        }
    }
    fin.close();
}

void HighScore::writeToFile(string file_name){
    ofstream fout(file_name);
    for(int i=0; i<10; i++)
        fout<<names[i]<<" "<<points[i]<<endl;
    fout.close();
}

string HighScore::getNameByRank(int rank){
    return names[rank-1];
}

long long HighScore::getPointsByRank(int rank){
    return max(0ll,points[rank-1]);
}

int HighScore::getRankForPoints(long long p){
    int i=0;
    while(i<10&&points[i]>=p)
        i++;
    return i+1;
}

void HighScore::addPlayer(string name, long long p){
    int rank=getRankForPoints(p);
    if(rank==11)
        return;
    rank--;

    for(int i=8;i>=rank; i--)
        names[i+1]=names[i],points[i+1]=points[i];
    names[rank]=name;
    points[rank]=p;
}
