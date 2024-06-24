#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QPainter>
#include <QMouseEvent>
#include <cmath>    // 用于使用qRound函数
#include <QLabel>
#include <QTimer>
#include <QMessageBox>
#include <QToolBar>
#include <QMenuBar>
#include <QDir>

extern std::map<int, QPoint> centerPointData;//中心点数据

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //setMouseTracking(true);//设计师里两个都要勾
    // 初始化列的映射关系
    columnMap[1] = "A";
    columnMap[2] = "B";
    columnMap[3] = "C";
    columnMap[4] = "D";
    columnMap[5] = "E";
    columnMap[6] = "F";
    columnMap[7] = "G";
    columnMap[8] = "H";
    columnMap[9] = "I";
    columnMap[10] = "J";
    columnMap[11] = "K";

    int window_y = MARGIN * 2 + CELL_SIDE_LENGTH * sin(M_PI/6) + BOARD_GRAD_SIZE * (CELL_SIDE_LENGTH + CELL_SIDE_LENGTH * sin(M_PI/6));
    int window_x = MARGIN * 2 + 2 * CELL_VER_LENGTH * BOARD_GRAD_SIZE + (BOARD_GRAD_SIZE - 1) * CELL_VER_LENGTH;
    setFixedSize(window_x, window_y);//初始窗口大小

    //布局管理器也会影响最终大小
    //qDebug()<<"window_x="<<window_x<<"window_y="<<window_y;

    //状态栏
    coordinateLabel = new QLabel("0", this);
    ui->statusbar->addPermanentWidget(coordinateLabel);

    // 创建菜单栏
    QMenuBar *menuBar = new QMenuBar(this);

    // 创建人机模式菜单
    QMenu *aiModeMenu = menuBar->addMenu(tr("人机模式"));
    QAction *aiFirstModeAction = aiModeMenu->addAction(tr("我方人机先手"));
    QAction *aiSecondModeAction = aiModeMenu->addAction(tr("我方人机后手"));

    // 添加菜单项
    QAction *manModeAction = menuBar->addAction(tr("双人模式"));
    QAction *restartAction = menuBar->addAction(tr("重新开始"));

    // 连接信号和槽
    connect(manModeAction, &QAction::triggered, this, &MainWindow::onActionManModeTriggered);
    connect(aiFirstModeAction, &QAction::triggered, this, &MainWindow::onActionAIFirstModeTriggered);
    connect(aiSecondModeAction, &QAction::triggered, this, &MainWindow::onActionAISecondModeTriggered);
    connect(restartAction, &QAction::triggered, this, &MainWindow::onActionRestartTriggered);

    // 将菜单栏添加到主窗口
    setMenuBar(menuBar);

    //创建定时器并连接到槽函数
    updateTimer = new QTimer(this);
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(updatePosition()));

    initGame();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    // 禁用双击关闭窗口的操作
    event->ignore();
}

void MainWindow::chessOneByPerson()
{
    //有效点击并且该处没有子
    if(clickPosRow != -1 && clickPosCol != -1 && game->gameMapVec[clickPosRow][clickPosCol] == 0)
    {
        game->actionByPerson(clickPosRow, clickPosCol);
        //没有落子音效

        update();
    }
}

void MainWindow::setAIFlag(bool aiMode)
{
    if(aiMode == true)
    {
        game->AIFlag = 1;
    }
    else
    {
        game->AIFlag = -1;
    }

}

void MainWindow::chessOneByAI()
{
    game->actionByAI(clickPosRow, clickPosCol);
    update();
}

void MainWindow::onActionManModeTriggered()
{
    initManGame();
}

void MainWindow::onActionAIFirstModeTriggered()
{
    aiMode = true;//我方AI先手（红色）
    initAIGame(aiMode);
}

void MainWindow::onActionAISecondModeTriggered()
{
    aiMode = false;//我方AI后手（蓝色）
    initAIGame(aiMode);
}

void MainWindow::onActionRestartTriggered()
{
    if (game_type == MAN)
    {
        initManGame();
    }
    else if (game_type == AI)
    {
        initAIGame(aiMode);
    }
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setPen(QColor(0,0,0));//黑色
    painter.setRenderHint(QPainter::Antialiasing);//抗锯齿

    //绘制棋盘并保存中心点
    drawBoard(painter);

    //绘制边界
    drawBoundary(painter);

    //绘制选中点标记
    drawSelectedPoint(painter, clickPosRow, clickPosCol, game->playerFlag);

    //绘制棋子
    drawChess(painter);

    // 结束绘图操作
    painter.end();

    //判断输赢并结算
    isWinAndEndGame();
}

void MainWindow::drawBoard(QPainter &painter)
{
    //绘制棋盘并保存中心点
    for(int row = 0; row < NUM_ROWS; ++row)
    {
        for(int col = 0; col < NUM_COLS; ++col)
        {
            QPolygon cell;//六边形
            //每个格子的中心点
            int center_x = MARGIN + (row + 1) * CELL_VER_LENGTH + 2 * col * CELL_VER_LENGTH;
            int center_y = MARGIN + CELL_SIDE_LENGTH + row * CELL_SIDE_LENGTH * 3/2;

            // 记录每个格子的坐标和编号信息
            QPoint centerPoint(center_x, center_y);//记录每个中心点
            int key = row * NUM_COLS + col ;//保证每个格子都有单独的对应
            centerPointData[key] = centerPoint;//<key,point>

            //计算该格子的六个顶点
            for (int i = 0; i < 6; ++i)
            {
                if(0 == i)
                {
                    int vertex_x = center_x + CELL_SIDE_LENGTH * cos(M_PI/6);
                    int vertex_y = center_y + CELL_SIDE_LENGTH * sin(M_PI/6);
                    cell << QPoint(vertex_x, vertex_y);
                }
                else
                {
                    int vertex_x = center_x + CELL_SIDE_LENGTH * cos(M_PI/6 + M_PI*i/3);
                    int vertex_y = center_y + CELL_SIDE_LENGTH * sin(M_PI/6 + M_PI*i/3);
                    cell << QPoint(vertex_x, vertex_y);
                }
            }

            //设置填充颜色
            QColor grayColor(192, 192, 192);
            if (row >= 0 && row <= NUM_ROWS - 1) {
                painter.setBrush(QBrush(grayColor));
            } else {
                painter.setBrush(Qt::NoBrush);
            }

            painter.drawPolygon(cell);
        }
    }
}

void MainWindow::drawSelectedPoint(QPainter &painter, int row, int col, bool playerFlag)
{
    //绘制选中点
    QPen pen;
    pen.setWidth(MARK_SIZE);
    if(row >= 0 && row < BOARD_GRAD_SIZE &&
        col >= 0 && col < BOARD_GRAD_SIZE &&
        game->gameMapVec[row][col] == 0)
    {
        if(playerFlag)
        {
            pen.setColor(Qt::red);
        }
        else
        {
            pen.setColor(Qt::blue);
        }
        pen.setWidth(7);
        painter.setPen(pen);
        // 使用centerPointData[clickPosKey]绘制选中点
        QPoint selectedPoint = centerPointData[clickPosKey];
        int x = selectedPoint.x();
        int y = selectedPoint.y();
        painter.drawPoint(x, y);
    }
}

void MainWindow::drawBoundary(QPainter &painter)
{
    //画边界，覆盖原来的边界，都倒着画
    int width = 6;
    //上边界
    for(int col = 10, row = 0; col >= 0; col--)
    {//10-0
        QPolygon top;
        QPen pen;
        pen.setColor(QColor(255, 0, 0));//红色(255, 0, 0)
        pen.setWidth(width);
        painter.setPen(pen);

        //每个格子的中心点
        int center_x = MARGIN + (row+1)*CELL_VER_LENGTH + 2*col*CELL_VER_LENGTH;
        int center_y = MARGIN + CELL_SIDE_LENGTH + row*CELL_SIDE_LENGTH*3/2;
        //计算顶点，极坐标法，也是从右到左

        //在数学中，逆时针方向被定义为正角度，顺时针方向被定义为负角度。然而，在某些编程框架或库中，坐标系的定义可能与数学中的定义不同。

        //在Qt中，通常使用的是笛卡尔坐标系，其中原点位于左上角，y轴向下延伸，x轴向右延伸。这与数学中常用的笛卡尔坐标系定义是相反的。

        //因此，在Qt中，当您使用cos和sin函数计算坐标时，角度的正负号与数学中的定义相反。如果您想要实现逆时针旋转，应该使用负角度；如果您想要实现顺时针旋转，应该使用正角度。

        //先按照正常的写（左边都是都是+，让角度去决定，角度要有正负），再把Y轴角度反一下就行了
        for(int i = 0; i < 3; i++){//每个块分配三个点，最后一个点重合的不影响
            if(col == 10){//最右边那个块
                if(i == 0){//的最右边那个点
                    int x = center_x + CELL_VER_LENGTH*cos(M_PI/3);
                    int y = center_y + CELL_VER_LENGTH*sin(-M_PI/3);
                    top << QPoint(x, y);
                }
                else if(i == 1){//的第2个点
                    int x = center_x + CELL_SIDE_LENGTH*cos(M_PI/3 + M_PI/6);
                    int y = center_y + CELL_SIDE_LENGTH*sin(-(M_PI/3 + M_PI/6));
                    top << QPoint(x, y);
                }
                else{//的第三个点
                    int x = center_x + CELL_SIDE_LENGTH*cos(M_PI/3 + M_PI/6 + M_PI/3);
                    int y = center_y + CELL_SIDE_LENGTH*sin(-(M_PI/3 + M_PI/6 + M_PI/3));
                    top << QPoint(x, y);
                }
            }
            else{//左边10个块
                if(i == 0){//的最右边那个点
                    int x = center_x + CELL_SIDE_LENGTH*cos(M_PI/6);
                    int y = center_y + CELL_SIDE_LENGTH*sin(-M_PI/6);
                    top << QPoint(x, y);
                }
                else{//的剩下2个点
                    int x = center_x + CELL_SIDE_LENGTH*cos(M_PI/6 + M_PI*i/3);
                    int y = center_y + CELL_SIDE_LENGTH*sin(-(M_PI/6 + M_PI*i/3));
                    top << QPoint(x, y);
                }
            }
        }//内层for结束
        painter.drawPolyline(top);//连线
    }

    //下边界
    for(int col = 10, row = 10; col >= 0; col--)
    {//10-0
        QPolygon bottom;
        QPen pen;
        pen.setColor(QColor(255, 0, 0));//红色
        pen.setWidth(width);
        painter.setPen(pen);

        //每个格子的中心点
        int center_x = MARGIN + (row+1)*CELL_VER_LENGTH + 2*col*CELL_VER_LENGTH;
        int center_y = MARGIN + CELL_SIDE_LENGTH + row*CELL_SIDE_LENGTH*3/2;
        //计算顶点，极坐标法，也是从右到左

        for(int i = 0; i < 3; i++){//每个块分配三个点，最后一个点重合的不影响
            if(col == 0){//最左边那个块
                if(i == 0){//的最右边那个点
                    int x = center_x + CELL_SIDE_LENGTH*cos(M_PI/6);
                    int y = center_y + CELL_SIDE_LENGTH*sin(M_PI/6);
                    bottom << QPoint(x, y);
                }
                else if(i == 1){//的第2个点
                    int x = center_x + CELL_SIDE_LENGTH*cos(M_PI/6 + M_PI/3);
                    int y = center_y + CELL_SIDE_LENGTH*sin(M_PI/6 + M_PI/3);
                    bottom << QPoint(x, y);
                }
                else{//的第3个点
                    int x = center_x + CELL_VER_LENGTH*cos(M_PI/6 + M_PI/3 + M_PI/6);
                    int y = center_y + CELL_VER_LENGTH*sin(M_PI/6 + M_PI/3 + M_PI/6);
                    bottom << QPoint(x, y);
                }
            }
            else{//右边10个块
                if(i == 0){//的最右边那个点
                    int x = center_x + CELL_SIDE_LENGTH*cos(M_PI/6);
                    int y = center_y + CELL_SIDE_LENGTH*sin(M_PI/6);
                    bottom << QPoint(x, y);
                }
                else{//的剩下2个点
                    int x = center_x + CELL_SIDE_LENGTH*cos(M_PI/6 + M_PI*i/3);
                    int y = center_y + CELL_SIDE_LENGTH*sin(M_PI/6 + M_PI*i/3);
                    bottom << QPoint(x, y);
                }
            }
        }
        painter.drawPolyline(bottom);//连线
    }

    //左边界
    for(int row = 10, col = 0; row >= 0; row--)
    {
        QPolygon left;
        QPen pen;
        pen.setColor(QColor(0, 0, 255));//蓝色(0, 0, 255)
        pen.setWidth(width);
        painter.setPen(pen);

        //每个格子的中心点
        int center_x = MARGIN + (row+1)*CELL_VER_LENGTH + 2*col*CELL_VER_LENGTH;
        int center_y = MARGIN + CELL_SIDE_LENGTH + row*CELL_SIDE_LENGTH*3/2;
        //计算顶点，极坐标法，也是从右到左

        for(int i = 0; i < 3; i++){//每个块分配三个点，最后一个点重合的不影响
            if(row == 10){//最下边那个块
                if(i == 0){//的最下边那个点
                    int x = center_x + CELL_VER_LENGTH*cos(2*M_PI/3);
                    int y = center_y + CELL_VER_LENGTH*sin(2*M_PI/3);
                    left << QPoint(x, y);
                }
                else if(i == 1){//的第2个点
                    int x = center_x + CELL_SIDE_LENGTH*cos(2*M_PI/3 + M_PI/6);
                    int y = center_y + CELL_SIDE_LENGTH*sin(2*M_PI/3 + M_PI/6);
                    left << QPoint(x, y);
                }
                else{//的第3个点
                    int x = center_x + CELL_SIDE_LENGTH*cos(3*M_PI/3 + M_PI/6);
                    int y = center_y + CELL_SIDE_LENGTH*sin(3*M_PI/3 + M_PI/6);
                    left << QPoint(x, y);
                }
            }
            else{//上边10个块
                if(i == 0){//的最下边那个点
                    int x = center_x + CELL_SIDE_LENGTH*cos(M_PI/2);
                    int y = center_y + CELL_SIDE_LENGTH*sin(M_PI/2);
                    left << QPoint(x, y);
                }
                else{//的剩下2个点 60du
                    int x = center_x + CELL_SIDE_LENGTH*cos(M_PI/2 + M_PI*i/3);
                    int y = center_y + CELL_SIDE_LENGTH*sin(M_PI/2 + M_PI*i/3);
                    left << QPoint(x, y);
                }
            }
        }
        painter.drawPolyline(left);//连线

    }

    //右边界
    for(int row = 10, col = 10; row >= 0; row--)
    {
        QPolygon right;
        QPen pen;
        pen.setColor(QColor(0, 0, 255));//蓝色
        pen.setWidth(width);
        painter.setPen(pen);

        //每个格子的中心点
        int center_x = MARGIN + (row+1)*CELL_VER_LENGTH + 2*col*CELL_VER_LENGTH;
        int center_y = MARGIN + CELL_SIDE_LENGTH + row*CELL_SIDE_LENGTH*3/2;
        //计算顶点，极坐标法，也是从右到左

        for(int i = 0; i < 3; i++){//每个块分配三个点，最后一个点重合的不影响
            if(row == 0){//最上边那个块
                if(i == 0){//的最下边那个点 -30du
                    int x = center_x + CELL_SIDE_LENGTH*cos(-M_PI/6);
                    int y = center_y + CELL_SIDE_LENGTH*sin(M_PI/6);
                    right << QPoint(x, y);
                }
                else if(i == 1){//的第2个点 60du
                    int x = center_x + CELL_SIDE_LENGTH*cos(M_PI/6);
                    int y = center_y + CELL_SIDE_LENGTH*sin(-(-M_PI/6 + M_PI/3));
                    right << QPoint(x, y);
                }
                else{//的第3个点 30du
                    int x = center_x + CELL_VER_LENGTH*cos(M_PI/3);
                    int y = center_y + CELL_VER_LENGTH*sin(-(M_PI/3));
                    right << QPoint(x, y);
                }
            }
            else{//下边10个块
                if(i == 0){//的最下边那个点 -30du
                    int x = center_x + CELL_SIDE_LENGTH*cos(-M_PI/6);
                    int y = center_y + CELL_SIDE_LENGTH*sin(M_PI/6);
                    right << QPoint(x, y);
                }
                else{//的剩下2个点 60du
                    int x = center_x + CELL_SIDE_LENGTH*cos((-M_PI/6) + M_PI/3*i);
                    int y = center_y + CELL_SIDE_LENGTH*sin(-(-M_PI/6 + M_PI/3*i));
                    right << QPoint(x, y);
                }
            }
        }
        painter.drawPolyline(right);//连线
    }
}

void MainWindow::drawChess(QPainter &painter)
{
    for(int row = 0; row < BOARD_GRAD_SIZE; row++)//0-10
    {
        for(int col = 0; col < BOARD_GRAD_SIZE; col++)
        {
            if(game->gameMapVec[row][col] == 1)//红方
            {
                QPolygon cell;//六边形
                //每个格子的中心点
                int center_x = MARGIN + (row + 1) * CELL_VER_LENGTH + 2 * col * CELL_VER_LENGTH;
                int center_y = MARGIN + CELL_SIDE_LENGTH + row * CELL_SIDE_LENGTH * 3/2;

                //计算该格子的六个顶点
                for (int i = 0; i < 6; ++i)
                {
                    if(0 == i)
                    {
                        int vertex_x = center_x + CELL_SIDE_LENGTH * cos(M_PI/6);
                        int vertex_y = center_y + CELL_SIDE_LENGTH * sin(M_PI/6);
                        cell << QPoint(vertex_x, vertex_y);
                    }
                    else
                    {
                        int vertex_x = center_x + CELL_SIDE_LENGTH * cos(M_PI/6 + M_PI*i/3);
                        int vertex_y = center_y + CELL_SIDE_LENGTH * sin(M_PI/6 + M_PI*i/3);
                        cell << QPoint(vertex_x, vertex_y);
                    }
                }
                painter.setPen(QPen(Qt::white, 2));
                painter.setBrush(Qt::red);
                painter.drawPolygon(cell);
            }
            else if(game->gameMapVec[row][col] == -1)//蓝方
            {
                QPolygon cell;//六边形
                //每个格子的中心点
                int center_x = MARGIN + (row + 1) * CELL_VER_LENGTH + 2 * col * CELL_VER_LENGTH;
                int center_y = MARGIN + CELL_SIDE_LENGTH + row * CELL_SIDE_LENGTH * 3/2;

                //计算该格子的六个顶点
                for (int i = 0; i < 6; ++i)
                {
                    if(0 == i)
                    {
                        int vertex_x = center_x + CELL_SIDE_LENGTH * cos(M_PI/6);
                        int vertex_y = center_y + CELL_SIDE_LENGTH * sin(M_PI/6);
                        cell << QPoint(vertex_x, vertex_y);
                    }
                    else
                    {
                        int vertex_x = center_x + CELL_SIDE_LENGTH * cos(M_PI/6 + M_PI*i/3);
                        int vertex_y = center_y + CELL_SIDE_LENGTH * sin(M_PI/6 + M_PI*i/3);
                        cell << QPoint(vertex_x, vertex_y);
                    }
                }
                painter.setPen(QPen(Qt::white, 2));
                painter.setBrush(Qt::blue);
                painter.drawPolygon(cell);
            }
        }
    }
}

void MainWindow::isWinAndEndGame()
{
    if(clickPosRow >= 0 && clickPosRow < BOARD_GRAD_SIZE &&
        clickPosCol >= 0 && clickPosCol < BOARD_GRAD_SIZE &&
        (game->gameMapVec[clickPosRow][clickPosCol] == 1 ||
            game->gameMapVec[clickPosRow][clickPosCol] == -1))
    {
        //bool test = game->isWin(clickPosRow, clickPosCol);
        //qDebug()<<test;
        if(game->isWin(clickPosRow,clickPosCol) && game->gameStatus == PLAYING)
        {
            game->gameStatus = WIN;
            //没有胜利音效
            QString str;
            if(game->gameMapVec[clickPosRow][clickPosCol] == 1)
            {
                str = "红方";
            }
            else if(game->gameMapVec[clickPosRow][clickPosCol] == -1)
            {
                str = "蓝方";
            }
            QMessageBox::StandardButton btnValue = QMessageBox::information(this,"游戏结束",str + "胜利!");

            //重置游戏状态，否则容易死循环
            if(btnValue == QMessageBox::Ok)
            {
                if(game_type == MAN)
                {
                    game->startGame(game_type);
                }
                else if(game_type == AI)
                {
                    game->startGame(game_type);
                }
                game->gameStatus = PLAYING;

            }
        }
    }
}

void MainWindow::initGame()
{
    game = new GameModel;
    initManGame();//默认双人，选AI的时候再AI
}

void MainWindow::initManGame()
{
    game_type = MAN;
    game->gameStatus = PLAYING;
    game->startGame(game_type);
    update();//当调用QWidget的update()函数时，它会触发一个重绘事件，使窗口的paintEvent()函数被调用。

}

void MainWindow::initAIGame(bool aiMode)
{
    game_type = AI;
    game->gameStatus = PLAYING;
    //虽然我方AI后手，但是是我在输入对方位置啊，应该也是红方
    //我方AI先手的时候，我方是红方，AI直接下子
    //所以不管先手后手第一下都是红方，mode这个参数没啥用
    setAIFlag(aiMode);//设置AI的颜色
    game->startGame(game_type);
    update();
}

void MainWindow::updatePosition()
{
    // 获取鼠标位置
    QPoint mousePos = QCursor::pos();
    mousePos = mapFromGlobal(mousePos); // 全局坐标
    //qDebug()<<mousePos;
    clickPosRow = -1;
    clickPosCol = -1;

    // 计算对应格子的行和列
    int nearestKey = findNearestKey(mousePos);
    int row = nearestKey / 11 ; // 0-10!
    int col = nearestKey % 11 ;

    //qDebug() << nearestKey;

    QString rowStr = QString::number(row+1);//只有显示的时候+1
    QString colStrA = columnMap.value(col+1);//字母纵坐标
    QString colStrB = QString::number(col+1);//其他地方必须都是0-10!!!!!!!!!!!!

    // 更新棋子坐标的显示
    coordinateLabel->setText("当前位置: " + rowStr + ", " + colStrB + "(" +colStrA + ")");

    //与中心点的距离
    int len = qRound(double((mousePos - centerPointData[nearestKey]).manhattanLength()));
    //qDebug() << len;
    //qDebug()<<(len < POS_OFFSET);

    if(len < POS_OFFSET)
    {
        clickPosKey = nearestKey;
        clickPosRow = row;
        clickPosCol = col;

        //qDebug() << game->gameMapVec[clickPosRow][clickPosCol];

        if(0 == game->gameMapVec[clickPosRow][clickPosCol])
        {
            selectPos = true;
        }
    }
    else//不在合适的位置，比如格子边上
    {
        clickPosRow = -1;//如果不主动改回去的话clickPosRow和clickPosCol一直都会不变。好像对bug没啥用，不过也没有大碍？
        clickPosCol = -1;
        selectPos = false;//这行没变成false的话会导致只要一次为true之后鼠标不点的话怎么移动都是true，这行解决了点击边界就会下子这个bug
    }
    //qDebug() << selectPos;
    update();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);
    updatePosition();//这行有就快，没有就慢
    // 启动定时器，延迟更新操作
    updateTimer->start(10); // 设置延迟时间为10毫秒
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(selectPos == false)
    {
        return;
    }
    else
    {
        //落子之前，把落子标记再设置为false，注意，人机不是通过mouseReleaseEvent的，所以这个不会触发
        selectPos = false;
    }

    if(game_type == MAN)//双人模式
    {
        chessOneByPerson();
    }

    else if(game_type == AI && aiMode == false)//我方AI后手
    {
        chessOneByPerson();
        QTimer::singleShot(AI_THINK_TIME, this, SLOT(chessOneByAI()));
    }

    else if(game_type == AI && aiMode == true)//我方AI先手
    {
        QTimer::singleShot(AI_THINK_TIME, this, SLOT(chessOneByAI()));
        chessOneByPerson();
    }

}

int MainWindow::findNearestKey(const QPoint &mousePos)
{
    std::pair<int, QPoint> nearestItem = {0, QPoint()};
    int minDistance = std::numeric_limits<int>::max();

    for (const auto &item : centerPointData)
    {
        int distance = qRound(double((mousePos - item.second).manhattanLength()));
        if (distance < minDistance)
        {
            minDistance = distance;
            nearestItem = item;
        }
    }

    return nearestItem.first;
}

