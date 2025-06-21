#include"MainWindow.h"
#include "ui_MainWindow.h"
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
#include<QFile>
#include<QDir>
#include <QThread>
#include<QSystemTrayIcon>
#include<QFileDialog>
#include<QDesktopServices>
#include<QUrl>



void MainWindow::setupFamilyTab() {
    // 初始化模型
    familyModel = new QStandardItemModel(this);
    familyModel->setHorizontalHeaderLabels({ "家庭名称", "家长数量", "孩子数量", "总费用", "创建时间" });
    ui.familyTableView->setModel(familyModel);
    ui.familyTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.familyTableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    memberModel = new QStandardItemModel(this);
    memberModel->setHorizontalHeaderLabels({ "用户名", "角色", "当前分摊" }); // 移除了"加入时间"
    ui.memberTableView->setModel(memberModel);
    ui.memberTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.memberTableView->setSelectionBehavior(QAbstractItemView::SelectRows);


    // 连接信号槽
    connect(ui.createFamilyButton, &QPushButton::clicked, this, &MainWindow::onCreateFamily);
    connect(ui.disbandFamilyButton, &QPushButton::clicked, this, &MainWindow::onDisbandFamily);
    connect(ui.addMemberButton, &QPushButton::clicked, this, &MainWindow::onAddMember);
    connect(ui.removeMemberButton, &QPushButton::clicked, this, &MainWindow::onRemoveMember);
    connect(ui.promoteButton, &QPushButton::clicked, this, &MainWindow::onPromoteMember);
    connect(ui.demoteButton, &QPushButton::clicked, this, &MainWindow::onDemoteMember);

    connect(ui.familyTableView->selectionModel(), &QItemSelectionModel::selectionChanged,
        this, &MainWindow::onFamilySelectionChanged);

}



void MainWindow::refreshFamilyList() {
    familyModel->removeRows(0, familyModel->rowCount());

    familyManager.loadFamilies();
    auto families = familyManager.getAllFamilies();
    for (const Family& family : families) {
        QList<QStandardItem*> items;
        items << new QStandardItem(family.getFamilyName());
        items << new QStandardItem(QString::number(family.getParents().size()));
        items << new QStandardItem(QString::number(family.getChildren().size()));
        items << new QStandardItem(QString::number(familyManager.getFamilyTotalCharge(family.getFamilyName()), 'f', 2));
        QDate date = family.getCreateTime();
        QString time = family.getCreateTime().toString("yyyy-MM-dd");
  
        items << new QStandardItem(family.getCreateTime().toString("yyyy-MM-dd"));
        familyModel->appendRow(items);
    }

    // 自动选择第一行
    if (familyModel->rowCount() > 0) {
        ui.familyTableView->selectRow(0);
    }


}

void MainWindow::refreshMemberList(const QString& familyName) {
    memberModel->removeRows(0, memberModel->rowCount());
    currentFamily = familyName;

    if (familyName.isEmpty()) return;

    auto parents = userManager.getFamilyParents(familyName);
    auto children = userManager.getFamilyChildren(familyName);

    // 添加家长（现在只有3列）
    for (const User& user : parents) {
        QList<QStandardItem*> items;
        items << new QStandardItem(user.getUserName());
        items << new QStandardItem("家长");
        items << new QStandardItem(QString::number(user.getFamilyChargeShare(), 'f', 2));
        memberModel->appendRow(items);
    }

    // 添加孩子（现在只有3列）
    for (const User& user : children) {
        QList<QStandardItem*> items;
        items << new QStandardItem(user.getUserName());
        items << new QStandardItem("孩子");
        items << new QStandardItem("0.00"); // 孩子不分摊费用
        memberModel->appendRow(items);
    }
}


void MainWindow::onFamilySelectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
    Q_UNUSED(deselected);

    if (selected.indexes().isEmpty()) {
        refreshMemberList("");
        return;
    }

    int row = selected.indexes().first().row();
    QString familyName = familyModel->item(row, 0)->text();
    refreshMemberList(familyName);
}

void MainWindow::onCreateFamily() {
    bool ok;
    QString familyName = QInputDialog::getText(this, tr("创建家庭"),
        tr("请输入家庭名称:"),
        QLineEdit::Normal, "", &ok);
    if (!ok || familyName.isEmpty()) return;


    if (familyManager.createFamily(familyName, currentUser.getUserName())) {
        refreshFamilyList();
        QMessageBox::information(this, tr("成功"), tr("家庭创建成功!"));
    }
    else {
        QMessageBox::warning(this, tr("错误"), tr("家庭创建失败，可能名称已存在"));
    }
}

void MainWindow::onDisbandFamily() {
    if (currentFamily.isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("请先选择一个家庭"));
        return;
    }

    if (QMessageBox::question(this, tr("确认解散"),
        tr("确定要解散家庭 %1 吗？此操作不可撤销！").arg(currentFamily))
        != QMessageBox::Yes) {
        return;
    }

    if (familyManager.disbandFamily(currentFamily)) {
        refreshFamilyList();
        refreshMemberList("");
        QMessageBox::information(this, tr("成功"), tr("家庭已解散"));
    }
    else {
        QMessageBox::warning(this, tr("错误"), tr("解散家庭失败"));
    }
}

void MainWindow::onAddMember() {
    if (currentFamily.isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("请先选择一个家庭"));
        return;
    }

    bool ok;
    QString username = QInputDialog::getText(this, tr("添加成员"),
        tr("请输入要添加的用户名:"),
        QLineEdit::Normal, "", &ok);
    if (!ok || username.isEmpty()) return;

    bool asParent = (QMessageBox::question(this, tr("设置角色"),
        tr("将该成员设为家长吗？"),
        QMessageBox::Yes | QMessageBox::No)
        == QMessageBox::Yes);


    if (familyManager.addMember(currentFamily, username, asParent)) {
        refreshMemberList(currentFamily);
        QMessageBox::information(this, tr("成功"), tr("成员添加成功!"));
    }
    else {
        QMessageBox::warning(this, tr("错误"), tr("添加成员失败，可能用户不存在或已在家庭中"));
    }
}

void MainWindow::onRemoveMember() {
    if (currentFamily.isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("请先选择一个家庭"));
        return;
    }

    int row = ui.memberTableView->currentIndex().row();
    if (row < 0) {
        QMessageBox::warning(this, tr("错误"), tr("请先选择一个成员"));
        return;
    }

    QString username = memberModel->item(row, 0)->text();

    if (QMessageBox::question(this, tr("确认移除"),
        tr("确定要将 %1 从家庭中移除吗？").arg(username))
        != QMessageBox::Yes) {
        return;
    }


    if (familyManager.removeMember(currentFamily, username)) {
        refreshMemberList(currentFamily);
        QMessageBox::information(this, tr("成功"), tr("成员已移除"));
    }
    else {
        QMessageBox::warning(this, tr("错误"), tr("移除成员失败"));
    }
}

void MainWindow::onPromoteMember() {
    changeMemberRole(true);
}

void MainWindow::onDemoteMember() {
    changeMemberRole(false);
}

void MainWindow::changeMemberRole(bool promote) {
    if (currentFamily.isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("请先选择一个家庭"));
        return;
    }

    int row = ui.memberTableView->currentIndex().row();
    if (row < 0) {
        QMessageBox::warning(this, tr("错误"), tr("请先选择一个成员"));
        return;
    }

    QString username = memberModel->item(row, 0)->text();
    QString currentRole = memberModel->item(row, 1)->text();

    if ((promote && currentRole == "家长") || (!promote && currentRole == "孩子")) {
        QMessageBox::information(this, tr("提示"), tr("成员已经是该角色"));
        return;
    }

    if (familyManager.setMemberRole(currentFamily, username, promote)) {
        // 更新用户角色
        User user = userManager.getUser(username);
        user.setRole(promote ? User::FAMILY_PARENT : User::FAMILY_CHILD);
        userManager.updateUser(user);

        refreshMemberList(currentFamily);
        QMessageBox::information(this, tr("成功"),
            promote ? tr("成员已提升为家长") : tr("成员已设为孩子"));
    }
    else {
        QMessageBox::warning(this, tr("错误"), tr("修改角色失败"));
    }
}


void MainWindow::setupBillingTab()
{
    // 在账单标签页添加过滤选项
    QHBoxLayout* filterLayout = new QHBoxLayout();

    callRecordFilterCombo = new QComboBox(this);
    callRecordFilterCombo->addItem("全部通话", 0);
    callRecordFilterCombo->addItem("已接通", 1);
    callRecordFilterCombo->addItem("未接通", 2);
    callRecordFilterCombo->addItem("家庭通话", 3);

    filterLayout->addWidget(new QLabel("过滤:", this));
    filterLayout->addWidget(callRecordFilterCombo);
    filterLayout->addStretch();

    QVBoxLayout* billingLayout = qobject_cast<QVBoxLayout*>(ui.billingTab->layout());
    billingLayout->insertLayout(0, filterLayout);

   // connect(callRecordFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
    //    this, &MainWindow::onCallRecordFilterChanged);

    // 添加家庭账单按钮
    QPushButton* familyBillBtn = new QPushButton("生成家庭账单", this);
    billingLayout->addWidget(familyBillBtn);
    connect(familyBillBtn, &QPushButton::clicked, this, &MainWindow::onGenerateFamilyBill);
}


// 生成家庭账单
void MainWindow::onGenerateFamilyBill() {
    if (!familyManager.isFamilyParent(familyManager.getUserFamily(currentUser.getUserName()), currentUser.getUserName())) {
        QMessageBox::warning(this, "权限不足", "只有家庭家长可以生成家庭账单");
        return;
    }

    QString defaultName = QString("家庭账单_%1_%2.html")
        .arg(familyManager.getUserFamily(currentUser.getUserName()))
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd"));

    QString fileName = QFileDialog::getSaveFileName(this, "保存家庭账单",
        defaultName,
        "HTML文件 (*.html)");
    if (fileName.isEmpty()) return;

    FamilyManager familyMgr;
    familyMgr.loadFamilies();
    Family family = familyMgr.getFamily(familyManager.getUserFamily(currentUser.getUserName()));

    if (family.isEmpty()) {
        QMessageBox::warning(this, "错误", "未找到家庭信息");
        return;
    }

    // 生成HTML账单
    billingSystem.generateFamilyReport(family, fileName);

    // 显示生成结果
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("账单生成成功");
    msgBox.setText(QString("家庭账单已生成!"));
    msgBox.setInformativeText(QString("文件已保存到:\n%1\n\n是否立即查看?").arg(fileName));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);

    if (msgBox.exec() == QMessageBox::Yes) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
    }
}

//// 通话记录过滤
//void MainWindow::onCallRecordFilterChanged(int index) {
//    QString number = ui.numberComboBox->currentText();
//    Billing::BillingHistory history = billingSystem.getBillingHistory(number);
//
//    QStandardItemModel* model = new QStandardItemModel(this);
//    model->setHorizontalHeaderLabels({ "时间", "对方号码", "时长", "状态", "费用", "类型" });
//
//    int filterType = callRecordFilterCombo->currentData().toInt();
//
//    for (const Billing::CallRecord& record : history.Records) {
//        // 应用过滤条件
//        if ((filterType == 1 && !record.answered) ||
//            (filterType == 2 && record.answered) ||
//            (filterType == 3 && !record.isFamilyCall)) {
//            continue;
//        }
//
//        QList<QStandardItem*> items;
//        items << new QStandardItem(record.callTime.toString("MM-dd hh:mm"));
//        items << new QStandardItem(record.number);
//        items << new QStandardItem(QString::number(record.duration));
//
//        QStandardItem* statusItem = new QStandardItem(record.answered ? "? 接通" : "? 未接");
//        statusItem->setForeground(record.answered ? Qt::darkGreen : Qt::red);
//        items << statusItem;
//
//        QStandardItem* chargeItem = new QStandardItem(QString::number(record.charge, 'f', 2));
//        chargeItem->setTextAlignment(Qt::AlignRight);
//        items << chargeItem;
//
//        QStandardItem* typeItem = new QStandardItem(record.isFamilyCall ? "家庭" : "外部");
//        typeItem->setBackground(record.isFamilyCall ? QColor(230, 255, 230) : Qt::white);
//        items << typeItem;
//
//        model->appendRow(items);
//    }
//
//    ui.callRecordTableView->setModel(model);
//    ui.callRecordTableView->resizeColumnsToContents();
//}

// 更新家庭费用信息
void MainWindow::refreshFamilyBillingInfo() {
    if (!familyManager.isFamilyMember(currentUser.getUserName())) return;

    FamilyManager familyMgr;
    familyMgr.loadFamilies();
    Family family = familyMgr.getFamily(familyManager.getUserFamily(currentUser.getUserName()));

    if (family.isEmpty()) return;

    // 更新总费用标签
    double totalCharge = family.getFamilyTotalCharge();
    familyChargeLabel->setText(QString("家庭总费用: ?%1").arg(totalCharge, 0, 'f', 2));

    // 更新分摊表格
    familyBillingModel->removeRows(0, familyBillingModel->rowCount());

    auto distribution = family.getChargeDistribution();
    for (const auto& [parent, amount] : distribution) {
        QList<QStandardItem*> items;
        items << new QStandardItem(parent);
        items << new QStandardItem(QString::number(amount, 'f', 2));

        // 获取已支付金额（从账单系统）
        double paid = 0.0;
        if (billingSystem.getAllBillingHistory().contains(parent)) {
            auto records = billingSystem.getAllBillingHistory()[parent]->Records;
            for (const auto& record : records) {
                if (record.plan == "family_payment") {
                    paid += record.charge;
                }
            }
        }

        items << new QStandardItem(QString::number(paid, 'f', 2));
        items << new QStandardItem(QString::number(amount - paid, 'f', 2));

        // 标记欠费
        if (paid < amount) {
            items.back()->setForeground(Qt::red);
        }

        familyBillingModel->appendRow(items);
    }
}

// 处理家庭费用分摊信号
void MainWindow::onFamilyChargeDistributed(const QString& familyName, double amount) {
    if (familyManager.isFamilyMember(currentUser.getUserName()) && familyManager.getUserFamily(currentUser.getUserName()) == familyName) {
        refreshFamilyBillingInfo();

        // 显示通知
        QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::MessageIcon(QSystemTrayIcon::Information);
        trayIcon->showMessage("家庭费用更新",
            QString("新的家庭费用已分摊: %1").arg(amount, 0, 'f', 2),
            icon, 3000);
    }
}
