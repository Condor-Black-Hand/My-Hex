#ifndef GAMEMODEL_H
#define GAMEMODEL_H

#include <QPoint>
#include <cmath>
#include <map>
#include <vector>

enum GameType
{
    MAN,//人人
    AI  //人机
};

enum GameStatus
{
    PLAYING,
    WIN
};

//棋盘参数
const int BOARD_GRAD_SIZE = 11;//棋盘尺寸 11*11
const int NUM_ROWS = BOARD_GRAD_SIZE;//行 0-11 从代码理解上还是有必要写这两行
const int NUM_COLS = BOARD_GRAD_SIZE;//列 0-11
const int MARGIN = 60;//边距
const int CELL_SIDE_LENGTH = 40;//格子边长
const int CELL_VER_LENGTH = CELL_SIDE_LENGTH * cos(M_PI/6);//格子垂直半径
const int POS_OFFSET = CELL_VER_LENGTH * 0.8;//鼠标模糊距离
const int MARK_SIZE = 5;//标记大小

const int AI_THINK_TIME = 0;//AI下棋的思考时间

const int dx[] = {0, 1, 1, 0, -1, -1};
const int dy[] = {-1, -1, 0, 1, 1, 0};

class GameModel
{
public:
    GameModel();

    std::vector<std::vector<int>> gameMapVec;    //当前游戏棋盘和棋子的情况，空白为0，红方为1，蓝方为-1
    //std::vector<std::vector<int>> scoreMapVec; //各个点位的评分情况，作为AI下棋依据？
    bool playerFlag;    //标示下棋方 true:红方 false:蓝方 AI方
    GameType gameType;//游戏模式
    GameStatus gameStatus;//游戏状态

    void startGame(GameType type);//开始游戏
    void actionByPerson(int row, int col);//人执行下棋
    void actionByAI(int &clickRow, int &clickCol);//机器执行下棋
    void updateGameMap(int row, int col);//每次落子之后更新游戏棋盘

    bool isWin(int row, int col);//判断游戏是否胜利
    bool dfs(int row, int col, std::vector<std::vector<bool>>& visited, int targetColor);
    bool isValid(int row, int col);
    bool checkRedWin();
    bool checkBlueWin();

    int AIFlag = 1;//默认我方AI是红方
    int alphaBetaSearch(int depth, int alpha, int beta, int currentPlayer, int &bestMoveRow, int &bestMoveCol);
    int evaluateScore();
    int calculateScore(int row, int col, int flag);
    int calculateBlockage(int, int, int);
    int calculateEdgeProximity(int, int, int);

    int MAX_DEPTH = 4;

private:


};

#endif // GAMEMODEL_H
