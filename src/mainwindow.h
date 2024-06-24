#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include "gamemodel.h"
#include "qlabel.h"
#include "qtimer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    GameModel *game;//游戏指针
    GameType game_type;//存储游戏类型
    QTimer* updateTimer; // 声明一个 QTimer 对象用于延迟更新操作
    int clickPosRow, clickPosCol, clickPosKey;//存储将点击的位置
    bool selectPos = false;//是否移动到合适的位置，以选中某个点
    QLabel *coordinateLabel;//显示坐标
    QMap<int, QString> columnMap; // 列的映射关系
    bool aiMode = false;//默认ai后手

    void paintEvent(QPaintEvent *event);
    void drawBoard(QPainter& painter);
    void drawSelectedPoint(QPainter& painter, int row, int col, bool playerFlag);
    void drawBoundary(QPainter& painter);
    void drawChess(QPainter& painter);
    void isWinAndEndGame();

    void initGame();
    void initManGame();
    void initAIGame(bool aiMode);
    void mouseMoveEvent(QMouseEvent *event);//move
    void mouseReleaseEvent(QMouseEvent *event);//release
    int findNearestKey(const QPoint &mousePos);//找到最近的中心点

    void mouseDoubleClickEvent(QMouseEvent *event);
    void chessOneByPerson();

    void setAIFlag(bool aiMode);
private slots:
    void updatePosition();//更新当前鼠标位置
    void chessOneByAI();

    void onActionManModeTriggered();//双人模式
    void onActionAIFirstModeTriggered();//人机先手模式
    void onActionAISecondModeTriggered();//人机后手模式
    void onActionRestartTriggered();
};
#endif // MAINWINDOW_H
