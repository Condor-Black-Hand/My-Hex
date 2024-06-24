#include "gamemodel.h"
#include "qdebug.h"
//#include "qdebug.h"
#include <ctime>

std::map<int, QPoint> centerPointData;//中心点数据

GameModel::GameModel()
{

}

void GameModel::startGame(GameType type)
{
    gameType = type;

    //轮到红方下棋为true, 蓝方为false
    playerFlag = true;
    //初始棋盘
    gameMapVec.clear();
    for(int i = 0; i < BOARD_GRAD_SIZE; i++)
    {
        std::vector<int> lineBoard;
        for(int j = 0; j < BOARD_GRAD_SIZE; j++)
        {
            lineBoard.push_back(0);
        }
        gameMapVec.push_back(lineBoard);
    }

    if(gameType == AI && AIFlag == 1)//AI先手的话
    {
        gameMapVec[3][7] = 1;//胜率较高的点？
        playerFlag = false;//交给对方来下
    }
}

void GameModel::actionByPerson(int row, int col)
{
    updateGameMap(row, col);
}

int GameModel::evaluateScore()
{
    int aiScore = 0; // AI得分
    //int opponentScore = 0; // 对手得分

    // 遍历棋盘上的每个位置
    for (int row = 0; row < BOARD_GRAD_SIZE; row++)
    {
        for (int col = 0; col < BOARD_GRAD_SIZE; col++)
        {
            // 如果该位置是AI的棋子
            if (gameMapVec[row][col] == AIFlag)
            {
                // 计算AI得分
                aiScore += calculateScore(row, col, AIFlag);
            }
        }
    }

    return aiScore;
}

int GameModel::calculateScore(int row, int col, int flag)
{
    int score = 0;

    // 计算边缘接近度得分
    score += calculateEdgeProximity(row, col, flag);

    // 计算阻塞得分
    score += calculateBlockage(row, col, flag);

    //qDebug()<<score;
    return score;
}

int GameModel::calculateBlockage(int row, int col, int flag)//咋感觉没啥用呢
{
    int score = 0;
    int opponentFlag = -flag;

    int many = 5;
    int more = 20;
    int most = 65;//这个很关键

    // 遍历棋子周围的六个位置
    for (int i = 0; i < 6; i++)
    {
        int newRow = row + dx[i];
        int newCol = col + dy[i];

        // 如果该位置在棋盘内，并且是对手的棋子
        if (isValid(newRow, newCol) && gameMapVec[newRow][newCol] == opponentFlag)
        {
            if(flag == 1)//ai红方
            {
                if(col >= 5)//右半边
                {
                    if(i == 0)//正左边
                    {
                        score += more;
                        if(col == 9)//快到边界了
                        {
                            score += most;
                        }
                        if(col == 10)//快到边界了
                        {
                            score += most-20;
                        }
                    }
                    else if(i == 1)//斜左边
                    {
                        score += many;
                    }
                }
                else if(col < 5)//左半边
                {
                    if(i == 3)//正右边
                    {
                        score += more;
                        if(col == 1)//快到边界了
                        {
                            score += most;
                        }
                        if(col == 10)//快到边界了
                        {
                            score += most-20;
                        }
                    }
                    else if(i == 4)//斜右边
                    {
                        score += many;
                    }
                }
            }
            else if(flag == -1)//ai蓝方
            {
                if(row <= 5)//上半边
                {
                    if(i == 2)//正下边
                    {
                        score += more;
                        if(row == 1)//快到边界了
                        {
                            score += most;
                        }
                        if(row == 0)//快到边界了
                        {
                            score += most-20;
                        }
                    }
                    else if(i == 1)//斜下边
                    {
                        score += most;
                    }
                }
                else if(row > 5)//下半边
                {
                    if(i == 5)//正上边
                    {
                        score += more;
                        if(row == 9)//快到边界了
                        {
                            score += most;
                        }
                        if(row == 0)//快到边界了
                        {
                            score += most-20;
                        }
                    }
                    else if(i == 4)//斜上边
                    {
                        score += many;
                    }
                }
            }
        }
    }
    //qDebug()<<score;
    return score;
}

int GameModel::calculateEdgeProximity(int row, int col, int flag)
{
    int score = 0;
    int many = 70;
    int more = 440;
    // 计算棋子到棋盘中心的距离
    int distanceToCenter = abs(BOARD_GRAD_SIZE / 2 - row) + abs(BOARD_GRAD_SIZE / 2 - col);

    // 如果ai是红方
    if (1 == AIFlag)
    {
        // 计算棋子到棋盘上边缘的距离
        int distanceToUpperEdge = row;
        // 计算棋子到棋盘下边缘的距离
        int distanceToLowerEdge = BOARD_GRAD_SIZE - row - 1;

        // 得分为距离的倒数，距离越小，得分越高
        // 同时，距中心距离的倒数越高，离中心越远，得分越低
        score = (many / (distanceToUpperEdge + 1)) + (many / (distanceToLowerEdge + 1)) + (more / (distanceToCenter + 1));
    }

    // 如果ai是蓝方
    else if (-1 == AIFlag)
    {
        // 计算棋子到棋盘左边缘的距离
        int distanceToLeftEdge = col;
        // 计算棋子到棋盘右边缘的距离
        int distanceToRightEdge = BOARD_GRAD_SIZE - col - 1;

        // 得分为距离的倒数，距离越小，得分越高
        // 同时，距离中心越远，得分越低
        score = (many / (distanceToRightEdge + 1)) + (many / (distanceToLeftEdge + 1)) + (more / (distanceToCenter + 1));

    }
    //qDebug()<<score;
    return score;
}

int GameModel::alphaBetaSearch(int depth, int alpha, int beta, int AIFlag, int &bestMoveRow, int &bestMoveCol)//AIFlag代表先手后手 1 or -1
{
    // 判断是否达到搜索深度或游戏结束
    if (depth == 0 || gameStatus == WIN)
    {
        return evaluateScore();
    }

    // 遍历棋盘上的每个位置，也许可以改变方式
    for (int row = 0; row < BOARD_GRAD_SIZE; row++)
    {
        for (int col = 0; col < BOARD_GRAD_SIZE; col++)
        {
            // 如果该位置为空
            if (gameMapVec[row][col] == 0)
            {
                // 假设当前玩家在该位置落子
                gameMapVec[row][col] = AIFlag;
                int score = -alphaBetaSearch(depth - 1, -beta, -alpha, -AIFlag, bestMoveRow, bestMoveCol);
                //将之前假设的落子位置还原为空
                gameMapVec[row][col] = 0;
                // 更新alpha值
                if (score > alpha)
                {
                    alpha = score;
                    if (depth == MAX_DEPTH)
                    {
                        bestMoveRow = row;
                        bestMoveCol = col;
                    }
                }

                //qDebug()<<alpha;

                // 进行α-β剪枝
                if (alpha >= beta)
                {
                    return alpha;
                }
            }
        }
    }
    return alpha;
}

void GameModel::actionByAI(int &clickRow, int &clickCol)//参数是人点的位置，修改之后变成AI的落子位置
{
    // 使用α-β剪枝搜索最佳走法
    int bestMoveRow = -1, bestMoveCol = -1;

    //qDebug()<<AIFlag;
    //关于AIFlag，默认为1(红方)，setAIFlag(aiMode)帮忙设置了是1还是-1
    alphaBetaSearch(MAX_DEPTH, INT_MIN, INT_MAX, AIFlag, bestMoveRow, bestMoveCol);

    // 更新游戏状态
    clickRow = bestMoveRow;
    clickCol = bestMoveCol;
    //qDebug()<<clickRow<<clickCol;

    updateGameMap(clickRow, clickCol);
}

void GameModel::updateGameMap(int row, int col)
{
    if(playerFlag)
        gameMapVec[row][col] = 1;
    else
        gameMapVec[row][col] = -1;

    //换手
    playerFlag = !playerFlag;
}

bool GameModel::isWin(int row, int col)
{
    int targetColor = gameMapVec[row][col];//1 or -1

    // 判断红方是否胜利
    if (targetColor == 1 && checkRedWin()) {
        return true;
    }

    // 判断蓝方是否胜利
    if (targetColor == -1 && checkBlueWin()) {
        return true;
    }

    return false;
}

bool GameModel::checkRedWin()
{
    std::vector<std::vector<bool>> visited(BOARD_GRAD_SIZE, std::vector<bool>(BOARD_GRAD_SIZE, false));
    bool redWin = false;
    for (int i = 0; i < BOARD_GRAD_SIZE; i++) {//从第一排开始
        if (gameMapVec[0][i] == 1 && !visited[0][i]) {
            if (dfs(0, i, visited, 1)) {
                redWin = true;
                break;
            }
        }
    }
    return redWin;
}

bool GameModel::checkBlueWin()
{
    std::vector<std::vector<bool>> visited(BOARD_GRAD_SIZE, std::vector<bool>(BOARD_GRAD_SIZE, false));
    bool blueWin = false;
    for (int i = 0; i < BOARD_GRAD_SIZE; i++) {//从第一列开始
        if (gameMapVec[i][0] == -1 && !visited[i][0]) {
            if (dfs(i, 0, visited, -1)) {
                blueWin = true;
                break;
            }
        }
    }
    return blueWin;
}

bool GameModel::dfs(int row, int col, std::vector<std::vector<bool>>& visited, int targetColor)
{
    // 标记当前格子为已访问
    visited[row][col] = true;

    // 如果当前格子位于目标颜色的边界，则说明找到了一条连接两个边的路径，返回true
    if (col == BOARD_GRAD_SIZE - 1 && gameMapVec[row][col] == -1) {
        return true;//蓝色胜利
    }

    else if (row == BOARD_GRAD_SIZE - 1 && gameMapVec[row][col] == 1) {
        return true;//红色胜利
    }

    // 遍历当前格子的相邻格子
    for (int i = 0; i < 6; i++) {
        int newRow = row + dx[i];
        int newCol = col + dy[i];

        // 判断相邻格子是否合法且未访问过
        if (isValid(newRow, newCol) && !visited[newRow][newCol]) {
            // 如果相邻格子是目标颜色格子，则继续递归搜索
            if (gameMapVec[newRow][newCol] == targetColor) {
                if (dfs(newRow, newCol, visited, targetColor)) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool GameModel::isValid(int row, int col)
{
    return row >= 0 && row < BOARD_GRAD_SIZE && col >= 0 && col < BOARD_GRAD_SIZE;
}
