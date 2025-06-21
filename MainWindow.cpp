#include "MainWindow.h"
#include "ui_MainWindow.h"
#include"SearchDialog.h"
#include "AddEditUserDialog.h"
#include "AddEditNumberDialog.h"
#include<algorithm>
#include <QMessageBox>
#include <QInputDialog>
#include<QRandomGenerator>
#include"User.h"
#include"PhoneNumber.h"
#include"UserManager.h"
#include"FamilyManager.h"
#include <numeric>
#include <QPrinter>
#include <QPrintDialog>
#include<QFile>
#include<QMenuBar>
#include<QRadioButton>
#include<QTextBrowser>
#include<QDir>
#include <QThread>
#include<QToolBar>
#include<QDesktopServices>
#include<QButtonGroup>
#include<QStandardPaths>
#include<QFileDialog>
#include<QSystemTrayIcon>
#include <algorithm>  // 新增头文件
#include <random>     // 新增头文件

MainWindow::MainWindow(const User& currentUser, QWidget* parent)
    : QMainWindow(parent)
    , currentUser(currentUser)
    , trayIcon(new QSystemTrayIcon(this))
{
    Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());
    ui.setupUi(this);


    // 设置托盘图标
    trayIcon->setIcon(QIcon(":/icons/app_icon.png"));
    trayIcon->show();

    // 初始化模型
    userModel = new QStandardItemModel(this);
    numberModel = new QStandardItemModel(this);

    setupBillingTab();
   
    setupFamilyTab();


    // 设置UI和连接
    setupUI();
    setupConnections();

 
    QString styleSheet =
        "QWidget {"
        "   background-color: #F0F2EB;"  // 浅米色背景(类似纸张)
        "   color: #505A4E;"             // 橄榄灰文字
        "   font-family: '微软雅黑 Light';"
        "   font-size: 13px;"
        "}"
        "QLabel {"
        "   color: #616F57;"             // 青灰文字
        "   font-weight: 450;"
        "   font-size: 0.95em;"
        "   font-smoothing: antialiased;" // 字体平滑
        "}"
        "QTextEdit {"
        "   background-color: #F9FAF4;"  // 更浅的米白
        "   border: 1px solid #D6D9C8;"  // 自然色边框
        "   border-radius: 3px;"
        "   padding: 10px;"
        "   selection-background-color: #b3e5fc;"  // 选中高亮
        "   font-size: 14px;"
        "   box-shadow: 0 1px 2px rgba(0,0,0,0.05);"  // 添加阴影
        "}"
        "QTabWidget::pane {"
        "   border: 1px solid #D6D9C8;"
        "}"
        "QTabBar::tab {"
        "   background: #E4E7D6;"        // 浅橄榄
        "   color: #616F57;"
        "   padding: 12px 20px;"         // 增加间距
        "   border: none;"
        "   border-bottom: 2px solid transparent;"  // 下划线效果
        "   font-weight: 500;"
        "   transition: all 0.3s ease;" // 过渡动画
        "}"
        "QTabBar::tab:selected {"
        "   background: #F0F2EB;"        // 与主背景一致
        "   color: #505A4E;"
        "   border-bottom: 2px solid #3498db;"  // 激活下划线
        "   background-color: rgba(52,152,219,0.1);"  // 半透明背景
        "}"
        "QTabBar::tab:hover {"
        "   background-color: rgba(52,152,219,0.05);"  // 悬停效果
        "}"
        "QScrollBar:vertical {"
        "   width: 10px;"
        "   background: rgba(0,0,0,0.04);"
        "}"
        "QScrollBar::handle:vertical {"
        "   background: rgba(0,0,0,0.2);"
        "   border-radius: 4px;"
        "}"
        "QScrollBar::add-line, QScrollBar::sub-line {"
        "   background: none;"
        "}"
        "QPushButton {"
        "   background-color: #8BA87D;"  // 自然绿按钮
        "   color: #F0F2EB;"             // 与背景同色系文字
        "   border: 1px solid #7A9073;"
        "   border-radius: 4px;"
        "   padding: 8px 16px;"  
        "   min-width: 80px;"
        "   font-weight: 500;"
        "   font-size: 14px;"
        "   letter-spacing: 0.5px;"       // 字间距
        "   box-shadow: 0 2px 4px rgba(52,152,219,0.3);"  // 主题色阴影
        "   transition: all 0.2s ease;"   // 过渡动画
        "}"
        "QPushButton:hover {"
        "   background-color: #7C9D6E;"  // 加深10%
        "   border-color: #6B8E61;"
        "   box-shadow: 0 3px 6px rgba(52,152,219,0.4);"  // 放大阴影
        "   transform: translateY(-1px);" // 上浮效果
        "}"
        "QPushButton:pressed {"
        "   background-color: #6B8E61;"  // 加深20%
        "   box-shadow: 0 1px 2px rgba(52,152,219,0.2);"
        "   transform: translateY(0);"
        "}"
        "QPushButton:focus {"
        "   outline: none;"               // 移除默认焦点框
        "   border: 2px solid #b3e5fc;"    // 柔和聚焦边框
        "}"
        // 成功/危险按钮变体
        "QPushButton.success {"
        "   background-color: #27ae60;"
        "   box-shadow: 0 2px 4px rgba(39,174,96,0.3);"
        "}"
        "QPushButton.danger {"
        "   background-color: #e74c3c;"
        "   box-shadow: 0 2px 4px rgba(231,76,60,0.3);"
        "}";

    this->setStyleSheet(styleSheet);
    // 根据用户角色更新UI
    updateUIByRole();

    connect(&billingSystem, &Billing::familyChargesProcessed,
        this, &MainWindow::onFamilyChargeDistributed);

    // 加载数据
    userManager.loadUsers();
    familyManager.loadFamilies();
    billingSystem.loadBillingData();
    userManager.init(&familyManager);
    familyManager.init(&userManager);

    refreshUserList();
    refreshNumberList();
    refreshFamilyList();


#ifdef QT_DEBUG
    QPushButton* testDataBtn = new QPushButton(tr("生成测试数据"), this);
    ui.horizontalLayout_3->insertWidget(0, testDataBtn);
    connect(testDataBtn, &QPushButton::clicked, this, &MainWindow::onGenerateTestDataClicked);
#endif
    qDebug() << "MainWindow initialized successfully";
}

MainWindow::~MainWindow()
{

}

void MainWindow::setupUI()
{
    // 设置窗口标题
    setWindowTitle(tr("电话计费系统 - 当前用户: %1").arg(currentUser.getUserName()));

    // 初始化用户信息表格，将userModel设置为数据模型
    ui.userTableView->setModel(userModel);
    ui.numberTableView->setModel(numberModel);//numberModel为数据模型

    // 设置表格列标题
    QStringList userHeaders;
    userHeaders << tr("用户名") << tr("角色") << tr("家庭名称") << tr("余额");
    userModel->setHorizontalHeaderLabels(userHeaders);

    QStringList numberHeaders;
    numberHeaders << tr("号码") << tr("类型") << tr("服务类型") << tr("套餐");
    numberModel->setHorizontalHeaderLabels(numberHeaders);

    // 设置表格属性
    ui.userTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.userTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.numberTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.numberTableView->setSelectionMode(QAbstractItemView::SingleSelection);


    // 创建通话记录表格
    callRecordTable = new QTableWidget(this);
    callRecordTable->setColumnCount(6);
    callRecordTable->setHorizontalHeaderLabels({ "时间", "时长(秒)", "类型","家庭通话(家庭内不计费）", "接通状态", "费用(¥)" });
    callRecordTable->verticalHeader()->setVisible(false);
    callRecordTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    callRecordTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 替换原来的callRecordTextEdit为表格
    ui.callRecordTab->layout()->replaceWidget(ui.callRecordTextEdit, callRecordTable);
    delete ui.callRecordTextEdit;

    // 设置表格样式
    callRecordTable->setStyleSheet(R"(
        QTableWidget {
            background: #FFFFFF;
            alternate-background-color: #F8F9FA;
            gridline-color: #EAECEF;
            font: 14px 'Segoe UI';
        }
        QHeaderView::section {
            background: #3498db;
            color: white;
            padding: 10px;
            border: none;
        }
        QTableWidget::item {
            padding: 8px;
            border-bottom: 1px solid #EAECEF;
        }
        QTableWidget::item:selected {
            background: #b3e5fc;
        }
    )");

    // 设置列宽比例
    callRecordTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    callRecordTable->setColumnWidth(1, 100);
    callRecordTable->setColumnWidth(2, 120);
    callRecordTable->setColumnWidth(3, 120);
    callRecordTable->setColumnWidth(4, 120); 
    callRecordTable->setColumnWidth(5, 150);
    ui.tabWidget->setCurrentIndex(0);


    QMenu* familyMenu = menuBar()->addMenu("家庭");
    QAction* familySummaryAction = new QAction("家庭账单汇总", this);
    connect(familySummaryAction, &QAction::triggered, this, &MainWindow::onGenerateFamilyBill);
    familyMenu->addAction(familySummaryAction);

    // 仅家长可见
    familyMenu->menuAction()->setVisible(familyManager.isFamilyParent(familyManager.getUserFamily(currentUser.getUserName()), currentUser.getUserName()));
    
    // 添加查看按钮
    QPushButton* viewUserButton = new QPushButton(tr("查看用户"), this);
    QPushButton* viewNumberButton = new QPushButton(tr("查看号码"), this);

    ui.userManagementTabLayout->addWidget(viewUserButton);
    ui.numberManagementTabLayout->addWidget(viewNumberButton);

    connect(viewUserButton, &QPushButton::clicked, [this]() {
        QModelIndexList selected = ui.userTableView->selectionModel()->selectedRows();
        if (!selected.isEmpty()) {
            QString username = userModel->item(selected.first().row(), 0)->text();
            showUserDetails(username);
        }
        });
    connect(viewNumberButton, &QPushButton::clicked, [this]() {
        QModelIndexList selected = ui.numberTableView->selectionModel()->selectedRows();
        if (!selected.isEmpty()) {
            QString number = numberModel->item(selected.first().row(), 0)->text();
            showCallRecords(number, this);  // 添加this作为父窗口
        }
        });

    setupNumberFilterUI();
    // 创建主工具栏
    mainToolBar = addToolBar(tr("主工具栏"));
    mainToolBar->setObjectName("mainToolBar"); // 设置对象名以便样式表应用

    // 添加搜索按钮
    QPushButton* searchButton = new QPushButton(tr("全局搜索"), this);
    searchButton->setIcon(QIcon(":/icons/search.png"));
    searchButton->setToolTip(tr("点击打开全局搜索界面"));
    mainToolBar->addWidget(searchButton);

    // 可以添加其他工具按钮
    mainToolBar->addSeparator();
    QPushButton* refreshButton = new QPushButton(tr("刷新"), this);
    mainToolBar->addWidget(refreshButton);

    // 连接信号
    connect(searchButton, &QPushButton::clicked, this, &MainWindow::onSearchButtonClicked);
    connect(refreshButton, &QPushButton::clicked, this, [this]() {
        refreshUserList();
        refreshNumberList();
        });
    ui.billingTab->setLayout(new QVBoxLayout());
    QMenu* exportMenu = menuBar()->addMenu("导出报表");
    QAction* fixedLongAction = new QAction("生成固定长途电话文件", this);
    QAction* fixedLocalAction = new QAction("生成固定本地电话文件", this);
    QAction* wirelessLongAction = new QAction("生成无线长途电话文件", this);
    QAction* wirelessLocalAction = new QAction("生成无线本地电话文件", this);
    QAction* wirelessAnswerAction = new QAction("生成无线接听电话文件", this);
    QAction* chargeSummaryAction = new QAction("生成统计电信费用文件", this);

    exportMenu->addAction(fixedLongAction);
    exportMenu->addAction(fixedLocalAction);
    exportMenu->addAction(wirelessLongAction);
    exportMenu->addAction(wirelessLocalAction);
    exportMenu->addAction(wirelessAnswerAction);
    exportMenu->addAction(chargeSummaryAction);

    connect(fixedLongAction, &QAction::triggered, this, &MainWindow::onGenerateFixedLongFile);
    connect(fixedLocalAction, &QAction::triggered, this, &MainWindow::onGenerateFixedLocalFile);
    connect(wirelessLongAction, &QAction::triggered, this, &MainWindow::onGenerateWirelessLongFile);
    connect(wirelessLocalAction, &QAction::triggered, this, &MainWindow::onGenerateWirelessLocalFile);
    connect(wirelessAnswerAction, &QAction::triggered, this, &MainWindow::onGenerateWirelessAnswerFile);
    connect(chargeSummaryAction, &QAction::triggered, this, &MainWindow::onGenerateChargeSummaryFile);

}

void MainWindow::setupConnections()
{
    // 用户管理按钮
    connect(ui.addUserButton, &QPushButton::clicked, this, &MainWindow::onAddUserClicked);
    connect(ui.deleteUserButton, &QPushButton::clicked, this, &MainWindow::onDeleteUserClicked);
    connect(ui.editUserButton, &QPushButton::clicked, this, &MainWindow::onEditUserClicked);

    // 电话号码管理按钮
    connect(ui.addNumberButton, &QPushButton::clicked, this, &MainWindow::onAddNumberClicked);
    connect(ui.deleteNumberButton, &QPushButton::clicked, this, &MainWindow::onDeleteNumberClicked);
    connect(ui.editNumberButton, &QPushButton::clicked, this, &MainWindow::onEditNumberClicked);

    // 其他功能按钮
    connect(ui.queryRecordButton, &QPushButton::clicked, this, &MainWindow::onQueryCallRecordClicked);
    connect(ui.generateBillButton, &QPushButton::clicked, this, &MainWindow::onGenerateBillClicked);
    connect(ui.logoutButton, &QPushButton::clicked, this, &MainWindow::onLogoutClicked);
    connect(ui.aboutButton, &QPushButton::clicked, this, &MainWindow::onAboutClicked);

    
	connect(ui.userTableView, &QTableView::doubleClicked,
		this, &MainWindow::onUserDoubleClicked);
	connect(ui.numberTableView, &QTableView::doubleClicked,
		this, &MainWindow::onNumberDoubleClicked);

    connect(ui.numberTableView->horizontalHeader(), &QHeaderView::sectionClicked,
        [this](int logicalIndex) {
            // 获取当前排序状态
            Qt::SortOrder currentOrder = ui.numberTableView->horizontalHeader()->sortIndicatorOrder();

            // 应用筛选时会保持排序状态，所以这里只需要确保筛选应用即可
            applyNumberFilters();
        });
     
}


void MainWindow::onSearchButtonClicked()
{
    SearchDialog dialog(&userManager, &familyManager, &billingSystem, currentUser, this);
    if (dialog.exec() == QDialog::Accepted) {
        // 退出搜索界面后刷新主窗口列表
        refreshUserList();
        refreshNumberList();
    }
}
void MainWindow::updateUIByRole()
{
    bool isAdmin = currentUser.isAdmin();

    // 根据用户角色显示/隐藏功能
    ui.userManagementTab->setEnabled(isAdmin);
    ui.addNumberButton->setEnabled(currentUser.canAddNumber());

    // 如果是家庭子用户，只能看到自己的号码
    if (currentUser.getRole() == User::FAMILY_CHILD) {
        ui.tabWidget->setTabEnabled(1, false); // 禁用用户管理标签页
    }
}

void MainWindow::refreshUserList()
{
    userModel->removeRows(0, userModel->rowCount());

    if (currentUser.isAdmin())
    { // 管理员可以看到所有用户信息
        QVector<User> users = userManager.getAllUsers();
        for (const User& user : users) {
            // 创建一个QStandardItem对象列表，用于存储当前用户的信息
            QList<QStandardItem*> items;
            items << new QStandardItem(user.getUserName()); // 添加用户名一次

            QString roleStr;
            switch (user.getRole()) {
            case User::ADMIN: roleStr = tr("管理员"); break;
            case User::NORMAL: roleStr = tr("普通用户"); break;
            case User::FAMILY_PARENT: roleStr = tr("家庭户主"); break;
            case User::FAMILY_CHILD: roleStr = tr("家庭子用户"); break;
            }

            items << new QStandardItem(roleStr); // 添加角色
            items << new QStandardItem(familyManager.getUserFamily(user.getUserName())); // 添加家庭名称
            items << new QStandardItem(QString::number(user.getBalance(), 'f', 2)); // 以两位小数形式添加余额
            userModel->appendRow(items); // 将这一行添加到模型中
        }
    }
    else if(currentUser.getRole() == User::FAMILY_PARENT) //看到自己家庭的信息
    {
        QVector<User> users = userManager.getFamilyMembers(familyManager.getUserFamily(currentUser.getUserName()));
        for (const User& user : users) {
            QList<QStandardItem*> items;
         
            items << new QStandardItem(user.getUserName()); // 添加用户名一次

            QString roleStr;
            switch (user.getRole()) {
            case User::ADMIN: roleStr = tr("管理员"); break;
            case User::NORMAL: roleStr = tr("普通用户"); break;
            case User::FAMILY_PARENT: roleStr = tr("家庭户主"); break;
            case User::FAMILY_CHILD: roleStr = tr("家庭子用户"); break;
            }

            items << new QStandardItem(roleStr); // 添加角色
            items << new QStandardItem(familyManager.getUserFamily(user.getUserName())); // 添加家庭名称
            items << new QStandardItem(QString::number(user.getBalance(), 'f', 2)); // 以两位小数形式添加余额
            userModel->appendRow(items); // 将这一行添加到模型中
        }
    }
    else // 其他用户只能看到自己的信息
    {
        QList<QStandardItem*> items;
        items << new QStandardItem(currentUser.getUserName()); // 添加用户名一次

        QString roleStr;
        switch (currentUser.getRole()) {
        case User::ADMIN: roleStr = tr("管理员"); break;
        case User::NORMAL: roleStr = tr("普通用户"); break;
        case User::FAMILY_PARENT: roleStr = tr("家庭户主"); break;
        case User::FAMILY_CHILD: roleStr = tr("家庭子用户"); break;
        }

        items << new QStandardItem(roleStr); // 添加角色
        items << new QStandardItem(familyManager.getUserFamily(currentUser.getUserName())); // 添加家庭名称
        items << new QStandardItem(QString::number(currentUser.getBalance(), 'f', 2)); // 以两位小数形式添加余额
        userModel->appendRow(items); // 将这一行添加到模型中
    
    }
}

void MainWindow::refreshNumberList()
{
   
    // 保存当前排序状态                      //水平表头            排序指示器索引
    int sortColumn = ui.numberTableView->horizontalHeader()->sortIndicatorSection();
    //  枚举成员Qt::SortOrder表示排序顺序，Ascending升序，Descending降序
    Qt::SortOrder sortOrder = ui.numberTableView->horizontalHeader()->sortIndicatorOrder();

    numberModel->removeRows(0, numberModel->rowCount());//清空原有数据

    QList<PhoneNumber> numbers;
    if (currentUser.isAdmin()) {
        QVector<User> users = userManager.getAllUsers();
        for (const User& user : users) {
            numbers.append(user.getPhoneNumbers());
        }
    }
    else {
        numbers = currentUser.getPhoneNumbers();
    }

    for (const PhoneNumber& number : numbers) {
        QList<QStandardItem*> items;
        items << new QStandardItem(number.getNumber());
        items << new QStandardItem(number.getType());
        items << new QStandardItem(number.getServiceType());
        items << new QStandardItem(number.getPlan());
        numberModel->appendRow(items);
    }

    // 恢复排序状态         排序函数
    ui.numberTableView->sortByColumn(sortColumn, sortOrder);
}

void MainWindow::onAddUserClicked()
{
    AddEditUserDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        User newUser = dialog.getUser();
        if (userManager.addUser(newUser)) {
            refreshUserList();
        }
        else {
            showError(tr("添加用户失败，用户名可能已存在"));
        }
    }
}

void MainWindow::onDeleteUserClicked()
{
    QModelIndexList selected = ui.userTableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        showError(tr("请先选择要删除的用户"));
        return;
    }

    QString username = userModel->item(selected.first().row(), 0)->text();
    if (username == currentUser.getUserName()) {
        showError(tr("不能删除当前登录的用户"));
        return;
    }

    if (QMessageBox::question(this, tr("确认删除"),
        tr("确定要删除用户 %1 吗？").arg(username)) == QMessageBox::Yes) {
        if (userManager.deleteUser(username)) {
            refreshUserList();
        }
        else {
            showError(tr("删除用户失败"));
        }
    }
}

void MainWindow::onEditUserClicked()
{
    QModelIndexList selected = ui.userTableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        showError(tr("请先选择要编辑的用户"));
        return;
    }

    QString username = userModel->item(selected.first().row(), 0)->text();
    User user = userManager.getUser(username);
    if (user.getUserName().isEmpty()) {
        showError(tr("获取用户信息失败"));
        return;
    }

    AddEditUserDialog dialog(user, this);
    if (dialog.exec() == QDialog::Accepted) {
        User updatedUser = dialog.getUser();
        if (userManager.updateUser(updatedUser)) {
            refreshUserList();
        }
        else {
            showError(tr("更新用户信息失败"));
        }
    }
}

void MainWindow::onAddNumberClicked()
{
    AddEditNumberDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        PhoneNumber newNumber = dialog.getPhoneNumber();
        currentUser.addNumber(newNumber);
        userManager.updateUser(currentUser);
        refreshNumberList();
    }
}

void MainWindow::onDeleteNumberClicked()
{
    QModelIndexList selected = ui.numberTableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        showError(tr("请先选择要删除的号码"));
        return;
    }

    QString number = numberModel->item(selected.first().row(), 0)->text();
    if (currentUser.removeNumber(number)) {
        userManager.updateUser(currentUser);
        refreshNumberList();
    }
    else {
        showError(tr("删除号码失败，您可能没有权限"));
    }
}

void MainWindow::onEditNumberClicked()
{
    QModelIndexList selected = ui.numberTableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        showError(tr("请先选择要编辑的号码"));
        return;
    }

    QString numberStr = numberModel->item(selected.first().row(), 0)->text();

    // 查找要编辑的号码
    PhoneNumber numberToEdit;
    for (const PhoneNumber& num : currentUser.getPhoneNumbers()) {
        if (num.getNumber() == numberStr) {
            numberToEdit = num;
            break;
        }
    }

    if (!numberToEdit.isValid()) {
        showError(tr("找不到要编辑的号码"));
        return;
    }

    AddEditNumberDialog dialog(numberToEdit, this);
    if (dialog.exec() == QDialog::Accepted) {
        PhoneNumber updatedNumber = dialog.getPhoneNumber();
        if (currentUser.removeNumber(numberStr)) {
            currentUser.addNumber(updatedNumber);
            userManager.updateUser(currentUser);
            refreshNumberList();
        }
        else {
            showError(tr("更新号码信息失败"));
        }
    }
}



void MainWindow::onQueryCallRecordClicked()
{
    bool ok;
    QString number = QInputDialog::getText(this, tr("查询通话记录"),
        tr("请输入要查询的电话号码:"), QLineEdit::Normal,
        QString(), &ok);
    //按取消键退出
    if (!ok)
    {
        return;
    }
    if (number.isEmpty()) {
        QMessageBox::warning(this, tr("输入错误"), tr("电话号码不能为空"));
        return;
    }

     //验证号码格式
    if (!PhoneNumber::validateNumberFormat(number)) {
        QMessageBox::warning(this, tr("格式错误"), tr("无效的电话号码格式"));
        return;
    }

    // 权限验证
    bool hasPermission = false;

    if (currentUser.isAdmin()) {
        // 管理员可以查看所有号码
        hasPermission = true;
    }
    else {
        // 普通用户只能查看自己管理的号码
        if (currentUser.canManageNumber(number)) {
            hasPermission = true;
        }
        // 家庭户主可以查看家庭成员号码
        else if (currentUser.getRole() == User::FAMILY_PARENT) {
            QVector<User> allUsers = userManager.getAllUsers();
            for (const User& user : allUsers) {
                if (familyManager.getUserFamily(user.getUserName()) == familyManager.getUserFamily(currentUser.getUserName()) &&    // 同一家庭
                    user.getPhoneNumbers().contains(number)) {
                    hasPermission = true;
                    break;
                }
            }
        }
    }

    if (!hasPermission) {
        QMessageBox::warning(this, tr("权限不足"),
            tr("您没有权限查看此号码的通话记录"));
        return;
    }

    // 准备表格更新
    QElapsedTimer timer;    // 创建QElapsedTimer对象，用于计算代码执行时间
    timer.start();      // 开始计时
    callRecordTable->setUpdatesEnabled(false); // 禁用UI刷新
    callRecordTable->setSortingEnabled(false); // 禁用排序防止插入时自动排序
    callRecordTable->clearContents();   // 清空原有数据
    callRecordTable->setRowCount(0);    // 设置行数为0

    // 样式常量
    //定义颜色常量
    const QColor longCallColor("#2980b9");
    const QColor localCallColor("#27ae60");
    const QColor connectedColor("#27ae60");
    const QColor notConnectedColor("#e74c3c");
    const QColor chargeColor("#c0392b");
    const QBrush summaryBackground(QColor("#eaf2f8"));

    try {
        // 批量插入数据优化
        Billing::BillingHistory history = billingSystem.getBillingHistory(number);
        callRecordTable->setRowCount(history.Records.size());//设置行数

        // 并行处理数据生成
        for (int row = 0; row < history.Records.size(); ++row) {
            const auto& record = history.Records[row];

            // 预处理服务类型判断
            const bool isLongCall = record.isLongDistance;
            const bool isAnswered = record.answered;
            const bool isFamilyCall = record.isFamilyCall;

            // 时间列
            QTableWidgetItem* timeItem = new QTableWidgetItem(
                record.callTime.toString("yyyy-MM-dd hh:mm:ss"));
            timeItem->setData(Qt::UserRole, record.callTime);

            // 时长列（居中显示）
            QTableWidgetItem* durationItem = new QTableWidgetItem(
                QString::number(record.duration));
            durationItem->setTextAlignment(Qt::AlignCenter);// 居中显示

            // 通话类型列
            QTableWidgetItem* typeItem = new QTableWidgetItem(
                isLongCall ? "📡 长途" : "🏠 本地");
            typeItem->setForeground(isLongCall ? longCallColor : localCallColor);


            // 创建家庭通话列条目
            QTableWidgetItem* familyItem = new QTableWidgetItem(
                isFamilyCall ? "👪 家庭" : "🌍 外部");
            familyItem->setForeground(isFamilyCall ? localCallColor : longCallColor);

            // 接通状态列
            QTableWidgetItem* statusItem = new QTableWidgetItem(
                isAnswered ? "✅ 已接通" : "❌ 未接通");
            statusItem->setForeground(isAnswered ? connectedColor : notConnectedColor);

            // 费用列（右对齐）
            QTableWidgetItem* chargeItem = new QTableWidgetItem(
                QString::number(record.charge, 'f', 2));
            chargeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            chargeItem->setForeground(chargeColor);

            // 批量设置单元格
            callRecordTable->setItem(row, static_cast<int>(CallRecordColumn::Time), timeItem);
            callRecordTable->setItem(row, static_cast<int>(CallRecordColumn::Duration), durationItem);
            callRecordTable->setItem(row, static_cast<int>(CallRecordColumn::Type), typeItem);
            callRecordTable->setItem(row, static_cast<int>(CallRecordColumn::Family), familyItem);
            callRecordTable->setItem(row, static_cast<int>(CallRecordColumn::Status), statusItem);
            callRecordTable->setItem(row, static_cast<int>(CallRecordColumn::Charge), chargeItem);
        }

        // 添加统计信息
        addSummaryRow(history, summaryBackground);
    }
    catch (const std::bad_alloc& e) {
        qCritical() << "Memory allocation failed:" << e.what();
        QMessageBox::critical(this, tr("错误"), tr("系统内存不足，无法显示全部记录"));
    }

    // 恢复UI设置
    callRecordTable->setUpdatesEnabled(true);
    callRecordTable->setSortingEnabled(true);
    callRecordTable->sortByColumn(static_cast<int>(CallRecordColumn::Time), Qt::DescendingOrder);
    callRecordTable->resizeColumnsToContents();

    qDebug() << "Records loaded in" << timer.elapsed() << "ms";
    // 更新用户最后查询时间
    currentUser.setLastUpdate(QDateTime::currentDateTime());
    userManager.updateUser(currentUser);
}


void MainWindow::onUserDoubleClicked(const QModelIndex& index)
{
    if (index.isValid()) {
        QString username = userModel->item(index.row(), 0)->text();
        showUserDetails(username);
    }
}

void MainWindow::onNumberDoubleClicked(const QModelIndex& index)
{
    if (index.isValid()) {
        QString number = numberModel->item(index.row(), 0)->text();
        showCallRecords(number);
    }
}

void MainWindow::showUserDetails(const QString& username)
{
    User user = userManager.getUser(username);
    if (user.getUserName().isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("用户不存在"));
        return;
    }

    // 创建详情对话框（设置为模态对话框）
    QDialog* userDialog = new QDialog(this);
    userDialog->setWindowTitle(tr("用户详情 - %1").arg(username));
    userDialog->resize(700, 500);

    QVBoxLayout* mainLayout = new QVBoxLayout(userDialog);

    // 1. 基本信息部分
    QGroupBox* infoGroup = new QGroupBox(tr("基本信息"), userDialog);
    QFormLayout* infoLayout = new QFormLayout(infoGroup);

    infoLayout->addRow(tr("用户名:"), new QLabel(username));
    infoLayout->addRow(tr("角色:"), new QLabel(
        user.getRole() == User::ADMIN ? "管理员" :
        (user.getRole() == User::FAMILY_PARENT ? "家庭家长" :
            (user.getRole() == User::FAMILY_CHILD ? "家庭子用户" : "普通用户"))));
    infoLayout->addRow(tr("余额:"), new QLabel(QString("¥%1").arg(user.getBalance(), 0, 'f', 2)));

    QString familyName = familyManager.getUserFamily(username);
    infoLayout->addRow(tr("家庭:"), new QLabel(familyName.isEmpty() ? "无" : familyName));

    infoGroup->setLayout(infoLayout);
    mainLayout->addWidget(infoGroup);

    // 2. 电话号码部分（支持双击查看通话记录）
    QGroupBox* numbersGroup = new QGroupBox(tr("电话号码（双击查看详情）"), userDialog);
    QVBoxLayout* numbersLayout = new QVBoxLayout(numbersGroup);

    QTableWidget* numbersTable = new QTableWidget(numbersGroup);
    numbersTable->setColumnCount(4);
    numbersTable->setHorizontalHeaderLabels({ tr("号码"), tr("类型"), tr("服务类型"), tr("套餐") });
    numbersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    numbersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    numbersTable->setSelectionMode(QAbstractItemView::SingleSelection);

    const QList<PhoneNumber>& numbers = user.getPhoneNumbers();
    numbersTable->setRowCount(numbers.size());

    for (int i = 0; i < numbers.size(); ++i) {
        const PhoneNumber& num = numbers[i];

        // 号码列（设置为可点击样式）
        QTableWidgetItem* numberItem = new QTableWidgetItem(num.getNumber());
        numberItem->setData(Qt::UserRole, num.getNumber()); // 存储完整号码
        numberItem->setForeground(Qt::blue);
        numberItem->setToolTip(tr("双击查看此号码的通话记录"));
        numbersTable->setItem(i, 0, numberItem);

        // 其他信息列
        numbersTable->setItem(i, 1, new QTableWidgetItem(num.getType()));
        numbersTable->setItem(i, 2, new QTableWidgetItem(num.getServiceType()));
        numbersTable->setItem(i, 3, new QTableWidgetItem(num.getPlan()));
    }

    // 调整列宽
    numbersTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    numbersTable->resizeColumnsToContents();

    // 连接双击事件
    connect(numbersTable, &QTableWidget::cellDoubleClicked, [this, userDialog](int row, int column) {
        if (column == 0) { // 只响应号码列的点击
            QString number = userDialog->findChild<QTableWidget*>()->item(row, 0)->text();
            this->showCallRecords(number, userDialog); // 传递父对话框
        }
        });

    numbersLayout->addWidget(numbersTable);
    numbersGroup->setLayout(numbersLayout);
    mainLayout->addWidget(numbersGroup);

    // 3. 如果是家庭成员，显示家庭成员信息
    if (!familyName.isEmpty()) {
        QGroupBox* familyGroup = new QGroupBox(tr("家庭成员"), userDialog);
        QVBoxLayout* familyLayout = new QVBoxLayout(familyGroup);

        QTableWidget* familyTable = new QTableWidget(familyGroup);
        familyTable->setColumnCount(3);
        familyTable->setHorizontalHeaderLabels({ tr("用户名"), tr("角色"), tr("手机号数量") });
        familyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

        QVector<QString> members = familyManager.getFamilyMembers(familyName);
        familyTable->setRowCount(members.size());

        for (int i = 0; i < members.size(); ++i) {
            User member = userManager.getUser(members[i]);
            familyTable->setItem(i, 0, new QTableWidgetItem(member.getUserName()));
            familyTable->setItem(i, 1, new QTableWidgetItem(
                familyManager.isFamilyParent(familyName, member.getUserName()) ? "家长" : "孩子"));
            familyTable->setItem(i, 2, new QTableWidgetItem(
                QString::number(member.getPhoneNumbers().size())));
        }

        familyTable->resizeColumnsToContents();
        familyLayout->addWidget(familyTable);
        familyGroup->setLayout(familyLayout);
        mainLayout->addWidget(familyGroup);
    }

    // 添加关闭按钮
    QDialogButtonBox* buttonBox = new QDialogButtonBox(userDialog);
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Close);
    connect(buttonBox, &QDialogButtonBox::rejected, userDialog, &QDialog::reject);

    mainLayout->addWidget(buttonBox);
    userDialog->setLayout(mainLayout);

    // 显示对话框（模态）
    userDialog->exec();

    // 对话框关闭后自动清理内存
    userDialog->deleteLater();
}

void MainWindow::showCallRecords(const QString& number, QWidget* parent)
{

    // 检查权限
    if (!currentUser.isAdmin() && !currentUser.canManageNumber(number)) {
        QMessageBox::warning(this, tr("权限不足"),
            tr("您没有权限查看此号码的通话记录"));
        return;
    }

    // 创建对话框
    QDialog* dialog = new QDialog(parent ? parent : this);
    dialog->setWindowTitle(tr("通话记录 - %1").arg(number));
    dialog->resize(800, 600);

    QVBoxLayout* layout = new QVBoxLayout(dialog);

    // 创建表格
    QTableWidget* table = new QTableWidget(dialog);
    table->setColumnCount(6);
    table->setHorizontalHeaderLabels(
        { tr("时间"), tr("对方号码"), tr("时长(秒)"), tr("类型"), tr("状态"), tr("费用") });
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSortingEnabled(true);

    // 获取通话记录
    Billing::BillingHistory history = billingSystem.getBillingHistory(number);
    table->setRowCount(history.Records.size());

    // 填充数据
    for (int i = 0; i < history.Records.size(); ++i) {
        const Billing::CallRecord& record = history.Records[i];

        // 时间列
        QTableWidgetItem* timeItem = new QTableWidgetItem(
            record.callTime.toString("yyyy-MM-dd hh:mm:ss"));
        timeItem->setData(Qt::UserRole, record.callTime);
        table->setItem(i, 0, timeItem);

        // 对方号码
        table->setItem(i, 1, new QTableWidgetItem(record.number));

        // 时长
        QTableWidgetItem* durationItem = new QTableWidgetItem(
            QString::number(record.duration));
        durationItem->setTextAlignment(Qt::AlignRight);
        table->setItem(i, 2, durationItem);

        // 类型
        QString typeStr = record.isLongDistance ? tr("长途") : tr("本地");
        if (record.isFamilyCall) {
            typeStr += tr("(家庭)");
        }
        table->setItem(i, 3, new QTableWidgetItem(typeStr));

        // 状态
        QTableWidgetItem* statusItem = new QTableWidgetItem(
            record.answered ? tr("接通") : tr("未接"));
        statusItem->setForeground(record.answered ? Qt::darkGreen : Qt::red);
        table->setItem(i, 4, statusItem);

        // 费用
        QTableWidgetItem* chargeItem = new QTableWidgetItem(
            QString::number(record.charge, 'f', 2));
        chargeItem->setTextAlignment(Qt::AlignRight);
        table->setItem(i, 5, chargeItem);
    }

    // 调整列宽
    table->resizeColumnsToContents();

    // 添加总计行
    table->setRowCount(table->rowCount() + 1);
    QTableWidgetItem* totalLabel = new QTableWidgetItem(tr("总计:"));
    totalLabel->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    table->setItem(table->rowCount() - 1, 4, totalLabel);

    QTableWidgetItem* totalValue = new QTableWidgetItem(
        QString::number(history.TotalCharge, 'f', 2));
    totalValue->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    totalValue->setForeground(Qt::blue);
    table->setItem(table->rowCount() - 1, 5, totalValue);

    layout->addWidget(table);

    // 添加关闭按钮
    QPushButton* closeButton = new QPushButton(tr("关闭"), dialog);
    connect(closeButton, &QPushButton::clicked, dialog, &QDialog::close);
    layout->addWidget(closeButton);

    dialog->setLayout(layout);
    dialog->exec();
}


void MainWindow::setupNumberFilterUI()
{
    // 创建筛选控件
    QWidget* filterWidget = new QWidget(this);
    QHBoxLayout* filterLayout = new QHBoxLayout(filterWidget);
    filterLayout->setContentsMargins(0, 0, 0, 0);

    // 服务类型筛选
    QLabel* serviceTypeLabel = new QLabel(tr("服务类型:"), filterWidget);
    serviceTypeFilterCombo = new QComboBox(filterWidget);
    serviceTypeFilterCombo->addItem(tr("所有类型"), "");
    serviceTypeFilterCombo->addItem(tr("主号"), "主号");
    serviceTypeFilterCombo->addItem(tr("副号"), "副号");
    serviceTypeFilterCombo->addItem(tr("家庭号"), "家庭号");

    // 手机类型筛选
    QLabel* phoneTypeLabel = new QLabel(tr("手机类型:"), filterWidget);
    phoneTypeFilterCombo = new QComboBox(filterWidget);
    phoneTypeFilterCombo->addItem(tr("所有类型"), "");
    phoneTypeFilterCombo->addItem(tr("固定"), "固定");
    phoneTypeFilterCombo->addItem(tr("无线"), "无线");

    // 操作按钮
    applyFilterButton = new QPushButton(tr("应用筛选"), filterWidget);
    resetFilterButton = new QPushButton(tr("重置筛选"), filterWidget);

    // 添加到布局
    filterLayout->addWidget(serviceTypeLabel);
    filterLayout->addWidget(serviceTypeFilterCombo);
    filterLayout->addWidget(phoneTypeLabel);
    filterLayout->addWidget(phoneTypeFilterCombo);
    filterLayout->addWidget(applyFilterButton);
    filterLayout->addWidget(resetFilterButton);
    filterLayout->addStretch();

    // 将筛选控件添加到号码管理标签页
    QVBoxLayout* numberLayout = qobject_cast<QVBoxLayout*>(ui.numberManagementTab->layout());
    numberLayout->insertWidget(0, filterWidget);

    // 连接信号
    connect(applyFilterButton, &QPushButton::clicked, this, &MainWindow::applyNumberFilters);
    connect(resetFilterButton, &QPushButton::clicked, this, &MainWindow::resetNumberFilters);

    // 启用表格排序
    ui.numberTableView->setSortingEnabled(true);
}

void MainWindow::applyNumberFilters()
{
    // 获取筛选条件
    QString serviceTypeFilter = serviceTypeFilterCombo->currentData().toString();
    QString phoneTypeFilter = phoneTypeFilterCombo->currentData().toString();

    // 获取所有号码数据
    QList<PhoneNumber> numbers;
    if (currentUser.isAdmin()) {
        QVector<User> users = userManager.getAllUsers();
        for (const User& user : users) {
            numbers.append(user.getPhoneNumbers());
        }
    }
    else {
        numbers = currentUser.getPhoneNumbers();
    }

    // 清空模型
    numberModel->removeRows(0, numberModel->rowCount());

    // 应用筛选
    for (const PhoneNumber& number : numbers) {
        bool matchServiceType = serviceTypeFilter.isEmpty() ||
            (number.getServiceType() == serviceTypeFilter);
        bool matchPhoneType = phoneTypeFilter.isEmpty() ||
            (number.getType() == phoneTypeFilter);

        if (matchServiceType && matchPhoneType) {
            QList<QStandardItem*> items;
            items << new QStandardItem(number.getNumber());
            items << new QStandardItem(number.getType());
            items << new QStandardItem(number.getServiceType());
            items << new QStandardItem(number.getPlan());
            numberModel->appendRow(items);
        }
    }

    // 恢复之前的排序状态
    ui.numberTableView->sortByColumn(ui.numberTableView->horizontalHeader()->sortIndicatorSection(),
        ui.numberTableView->horizontalHeader()->sortIndicatorOrder());
}
void MainWindow::resetNumberFilters()
{
    // 重置筛选条件
    serviceTypeFilterCombo->setCurrentIndex(0);
    phoneTypeFilterCombo->setCurrentIndex(0);

    // 刷新列表
    refreshNumberList();
}


// 新增的统计信息添加函数
void MainWindow::addSummaryRow(const Billing::BillingHistory& history, const QBrush& background)
{
    const int summaryRow = 0;
    callRecordTable->insertRow(summaryRow);

    QTableWidgetItem* summaryItem = new QTableWidgetItem(
        QString("📊 总计：%1 条记录 | 总费用：¥%2")
        .arg(history.Records.size())
        .arg(history.TotalCharge, 0, 'f', 2));

    // 样式设置
    summaryItem->setBackground(background);
    summaryItem->setFlags(summaryItem->flags() & ~Qt::ItemIsSelectable);
    summaryItem->setTextAlignment(Qt::AlignCenter);

    // 字体加粗
    QFont boldFont = summaryItem->font();
    boldFont.setBold(true);
    summaryItem->setFont(boldFont);

    callRecordTable->setItem(summaryRow, 0, summaryItem);
    callRecordTable->setSpan(summaryRow, 0, 1, static_cast<int>(CallRecordColumn::ColumnCount));
}

void MainWindow::onGenerateBillClicked()
{
    QString number = QInputDialog::getText(this, tr("生成账单"),
        tr("请输入要生成账单的电话号码:"));

    if (number.isEmpty()) {
        QMessageBox::warning(this, tr("输入错误"), tr("电话号码不能为空"));
        return;
    }

    // 验证号码格式
    //if (!PhoneNumber::validateNumberFormat(number)) {
    //    QMessageBox::warning(this, tr("格式错误"), tr("无效的电话号码格式"));
    //    return;
    //}

    // 权限验证
    bool hasPermission = false;
    bool isFamilyHead = false;

    if (currentUser.isAdmin()) {
        hasPermission = true; // 管理员可以查看所有账单
    }
    else {
        // 检查是否是号码拥有者
        for (const PhoneNumber& num : currentUser.getPhoneNumbers()) {
            if (num.getNumber() == number) {
                hasPermission = true;
                break;
            }
        }

        // 家庭户主可以查看家庭成员账单
        if (!hasPermission && currentUser.getRole() == User::FAMILY_PARENT) {
            QVector<User> allUsers = userManager.getAllUsers();
            for (const User& user : allUsers) {
                if (familyManager.getUserFamily(user.getUserName()) == familyManager.getUserFamily(currentUser.getUserName())) {
                    for (const PhoneNumber& num : user.getPhoneNumbers()) {
                        if (num.getNumber() == number) {
                            hasPermission = true;
                            isFamilyHead = true;
                            break;
                        }
                    }
                    if (hasPermission) break;
                }
            }
        }
    }

    if (!hasPermission) {
        QMessageBox::warning(this, tr("权限不足"),
            tr("您没有权限生成此号码的账单"));
        return;
    }

    // 获取账单历史
    Billing::BillingHistory history = billingSystem.getBillingHistory(number);
    if (history.Records.isEmpty()) {
        QMessageBox::information(this, tr("账单信息"),
            tr("号码 %1 没有通话记录，账单金额为0").arg(number));
        return;
    }

    // 计算账单周期（最近30天）
    QDateTime now = QDateTime::currentDateTime();
    QDateTime startDate = now.addDays(-30);

    // 筛选本周期内的记录
    QList<Billing::CallRecord> currentPeriodRecords;
    double totalCharge = 0.0;
    int totalDuration = 0;
    int callCount = 0;

    for (const auto& record : history.Records) {
        if (record.callTime >= startDate) {
            currentPeriodRecords.append(record);
            totalCharge += record.charge;
            totalDuration += record.duration;
            callCount++;
        }
    }

    // 按时间倒序排序
    std::sort(currentPeriodRecords.begin(), currentPeriodRecords.end(),
        [](const Billing::CallRecord& a, const Billing::CallRecord& b) {
            return a.callTime > b.callTime;
        });

    // 构建HTML格式账单
    QString htmlBill = "<html><head><style>"
        "body { font-family: Arial, sans-serif; margin: 20px; }"
        "h2 { color: #2c3e50; border-bottom: 2px solid #3498db; padding-bottom: 5px; }"
        "table { width: 100%; border-collapse: collapse; margin: 20px 0; }"
        "h3 { color: #34495e; margin: 15px 0 10px; }"
        ".separator { margin:25px 0; border-top:2px dashed #ecf0f1; }"
        "th { background-color: #3498db; color: white; text-align: left; padding: 8px; }"
        "td { padding: 8px; border-bottom: 1px solid #ddd; }"
        "tr:nth-child(even) { background-color: #f2f2f2; }"
        ".summary { background-color: #eaf2f8; padding: 15px; border-radius: 5px; margin-bottom: 20px; }"
        ".total { font-weight: bold; color: #e74c3c; }"
        "</style></head><body>";

    // 添加账单标题和基本信息
    htmlBill += "<h2>📞 电话账单明细</h2>";
    htmlBill += QString("<div class='summary'>"
        "<p><b>账单号码：</b><span style='color: #c0392b;'>%1</span></p>"
        "<p><b>套餐类型：</b><span style='background:#3498db; color:white; padding:2px 5px; border-radius:3px;'>%2</span></p>"
        "<p><b>计费周期：</b>%3 至 %4</p>"
        "</div>")
        .arg(number)  // %1
        .arg(history.NumberInfo.getPlan())  // %2 (套餐名称)
        .arg(startDate.toString("yyyy-MM-dd"))  // %3
        .arg(now.toString("yyyy-MM-dd"));  // %4

  
    // 添加统计信息（使用更美观的HTML格式）
    htmlBill += "<div class='summary' style='margin-top:15px;'>";
    htmlBill += QString("<h3>消费概览</h3>"
        "<p>总通话次数：<b>%1 次</b></p>"
        "<p>累计时长：<b>%2 分钟</b></p>"
        "<p>总消费金额：<b style='color:#c0392b;'>¥%3</b></p>")
        .arg(callCount)
        .arg(QString::number(totalDuration / 60.0, 'f', 1))
        .arg(QString::number(totalCharge, 'f', 2));
    htmlBill += "</div>";

    // 添加分隔线
    htmlBill += "<div class='separator'></div>";

    // 如果是家庭成员，计算家庭分摊
    if (familyManager.isFamilyMember(currentUser.getUserName())) {
        FamilyManager familyManager;
        familyManager.loadFamilies();
        Family family = familyManager.getFamily(familyManager.getUserFamily(currentUser.getUserName()));

        if (family.contains(currentUser.getUserName())) {
            // 计算家庭总费用
            double familyTotal = family.getFamilyTotalCharge();
            QVector<QPair<QString, double>> distribution = family.getChargeDistribution();

            // 在账单中添加家庭费用信息
            htmlBill += "<div class='family-section'>";
            htmlBill += "<h3>家庭费用分摊</h3>";
            htmlBill += QString("<p>家庭名称: <strong>%1</strong></p>").arg(family.getFamilyName());
            htmlBill += QString("<p>家庭总费用: <strong>¥%1</strong></p>").arg(familyTotal, 0, 'f', 2);

            htmlBill += "<table class='family-table'>";
            htmlBill += "<tr><th>家长</th><th>分摊金额</th></tr>";

            for (const auto& item : distribution) {
                htmlBill += QString("<tr><td>%1</td><td>¥%2</td></tr>")
                    .arg(item.first)
                    .arg(item.second, 0, 'f', 2);
            }

            htmlBill += "</table></div>";
        }
    }

    // 添加通话记录表格
    htmlBill += "<h3 style='margin-top:20px;'>详细通话记录</h3>";
    htmlBill += "<table>"
        "<tr>"
        "<th>序号</th>"
        "<th>通话时间</th>"
        "<th>时长(分钟)</th>"
        "<th>通话类型</th>"
        "<th>是否接通</th>"
        "<th>费率类型</th>"
        "<th>费用(¥)</th>"
        "</tr>";

    // 添加每条通话记录
    for (int i = 0; i < currentPeriodRecords.size(); ++i) {
        const auto& record = currentPeriodRecords[i];
        QString serviceType = record.callerrateStrategy->getCategory();
        QString answerStatus = serviceType.contains("answer") ? "是" : "否";
        QString callType = serviceType.contains("long") ? "长途" : "本地";
        QString rateType = record.callerrateStrategy->getName();
        double minutes = record.duration / 60.0;

        htmlBill += QString("<tr>"
            "<td>%1</td>"
            "<td>%2</td>"
            "<td>%3</td>"
            "<td>%4</td>"
            "<td>%5</td>"
            "<td>%6</td>"
            "<td>%7</td>"
            "</tr>")
            .arg(i + 1)
            .arg(record.callTime.toString("yyyy-MM-dd hh:mm:ss"))
            .arg(QString::number(minutes, 'f', 2))
            .arg(callType)
            .arg(answerStatus)
            .arg(rateType)
            .arg(QString::number(record.charge, 'f', 2));
    }

    htmlBill += "</table></body></html>";

    // 更新界面显示
    ui.billTextEdit->setHtml(htmlBill);
    ui.tabWidget->setCurrentIndex(4); // 切换到账单标签页

    // 生成HTML账单文件
    QString fileName = QString("bill_%1_%2.html")
        .arg(number)
        .arg(now.toString("yyyyMMdd_hhmmss"));
   

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << htmlBill;
        file.close();

        // 显示带超链接的成功消息
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("账单已生成"));
        msgBox.setText(tr("账单已成功生成！"));
        msgBox.setInformativeText(tr("文件已保存到：<a href=\"%1\">%1</a>").arg(QDir::toNativeSeparators(fileName)));
        msgBox.setTextFormat(Qt::RichText);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    }
    else {
        QMessageBox::critical(this, tr("保存失败"),
            tr("无法保存账单文件到：\n%1").arg(fileName));
    }

    // 更新用户最后操作时间
    currentUser.setLastUpdate(now);
    userManager.updateUser(currentUser);

}

void MainWindow::onLogoutClicked()
{
    if (QMessageBox::question(this, tr("退出登录"),
        tr("确定要退出当前账号吗？")) == QMessageBox::Yes) {
        close();
    }
}

void MainWindow::onAboutClicked()
{
    QMessageBox::about(this, tr("关于"),
        tr("电话计费系统\n版本 1.0\n版权所有 © 东北大学计算机科学与工程学院\n开发者：司广威、孙虹宇"));
}

void MainWindow::showError(const QString& message)
{
    QMessageBox::critical(this, tr("错误"), message);
}
void MainWindow::initializeModels()
{
    // 初始化用户表格模型
    userModel = new QStandardItemModel(this);
    userModel->setColumnCount(4); // 用户名、角色、家庭名称、余额
    userModel->setHorizontalHeaderLabels({ tr("用户名"), tr("角色"), tr("家庭名称"), tr("余额") });
    ui.userTableView->setModel(userModel);

    // 设置用户表格属性
    ui.userTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.userTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.userTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui.userTableView->verticalHeader()->setVisible(false);

    // 初始化号码表格模型
    numberModel = new QStandardItemModel(this);
    numberModel->setColumnCount(4); // 号码、类型、服务类型、套餐
    numberModel->setHorizontalHeaderLabels({ tr("号码"), tr("类型"), tr("服务类型"), tr("套餐") });
    ui.numberTableView->setModel(numberModel);

    // 设置号码表格属性
    ui.numberTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.numberTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.numberTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui.numberTableView->verticalHeader()->setVisible(false);

    // 初始化通话记录和账单显示区域
    ui.callRecordTextEdit->setReadOnly(true);
    ui.billTextEdit->setReadOnly(true);

    // 设置表格不可编辑（通过对话框编辑）
    ui.userTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui.numberTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}


// 测试数据生成实现
void MainWindow::onGenerateTestDataClicked()
{
    // 清空现有测试数据
    userManager.clearAllUsers();
    familyManager.clearAllFamilies();
    billingSystem.clearBillingData(); // 重置计费系统
    userManager.init(&familyManager);
    familyManager.init(&userManager); // 重置家庭管理器

    // 1. 随机创建用户
    const int userCount = QRandomGenerator::global()->bounded(10, 21);
    QVector<User> testUsers;
    QStringList familyNames = {
        "和谐之家", "幸福家庭", "科技世家", "书香门第",
        "阳光家园", "绿色家庭", "快乐家族", "智慧之家",
        "星辰家族", "传统之家", "未来家族", "精英世家",
        "温馨小筑", "成长乐园", "爱心港湾"
    };

    for (int i = 0; i < userCount; i++) {
        QString username = QString("user%1").arg(i + 1);
        QString password = "123";

        // 随机分配角色
        User::Role role = User::NORMAL;
        if (i == 0) {
            role = User::ADMIN; // 第一个用户固定为管理员
        }

        User user(username, password, role);

        // 随机分配家庭
        if (role == User::FAMILY_PARENT || role == User::FAMILY_CHILD) {
            QString familyName = familyNames[QRandomGenerator::global()->bounded(familyNames.size())];
        }

        // 随机余额
        user.setBalance(QRandomGenerator::global()->bounded(5000, 50001) / 100.0);
        testUsers.append(user);
    }

    // 2. 为每个用户随机添加电话号码
    QStringList plans = { "白金套餐", "家庭套餐", "标准套餐", "经济套餐" };
    QStringList serviceTypes = { "无线", "固定" };

    for (User& user : testUsers) {
        int numberCount = QRandomGenerator::global()->bounded(1, 3); // 1-2个号码
        for (int j = 0; j < numberCount; j++) {
            // 生成随机手机号
            QString number = "1" +
                QString::number(QRandomGenerator::global()->bounded(3, 10)) +
                QString::number(QRandomGenerator::global()->bounded(100000000, 999999999));

            // 随机号码类型
            PhoneNumber::Type type = static_cast<PhoneNumber::Type>(
                QRandomGenerator::global()->bounded(3));

            // 随机服务类型
            PhoneNumber::ServiceType serviceType = static_cast<PhoneNumber::ServiceType>(
                QRandomGenerator::global()->bounded(2));

            // 随机套餐
            QString plan = plans[QRandomGenerator::global()->bounded(plans.size())];

            user.addNumber(PhoneNumber(number, type, serviceType, plan));
        }
        userManager.addUser(user);

    }
    userManager.saveUsers();
    familyManager.setUserManager(&userManager);
    // 3. 随机创建家庭
// 初始化可用用户列表
    QStringList availableUsers;
    for (int i = 1; i <= userCount; ++i) {
        availableUsers << "user" + QString::number(i);
    }

    // 打乱用户顺序保证随机性
    std::random_device rd;
    std::default_random_engine rng(rd());
    std::shuffle(availableUsers.begin(), availableUsers.end(), rng);

    // 保证家庭数量不超过可用用户和家庭名数量
    int maxFamilies = qMin(availableUsers.size(), familyNames.size());
    int familyCount = QRandomGenerator::global()->bounded(1, maxFamilies + 1);
    int actualFamilyCount = 0;

    for (int i = 0; i < familyCount && !availableUsers.isEmpty(); ++i) {
        // 随机选家庭名
        int nameIndex = QRandomGenerator::global()->bounded(familyNames.size());
        QString familyName = familyNames.takeAt(nameIndex);

        // 选取创建者
        QString creator = availableUsers.takeFirst();

        if (familyManager.createFamily(familyName, creator)) {
            actualFamilyCount++;
            try {


                //随机创建时间
// 获取当前日期
                QDate currentDate = QDate::currentDate();

                // 设置时间范围为最近5年（含今天）
                QDate startDate = currentDate.addYears(-5);  // 5年前今天
                int daysRange = startDate.daysTo(currentDate);  // 计算总天数跨度

                // 生成随机天数（包含今天）
                QDate creationTime = startDate.addDays(QRandomGenerator::global()->bounded(daysRange + 1));
                familyManager.getFamily(familyName).setCreateTime(creationTime);

                User& creatorUser = userManager.getUser(creator);
                creatorUser.setRole(User::FAMILY_PARENT);
            }
            catch (const std::exception& e) {
                qWarning() << "家庭创建者角色设置失败：" << creator
                    << "错误：" << e.what();
                continue;  // 跳过无效用户或记录日志后继续
            }


            // 计算可添加的最大成员数
            int maxMembers = QRandomGenerator::global()->bounded(0, availableUsers.size() + 1);
            for (int j = 0; j < maxMembers; ++j) {
                QString member = availableUsers.takeFirst();
                bool isParent = QRandomGenerator::global()->bounded(100) < 30;
                // 更新成员角色
                try {
                    User& memberUser = userManager.getUser(member);
                    memberUser.setRole(isParent ? User::FAMILY_PARENT : User::FAMILY_CHILD);

                }
                catch (const std::out_of_range& e) {
                    qWarning() << "家庭成员不存在:" << member;
                    continue; // 跳过无效用户
                }
                familyManager.addMember(familyName, member, isParent);
            }
        }
    }
    familyManager.saveFamilies();


    // 4. 生成随机通话记录 - 完全符合CallRecord结构体
    int recordCount = QRandomGenerator::global()->bounded(150, 301); // 150-300条记录
    QDateTime baseTime = QDateTime::currentDateTime().addMonths(-1); // 从一个月前开始
    QStringList allNumbers = getAllPhoneNumbers();



    for (int i = 0; i < recordCount; i++) {
        Billing::CallRecord record;

        // 随机选择主叫号码（来自系统内）
        record.callerNumber = allNumbers[QRandomGenerator::global()->bounded(allNumbers.size())];

        // 获取主叫用户的所有号码
        User calleruser = findUserByNumber(record.callerNumber);
        QList<PhoneNumber> callerNumbers = calleruser.getPhoneNumbers();

        // 确保被叫号码是系统内的其他号码（可以是同用户的其他号码或他人号码）
        QStringList availableCalleeNumbers = allNumbers;
        availableCalleeNumbers.removeAll(record.callerNumber); // 排除自己当前主叫号码

        // 随机选择被叫号码（必须来自系统内）
        if (!availableCalleeNumbers.isEmpty()) {
            record.number = availableCalleeNumbers[QRandomGenerator::global()->bounded(availableCalleeNumbers.size())];

            // 判断是否是家庭内部通话
            QString callerFamily = familyManager.getUserFamily(calleruser.getUserName());
            QString calleeFamily = familyManager.getUserFamily(userManager.getUserByNumber(record.number).getUserName());
            bool isActuallyFamily = (!callerFamily.isEmpty() && callerFamily == calleeFamily);
            record.isFamilyCall = isActuallyFamily && (QRandomGenerator::global()->bounded(100) < 10); // 10%概率
        }
        else {
            // 当系统只有一个号码时，强制生成无效通话（持续时间0）
            record.number = record.callerNumber; // 此时会触发通话失败逻辑
            record.answered = false;
        }



        // 随机通话时间（过去30天内）
        record.callTime = baseTime.addSecs(QRandomGenerator::global()->bounded(2592000)); // 30天=2,592,000秒

        // 80%接通率
        record.answered = QRandomGenerator::global()->bounded(100) < 80;

        if (record.answered) {
            // 随机通话时长（10-600秒）
            record.duration = QRandomGenerator::global()->bounded(10, 601);
        }
        else {
            record.duration = 0;
        }

        //30%长途通话
        record.isLongDistance = QRandomGenerator::global()->bounded(100) < 30;


        // 随机套餐和服务类型（根据主叫号码的用户）
        User callerUser = findUserByNumber(record.callerNumber);
        if (!callerUser.getUserName().isEmpty()) {
            const QList<PhoneNumber>& numbers = callerUser.getPhoneNumbers();
            if (!numbers.isEmpty()) {
                // 找到匹配的号码对象
                for (const PhoneNumber& num : numbers) {
                    if (num.getNumber() == record.callerNumber) {
                        record.plan = num.getPlan();
                        record.serviceType = num.getServiceTypeEnum();
                        break;
                    }
                }
            }
            else {
                record.plan = plans[QRandomGenerator::global()->bounded(plans.size())];

                QString t = serviceTypes[QRandomGenerator::global()->bounded(serviceTypes.size())];
                if (t == "无线")
                    record.serviceType = PhoneNumber::WIRELESS;
                else if (t == "固定")
                    record.serviceType = PhoneNumber::FIXED;
            }
        }
        else {
            record.plan = plans[QRandomGenerator::global()->bounded(plans.size())];
            QString t = serviceTypes[QRandomGenerator::global()->bounded(serviceTypes.size())];
            if (t == "无线")
                record.serviceType = PhoneNumber::WIRELESS;
            else if (t == "固定")
                record.serviceType = PhoneNumber::FIXED;
        }

        record.callerrateStrategy = RateStrategyFactory::createStrategy(record.serviceType, record.isLongDistance);
        record.answerrateStrategy = RateStrategyFactory::createanswerStrategy(record.serviceType, record.answered);

        if (record.answerrateStrategy != nullptr && record.answerrateStrategy->getName() == "无线接受电话") {
            record.wirelessanwerscharge = record.answerrateStrategy->calculateCharge(record.duration, record.plan);
        }
        else { record.wirelessanwerscharge = 0; }

        record.charge = record.callerrateStrategy->calculateCharge(record.duration, record.plan);

        // 处理通话记录
        billingSystem.processCallRecords(record);
    }

    // 5. 处理家庭费用
   // 5. 处理家庭费用（修正版）
    for (auto& history : billingSystem.getAllBillingHistory()) {
        for (auto& record : history->Records) {


            //  获取主叫用户信息
            User callerUser = userManager.getUserByNumber(record.callerNumber);
            if (callerUser.getUserName().isEmpty()) {
                qWarning() << "主叫号码无匹配用户:" << record.callerNumber;
                continue;
            }


            //如果是孩子外呼
            if (callerUser.getRole() == User::FAMILY_CHILD) {
                QString familyName = familyManager.getUserFamily(callerUser.getUserName());
                double totalCharge = record.charge;
                familyManager.addFamilyTotalCharge(familyName, totalCharge);
                // 家长分摊逻辑
                auto parents = familyManager.getFamilyParents(familyName);
                if (!parents.empty()) {
                    double parentShare = totalCharge / parents.size();
                    for (const auto& parent : parents) {
                        User& parentUser = userManager.getUser(parent);
                        parentUser.setFamilyChargeShare(parentUser.getFamilyChargeShare() + parentShare);
                    }
                }
            }

            //如果是家庭家长
            else if (callerUser.getRole() == User::FAMILY_PARENT)
            {
                familyManager.addFamilyTotalCharge(familyManager.getUserFamily(callerUser.getUserName()), record.charge);
            }
        }

    }
    




    // 保存所有数据
    userManager.saveUsers();
    familyManager.saveFamilies();
    billingSystem.saveBillingData();

    // 刷新UI
    refreshUserList();
    refreshNumberList();
    refreshFamilyList();

    QMessageBox::information(this, tr("测试数据生成"),
        tr("已生成:\n- %1个测试用户\n- %2个家庭\n- %3条通话记录")
        .arg(userCount)
        .arg(actualFamilyCount)
        .arg(recordCount));
}

// 辅助函数：获取系统中所有电话号码
QStringList MainWindow::getAllPhoneNumbers()
{
    QStringList numbers;
    for (const User& user : userManager.getAllUsers()) {
        QList<PhoneNumber> userNumbers = user.getPhoneNumbers();
        for (const PhoneNumber& number : userNumbers) {
            numbers.append(number.getNumber());
        }
    }
    return numbers;
}

// 辅助函数：根据号码查找用户
User MainWindow::findUserByNumber(const QString& number)
{
    for (const User& user : userManager.getAllUsers()) {
        for (const PhoneNumber& num : user.getPhoneNumbers()) {
            if (num.getNumber() == number) {
                return user;
            }
        }
    }
    return User(); // 返回空用户
}

void MainWindow::askToShowOrSave(const QString& html, const QString& title) {
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("报告生成");
    msgBox.setText("报告已生成，您想如何操作？");

    // 使用addButton替代setButtonText
    QPushButton* saveButton = msgBox.addButton("保存到文件", QMessageBox::AcceptRole);
    //QPushButton* openButton = msgBox.addButton("直接查看", QMessageBox::ActionRole);
    QPushButton* cancelButton = qobject_cast<QPushButton*>(msgBox.addButton(QMessageBox::Cancel));

    // 设置默认按钮为保存按钮
    msgBox.setDefaultButton(saveButton);

    // 显示对话框并获取点击的按钮
    msgBox.exec();
    QAbstractButton* clickedButton = msgBox.clickedButton();

    // 处理按钮点击
    if (clickedButton == saveButton) {
        QString fileName = QFileDialog::getSaveFileName(this, "保存报告",
            QDir::homePath() + "/" + title + ".html",
            "HTML 文件 (*.html)");
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out << html;
                file.close();
                QMessageBox::information(this, "成功", "报告已保存到: " + fileName);
            }
            else {
                QMessageBox::critical(this, "错误", "无法保存文件: " + fileName);
            }
        }
    }
 
    if (clickedButton == cancelButton) {
        // 取消操作无需处理
    }
}


// 生成固定长途电话文件
void MainWindow::onGenerateFixedLongFile() {
    QString html = generateCallReportHtml("fixed_long", "固定长途电话报告", "固定长途电话");
    askToShowOrSave(html, "固定长途电话报告");
}

// 生成固定本地电话文件
void MainWindow::onGenerateFixedLocalFile() {
    QString html = generateCallReportHtml("fixed_local", "固定本地电话报告", "固定本地电话");
    askToShowOrSave(html, "固定本地电话报告");
}

// 生成无线长途电话文件
void MainWindow::onGenerateWirelessLongFile() {
    QString html = generateCallReportHtml("wireless_long", "无线长途电话报告", "无线长途电话");
    askToShowOrSave(html, "无线长途电话报告");
}

// 生成无线本地电话文件
void MainWindow::onGenerateWirelessLocalFile() {
    QString html = generateCallReportHtml("wireless_local", "无线本地电话报告", "无线本地电话");
    askToShowOrSave(html, "无线本地电话报告");
}

// 生成无线接听电话文件
void MainWindow::onGenerateWirelessAnswerFile() {
    QString html = generateCallReportHtml("wireless_answer", "无线接听电话报告", "无线接听电话");
    askToShowOrSave(html, "无线接听电话报告");
}

// 生成统计电信费用文件
void MainWindow::onGenerateChargeSummaryFile() {
    QString fileName = QFileDialog::getSaveFileName(this, "保存电信费用统计文件",
        QDir::homePath(),
        "HTML 文件 (*.html)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法创建文件: " + fileName);
        return;
    }

    QTextStream out(&file);
    out << generateChargeSummaryHtml();

    file.close();
    QMessageBox::information(this, "成功", "电信费用统计文件已生成: " + fileName);
}

// 通用的通话记录生成函数
void MainWindow::generateCallReport(const QString& category, const QString& reportTitle, const QString& callType) {
    QString fileName = QFileDialog::getSaveFileName(this, "保存" + reportTitle,
        QDir::homePath(),
        "HTML 文件 (*.html)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法创建文件: " + fileName);
        return;
    }

    QTextStream out(&file);
    out << generateCallReportHtml(category, reportTitle, callType);

    file.close();
    QMessageBox::information(this, "成功", reportTitle + "已生成: " + fileName);
}

// 生成通话记录HTML报告
QString MainWindow::generateCallReportHtml(const QString& category, const QString& reportTitle, const QString& callType) {
    QString html;
    QTextStream stream(&html);

    // 获取所有通话记录
    auto allhistory = billingSystem.getAllBillingHistory();
    // 筛选符合条件的记录
    QList<Billing::CallRecord> filteredRecords;
	if (category == "wireless_answer")
		for (const auto& history : allhistory) {
			for (const auto& record : history->Records)
				if (record.answerrateStrategy!=nullptr&&record.answerrateStrategy->getCategory() == category) {
					filteredRecords.append(record);
				}
		}
    else
        for (const auto& history : allhistory) {
            for (const auto& record : history->Records)
                if (record.callerrateStrategy->getCategory() == category) {
                    filteredRecords.append(record);
                }
        }

    // 生成HTML头部
    stream << "<!DOCTYPE html>\n"
        << "<html>\n"
        << "<head>\n"
        << "<meta charset=\"UTF-8\">\n"
        << "<title>" << reportTitle << "</title>\n"
        << "<style>\n"
        << "body { font-family: Arial, sans-serif; margin: 20px; }\n"
        << "h1 { color: #2c3e50; border-bottom: 2px solid #3498db; padding-bottom: 10px; }\n"
        << "table { width: 100%; border-collapse: collapse; margin-top: 20px; }\n"
        << "th { background-color: #3498db; color: white; text-align: left; padding: 10px; }\n"
        << "td { padding: 8px; border-bottom: 1px solid #ddd; }\n"
        << "tr:nth-child(even) { background-color: #f2f2f2; }\n"
        << ".summary { background-color: #eaf2f8; padding: 15px; border-radius: 5px; margin-bottom: 20px; }\n"
        << ".total { font-weight: bold; color: #e74c3c; }\n"
        << "</style>\n"
        << "</head>\n"
        << "<body>\n"
        << "<h1>" << reportTitle << "</h1>\n"
        << "<div class=\"summary\">\n"
        << "<p><strong>报告类型：</strong>" << callType << "</p>\n"
        << "<p><strong>记录总数：</strong>" << filteredRecords.size() << " 条</p>\n"
        << "<p><strong>生成时间：</strong>" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "</p>\n"
        << "</div>\n"
        << "<table>\n"
        << "<tr><th>序号</th><th>电话号码</th><th>通话时间</th><th>时长(秒)</th><th>费用(¥)</th></tr>\n";

    // 添加记录行
	if (category == "wireless_answer")
		for (int i = 0; i < filteredRecords.size(); ++i) {
			const auto& record = filteredRecords[i];
			stream << "<tr>\n"
				<< "<td>" << i + 1 << "</td>\n"
				<< "<td>" << record.number << "</td>\n"
				<< "<td>" << record.callTime.toString("yyyy-MM-dd hh:mm:ss") << "</td>\n"
				<< "<td>" << record.duration << "</td>\n"
				<< "<td>" << QString::number(record.wirelessanwerscharge, 'f', 2) << "</td>\n"
				<< "</tr>\n";
		}
	else
		for (int i = 0; i < filteredRecords.size(); ++i) {
			const auto& record = filteredRecords[i];
			stream << "<tr>\n"
				<< "<td>" << i + 1 << "</td>\n"
				<< "<td>" << record.callerNumber << "</td>\n"
				<< "<td>" << record.callTime.toString("yyyy-MM-dd hh:mm:ss") << "</td>\n"
				<< "<td>" << record.duration << "</td>\n"
				<< "<td>" << QString::number(record.charge, 'f', 2) << "</td>\n"
				<< "</tr>\n";
		}
    // 计算总计    
    double totalCharge = 0.0;
    int totalDuration = 0;
    if(category == "wireless_answer")
    for (const auto& record : filteredRecords) {
        totalCharge += record.wirelessanwerscharge;
        totalDuration += record.duration;
    }
    else
    for (const auto& record : filteredRecords) {
        totalCharge += record.charge;
        totalDuration += record.duration;
    }

    // 添加总计行
    stream << "<tr class=\"total\">\n"
        << "<td colspan=\"3\">总计</td>\n"
        << "<td>" << totalDuration << " 秒 (" << (totalDuration / 60.0) << " 分钟)</td>\n"
        << "<td>" << QString::number(totalCharge, 'f', 2) << "</td>\n"
        << "</tr>\n";

    stream << "</table>\n"
        << "</body>\n"
        << "</html>\n";

    return html;
}

// 生成费用统计HTML报告
QString MainWindow::generateChargeSummaryHtml() {
    QString html;
    // 创建一个QTextStream对象stream，并将其绑定到html字符串上，方便向html字符串中写入内容
    QTextStream stream(&html);

    // 获取所有通话记录
    QMap<QString, std::shared_ptr<Billing::BillingHistory>> allHistory = billingSystem.getAllBillingHistory();

    // 按电话号码分组统计
    QMap<QString, double> chargeByNumber;
    for (const auto& historyPair : allHistory) {
        for (const auto& record : historyPair->Records) {
            chargeByNumber[record.callerNumber] += record.charge;
        }
    }

    // 生成HTML头部
    stream << "<!DOCTYPE html>\n"
        << "<html>\n"
        << "<head>\n"
        << "<meta charset=\"UTF-8\">\n"
        << "<title>电信费用统计报告</title>\n"
        << "<style>\n"
        // 设置整个页面的字体、外边距等基础样式
        << "body { font-family: Arial, sans-serif; margin: 20px; }\n"
        // 设置标题h1的颜色、底部边框、内边距等样式
        << "h1 { color: #2c3e50; border-bottom: 2px solid #3498db; padding-bottom: 10px; }\n"
        // 设置表格的整体样式，包括宽度、边框合并、上边距等
        << "table { width: 100%; border-collapse: collapse; margin-top: 20px; }\n"
        //设置表头单元格的背景颜色、文字颜色、对齐方式、内边距等样式
        << "th { background-color: #3498db; color: white; text-align: left; padding: 10px; }\n"
        << "td { padding: 8px; border-bottom: 1px solid #ddd; }\n"
        << "tr:nth-child(even) { background-color: #f2f2f2; }\n"
        << ".summary { background-color: #eaf2f8; padding: 15px; border-radius: 5px; margin-bottom: 20px; }\n"
        << ".total { font-weight: bold; color: #e74c3c; }\n"
        << "</style>\n"
        << "</head>\n"
        << "<body>\n"
        << "<h1>电信费用统计报告</h1>\n"
        << "<div class=\"summary\">\n"
        << "<p><strong>报告类型：</strong>电信费用统计</p>\n"
        << "<p><strong>统计号码数：</strong>" << chargeByNumber.size() << " 个</p>\n"
        << "<p><strong>生成时间：</strong>" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "</p>\n"
        << "</div>\n"
        << "<table>\n"
        << "<tr><th>序号</th><th>电话号码</th><th>累计费用(¥)</th></tr>\n";

    // 添加记录行
    int i = 0;
    double grandTotal = 0.0;// 累计总费用
    for (auto it = chargeByNumber.begin(); it != chargeByNumber.end(); ++it) {
        stream << "<tr>\n"
            << "<td>" << ++i << "</td>\n"  // 序号列，值为i自增后的值
            << "<td>" << it.key() << "</td>\n"  // 电话号码列，值为当前键值对的键（即电话号码）
            << "<td>" << QString::number(it.value(), 'f', 2) << "</td>\n" // 累计费用列，值为当前键值对的值（即累计费用），使用QString::number函数将double类型转换为字符串，并保留两位小数
            << "</tr>\n";
        grandTotal += it.value();
    }

    // 添加总计行
    stream << "<tr class=\"total\">\n"
        << "<td colspan=\"2\">总计</td>\n"
        << "<td>" << QString::number(grandTotal, 'f', 2) << "</td>\n"
        << "</tr>\n";

    stream << "</table>\n"
        << "</body>\n"
        << "</html>\n";

    return html;
}
