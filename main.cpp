#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <cassert>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include <thread>
#include <sstream>
#include <ext/pb_ds/assoc_container.hpp>
#include "sdl_base.h"
#define NUM_CIVS 19
#define A first
#define B second
#define mp make_pair
using namespace std;
using namespace __gnu_pbds;
template<class T> string to_str(T x)
{
    stringstream ss;
    ss << x;
    return ss.str();
}
void showProgress(int cur, int n, int W = 50)
{
    n = max(n, 1);
    cout << "\r[";
    for(int i=0; i<cur*W/n; i++)
        cout << "=";
    for(int i=0; i<W - cur*W/n; i++)
        cout << " ";
    cout << "] " << cur*100/n << "%";
    cout.flush();
}
string civNames[NUM_CIVS + 100];
struct Player
{
    int civ, rating;
    Player(){}
    Player(int rating, int civ)
    {
        this->rating = rating;
        this->civ = civ;
    }
    string getCivName()
    {
        return civNames[civ];
    }
    static string getCivName(int c)
    {
        return civNames[c];
    }
    static void initCivs()
    {
        civNames[0] = "N/A";
        civNames[1] = "Britons";
        civNames[2] = "Franks";
        civNames[3] = "Goths";
        civNames[4] = "Teutons";
        civNames[5] = "Japanese";
        civNames[6] = "Chinese";
        civNames[7] = "Byzantines";
        civNames[8] = "Persians";
        civNames[9] = "Saracens";
        civNames[10] = "Turks";
        civNames[11] = "Vikings";
        civNames[12] = "Mongols";
        civNames[13] = "Celts";
        civNames[14] = "Spanish";
        civNames[15] = "Aztecs";
        civNames[16] = "Mayans";
        civNames[17] = "Huns";
        civNames[18] = "Koreans";
    }
    static int getCivID(string c)
    {
        for(int i=1; i<NUM_CIVS; i++)
        {
            if(c == civNames[i])
                return i;
        }
        return 0;
    }
};
struct Game
{
    vector<Player> winners, losers;
    string mapName, ladderName, gameMod;
    int duration, matchID;
    Game(){}
    Game(ifstream &fin)
    {
        int wsize, lsize;
        string s;
        getline(fin, s);
        matchID = atoi(s.c_str());
        getline(fin, ladderName);
        getline(fin, gameMod);
        getline(fin, mapName);
        getline(fin, s);
        duration = atoi(s.c_str());
        getline(fin, s);
        wsize = atoi(s.c_str());
        getline(fin, s);
        lsize = atoi(s.c_str());

        winners.resize(wsize);
        losers.resize(lsize);
        for(auto &i: winners)
        {
            getline(fin, s);
            i.civ = atoi(s.c_str());
            getline(fin, s);
            i.rating = atoi(s.c_str());
        }
        for(auto &i: losers)
        {
            getline(fin, s);
            i.civ = atoi(s.c_str());
            getline(fin, s);
            i.rating = atoi(s.c_str());
        }
    }
    bool isValidGame()
    {
        for(auto &i: winners)
            if(i.civ<1 || i.civ>=NUM_CIVS)
                return false;
        for(auto &i: losers)
            if(i.civ<1 || i.civ>=NUM_CIVS)
                return false;
        return getNumPlayers() > 0 && winners.size() == losers.size();
    }
    int getNumPlayers()
    {
        return winners.size() + losers.size();
    }
    int getMatchRating()
    {
        int res = 0;
        for(auto &i: winners)
            res += i.rating;
        for(auto &i: losers)
            res += i.rating;
        return res / (winners.size() + losers.size());
    }
    int getELOdiff()
    {
        int winnerRating = 0, loserRating = 0;
        for(auto &i: winners)
            winnerRating += i.rating;
        for(auto &i: losers)
            loserRating += i.rating;
        return winnerRating/(double)winners.size() - loserRating/(double)losers.size();
    }
    void printInfo()
    {
        if(!isValidGame())
        {
            cout << "Invalid game" << endl;
            return;
        }
        cout << "Match ID: " << matchID << "\n";
        cout << "Ladder: " << ladderName << "\n";
        cout << "Game Mod: " << gameMod << "\n";
        cout << "Map: " << mapName << "\n";
        cout << "Duration: " << duration << "\n";
        cout << "Winning team members:\n";
        cout.setf(ios::left);
        for(auto &i: winners)
            cout << setw(11) << i.getCivName() << i.rating << "\n";
        cout << "Losing team members:\n";
        for(auto &i: losers)
            cout << setw(11) << i.getCivName() << i.rating << "\n";
        cout.flush();
    }
    void outputToFile(ofstream &fout)
    {
        if(!isValidGame())
            return;
        fout << matchID << "\n" << ladderName << "\n" << gameMod << "\n" << mapName << "\n" << duration << "\n";
        fout << winners.size() << "\n" << losers.size() << "\n";
        for(auto &i: winners)
            fout << i.civ << "\n" << i.rating << "\n";
        for(auto &i: losers)
            fout << i.civ << "\n" << i.rating << "\n";
    }
    void outputToFile(string s)
    {
        ofstream fout(s.c_str(), ios::app);
        outputToFile(fout);
    }
    bool operator == (const Game &x) const
    {
        return matchID == x.matchID;
    }
    bool operator < (const Game &x) const
    {
        return matchID < x.matchID;
    }
};
void printWinRates(vector<Game> &games, string ladder = "", string mapName = "", int minRating = 0, int durMin = 0, int durMax = 1e9)
{
    sort(games.begin(), games.end());
    games.resize(unique(games.begin(), games.end()) - games.begin());
    int played[NUM_CIVS]{}, won[NUM_CIVS]{};
    int numGames = 0;
    for(auto &i: games)
    {
        if(i.isValidGame() && (ladder=="" or ladder==i.ladderName) && (mapName=="" or mapName==i.mapName) && i.getMatchRating()>=minRating
        && i.duration>=durMin && i.duration<=durMax)
        {
            for(auto &j: i.winners)
            {
                won[j.civ]++;
                played[j.civ]++;
            }
            for(auto &j: i.losers)
            {
                played[j.civ]++;
            }
            numGames++;
        }
    }
    cout << "n = " << numGames << "\n";
    cout.setf(ios::left);
    vector<pair<double, string> > c;
    for(int i=1; i<NUM_CIVS; i++)
    {
        if(played[i] == 0)
            c.emplace_back(0, Player::getCivName(i));
        else c.emplace_back(won[i] * 100.0 / played[i], Player::getCivName(i));
    }
    sort(c.begin(), c.end());
    for(auto &i: c)
    {
        cout << setw(11) << i.B << " " << i.A << "%\n";
    }
    cout.flush();
}
string tillChar(string &s, unsigned pos, char c)
{
    string res;
    while(s[pos] != c)
        res += s[pos++];
    return res;
}
vector<string> split(string &s, char c = ' ')
{
    vector<string> res;
    int prv = 0;
    s += c;
    for(int i=0; i<s.size(); i++)
    {
        if(s[i] == c)
        {
            if(i != prv)
                res.push_back(s.substr(prv, i-prv));
            prv = i+1;
        }
    }
    s.pop_back();
    return res;
}
int time_str_to_int(string s)
{
    auto x = split(s, ':');
    reverse(x.begin(), x.end());
    int mult = 1;
    int res = 0;
    for(auto &i: x)
    {
        res += atoi(i.c_str()) * mult;
        mult *= 60;
    }
    return res;
}
Game ExtractGameDataFromMatchHTML(string &s)
{
    Game g;
    string patLadder = "Age-of-Empires-II-The-Conquerors/";
    string patMap = "text-align: right;\">";
    string patColWin = "#00A651\">";
    string patColLose = "#FF0000\">";
    string patCiv = "/AOC/civs/";
    string patNewRating = "New Rating: <b>";
    string patMatch = "/match/view/";
    string patDuration = "#EDF3F9\">";
    string patGameMod = patDuration; //they're the same
    auto pos = s.find(patLadder);
    if(pos == string::npos) //not aoe2
        return g;
    pos += patLadder.size();
    g.ladderName = tillChar(s, pos, '\"');
    pos = s.find(patMatch, pos);
    if(pos == string::npos) // no match id
        return g;
    pos += patMatch.size();
    g.matchID = atoi(tillChar(s, pos, '/').c_str());
    pos = s.find(patMap, pos);
    if(pos == string::npos) //no map... bugged???
        return g;
    pos += patMap.size();
    g.mapName = tillChar(s, pos, '<');
    for(int i=0; i<2; i++)
    {
        pos = s.find(patDuration, pos);
        if(pos == string::npos) //no duration?
            return g;
        pos += patDuration.size();
    }
    g.duration = time_str_to_int(tillChar(s, pos, '<'));
    for(int i=0; i<2; i++)
    {
        pos = s.find(patGameMod, pos);
        if(pos == string::npos) //no duration?
            return g;
        pos += patGameMod.size();
    }
    g.gameMod = tillChar(s, pos, '<');
    if(s.find("(Computer)", pos) != string::npos) //no computers
        return Game();
    while(true) //grab winners
    {
        unsigned prvpos = pos;
        pos = s.find(patNewRating, pos);
        assert(pos != string::npos);
        pos += patNewRating.size();
        int rating = atoi(tillChar(s, pos, '<').c_str());
        auto nxt = s.find(patColWin, pos);
        if(nxt == string::npos) //no more green text, so no more winners
        {
            pos = prvpos;
            break;
        }
        nxt += patColWin.size(); //patColWin.size() == patColLose.size() so it doesn't matter

        rating -= atoi(tillChar(s, nxt, '<').c_str()); //calculate the rating before this game happened
        pos = s.find(patCiv, nxt) + patCiv.size();
        int civ = atoi(tillChar(s, pos, '.').c_str());

        g.winners.emplace_back(rating, civ);
    }
    while(true)
    {
        pos = s.find(patCiv, pos);
        if(pos == string::npos)
            return g;
        pos += patCiv.size();
        int civ = atoi(tillChar(s, pos, '.').c_str());
        pos = s.find(patColLose, pos) + patColLose.size();
        int rating = -atoi(tillChar(s, pos, '<').c_str());
        pos = s.find(patNewRating, pos) + patNewRating.size();
        rating += atoi(tillChar(s, pos, '<').c_str());
        g.losers.emplace_back(rating, civ);
    }
}
double checkELOfit(vector<double> &y, vector<int> &weights, int k)
{
    int c = y.size() / 2;
    double res = 0;
    for(int i=0; i<y.size(); i++)
    {
        res += weights[i] * pow(y[i] - 1 / (1 + pow(10, (c-i)/(double)k)), 2);
    }
    return res;
}
void ELOBestFit(vector<double> &y, vector<int> &weights)
{
    double lo = 1, hi = 1e4;
    while(lo + 0.1 < hi)
    {
        double m1 = (2*lo+hi) / 3;
        double m2 = (lo + 2*hi) / 3;
        if(checkELOfit(y, weights, m1) < checkELOfit(y, weights, m2))
            hi = m2;
        else lo = m1;
    }
    cout << "ELO constant = " << (int)lo << endl;
    int c = y.size() / 2, k = lo;
    setColor(0, 0, 255);
    int W = sdl_settings::WINDOW_W, H = sdl_settings::WINDOW_H;
    for(int i=1; i<y.size(); i++)
    {
        drawLine((i-1)*W/y.size(), H / (1 + pow(10, (i-c-1)/(double)k)), i*W/y.size(), H / (1 + pow(10, (i-c)/(double)k)));
    }
}
struct PointGraph
{
    vector<double> y;
    PointGraph(){}
    PointGraph(int size)
    {
        y.resize(size);
    }
    void smooth(int width = 10)
    {
        vector<double> nxt;
        nxt.resize(y.size());
        for(int i=0; i<y.size(); i++)
        {
            int cnt = 0;
            double sum = 0;
            for(int j=max(0, i - width); j<=min((int)y.size()-1, i + width); j++)
            {
                cnt++;
                sum += y[j];
            }
            nxt[i] = sum / cnt;
        }
        y = nxt;
    }
    void render(void (*bestFitFunc)(vector<double>&, vector<int>&) = NULL, vector<int> *weights = NULL)
    {
        int W = sdl_settings::WINDOW_W, H = sdl_settings::WINDOW_H;
        renderClear(255, 255, 255);

        for(int i=1; i<10; i++)
        {
            drawLine(W*i/10, 0, W*i/10, H, 255, 0, 0, 50);
            drawLine(0, H*i/10, W, H*i/10, 0, 255, 0, 50);
        }
        drawLine(W/2, 0, W/2, H, 255, 0, 0, 255);
        drawLine(0, H/2, W, H/2, 0, 255, 0, 255);

        setColor(0, 0, 0);
        for(int i=1; i<y.size(); i++)
        {
            drawLine((i-1)*W/y.size(), H - y[i-1]*H, i*W/y.size(), H - y[i]*H);
        }
        if(bestFitFunc != NULL)
            bestFitFunc(y, *weights);
    }
};
void graphWinRateByELODiff(vector<Game> &games, string ladder = "", string mapName = "", int numPlayers = 0, int minRating = 0, int durMin = 0, int durMax = 1e9)
{
    sort(games.begin(), games.end());
    games.resize(unique(games.begin(), games.end()) - games.begin());
    const int MAX_DIFF = 500;
    int played[MAX_DIFF*2 + 1]{}, won[MAX_DIFF*2 + 1]{};
    int numGames = 0;
    for(auto &i: games)
    {
        if(i.isValidGame() && (ladder=="" or ladder==i.ladderName) && (mapName=="" or mapName==i.mapName) && i.getMatchRating()>=minRating
        && i.duration>=durMin && i.duration<=durMax && (numPlayers==0 || numPlayers==i.getNumPlayers()))
        {
            int d = max(-MAX_DIFF, min(MAX_DIFF, i.getELOdiff()));
            played[MAX_DIFF + d]++;
            played[MAX_DIFF - d]++;
            won[MAX_DIFF + d]++;
            numGames++;
        }
    }
    PointGraph p(MAX_DIFF*2 + 1);
    vector<int> weights;
    weights.resize(p.y.size());
    for(int i=0; i<MAX_DIFF*2 + 1; i++)
    {
        weights[i] = played[i];
        if(played[i] == 0)
        {
            if(i == 0)
                p.y[i] = 0;
            else p.y[i] = p.y[i-1];
            continue;
        }
        p.y[i] = won[i] / (double)played[i];
    }
    p.smooth(10);
    initSDL("ELO Diff Graph");
    cout << "n = " << numGames << endl;
    p.render(ELOBestFit, &weights);
    SDL_RenderPresent(getRenderer());
    while(true)
    {
        while(SDL_PollEvent(&input))
        {
            if(input.type == SDL_QUIT)
            {
                SDL_DestroyWindow(getWindow());
                SDL_DestroyRenderer(getRenderer());
                return;
            }
        }
    }
}
void graphWinRateOverTime(vector<Game> &games, string civ, string ladder = "", string mapName = "", int numPlayers = 0, int minRating = 0, int durMin = 0, int durMax = 1e9)
{
    sort(games.begin(), games.end());
    games.resize(unique(games.begin(), games.end()) - games.begin());
    const int MAX_LEN = 100;
    int played[MAX_LEN+1]{}, won[MAX_LEN+1]{};
    int numGames = 0;
    int civID = Player::getCivID(civ);
    for(auto &i: games)
    {
        if(i.isValidGame() && (ladder=="" or ladder==i.ladderName) && (mapName=="" or mapName==i.mapName) && i.getMatchRating()>=minRating
        && i.duration>=durMin && i.duration<=durMax && (numPlayers==0 || numPlayers==i.getNumPlayers()))
        {
            for(auto &j: i.winners)
            {
                if(j.civ == civID)
                {
                    played[min(i.duration/60, MAX_LEN)]++;
                    won[min(i.duration/60, MAX_LEN)]++;
                    numGames++;
                }
            }
            for(auto &j: i.losers)
            {
                if(j.civ == civID)
                {
                    played[min(i.duration/60, MAX_LEN)]++;
                    numGames++;
                }
            }
        }
    }
    PointGraph p(MAX_LEN+1);
    for(int i=0; i<MAX_LEN; i++)
    {
        if(played[i] == 0)
        {
            if(i == 0)
                p.y[i] = 0;
            else p.y[i] = p.y[i-1];
            continue;
        }
        p.y[i] = won[i] / (double)played[i];
    }
    if(played[MAX_LEN] == 0) //have to do it outside the loop or else "undefined behavior"
        p.y[MAX_LEN] = p.y[MAX_LEN-1];
    else p.y[MAX_LEN] = won[MAX_LEN] / played[MAX_LEN];
    p.smooth(5);
    initSDL((civ + " winrate over time").c_str());
    cout << "n = " << numGames << endl;;
    p.render();
    SDL_RenderPresent(getRenderer());
    while(true)
    {
        while(SDL_PollEvent(&input))
        {
            if(input.type == SDL_QUIT)
            {
                SDL_DestroyWindow(getWindow());
                SDL_DestroyRenderer(getRenderer());
                return;
            }
        }
    }
}
int main(int argc, char **argv)
{
    initSDL("Program");
    SDL_DestroyWindow(getWindow());
    SDL_DestroyRenderer(getRenderer());
    ios::sync_with_stdio(false);
    Player::initCivs();
    ifstream fin("data.txt");
    gp_hash_table<int, null_type> matchIDs;
    vector<Game> games;
    if(!fin.fail())
    {
        while(!fin.eof())
        {
            Game g(fin);
            games.push_back(g);
            matchIDs.insert(g.matchID);
        }
        fin.close();
    }
    printWinRates(games, "RM-Team-Games", "Black Forest", 1700);
    graphWinRateByELODiff(games, "RM-Team-Games", "Black Forest", 8);
    //graphWinRateOverTime(games, "Goths", "RM-Team-Games", "Black Forest", 8, 1600);
    return 0;
}
