#include "SearchDialog.h"
#include "ui_SearchDialog.h"
#include <QMessageBox>
#include <QHeaderView>
#include<QTableWidget>

SearchDialog::SearchDialog(UserManager* userManager,
    FamilyManager* familyManager,
    Billing* billingSystem,
    const User& currentUser,
    QWidget* parent) :
    QDialog(parent),
    ui(new Ui::SearchDialog),
    m_userManager(userManager),
    m_familyManager(familyManager),
    m_billingSystem(billingSystem),
    m_currentUser(currentUser)
{
    ui->setupUi(this);
    setWindowTitle(tr("全局搜索"));

    // 设置搜索范围选项
    ui->scopeCombo->addItem(tr("全部"), 0);
    ui->scopeCombo->addItem(tr("用户"), 1);
    ui->scopeCombo->addItem(tr("电话号码"), 2);
    ui->scopeCombo->addItem(tr("通话记录"), 3);

    // 连接信号槽
    connect(ui->searchButton, &QPushButton::clicked, this, &SearchDialog::onSearchClicked);
    connect(ui->exitButton, &QPushButton::clicked, this, &SearchDialog::onExitClicked);
    connect(ui->searchInput, &QLineEdit::returnPressed, this, &SearchDialog::onSearchClicked);
}

SearchDialog::~SearchDialog()
{
    delete ui;
}

void SearchDialog::onSearchClicked()
{
    QString keyword = ui->searchInput->text().trimmed();
    if (keyword.isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("请输入搜索关键词"));
        return;
    }

    int scope = ui->scopeCombo->currentData().toInt();

    switch (scope) {
    case 0: // 全部
        searchUsers(keyword);
        searchPhoneNumbers(keyword);
        searchCallRecords(keyword);
        break;
    case 1: // 用户
        searchUsers(keyword);
        break;
    case 2: // 电话号码
        searchPhoneNumbers(keyword);
        break;
    case 3: // 通话记录
        searchCallRecords(keyword);
        break;
    }
}

void SearchDialog::onExitClicked()
{
    // 发出关闭信号并刷新主窗口
    accept(); // 使用accept()而不是close()以确保返回QDialog::Accepted
}

void SearchDialog::searchUsers(const QString& keyword)
{
    QVector<QStringList> results;
    QVector<User> allUsers = m_userManager->getAllUsers();

    for (const User& user : allUsers) {
        if (user.getUserName().contains(keyword, Qt::CaseInsensitive)) {
            QStringList row;
            row << user.getUserName()
                << (user.isAdmin() ? tr("管理员") :
                    (user.getRole() == User::FAMILY_PARENT ? tr("家庭家长") :
                        (user.getRole() == User::FAMILY_CHILD ? tr("家庭子用户") : tr("普通用户"))))
                << m_familyManager->getUserFamily(user.getUserName())
                << QString::number(user.getBalance(), 'f', 2);
            results.append(row);
        }
    }

    showResultsInTable(results, { tr("用户名"), tr("角色"), tr("家庭"), tr("余额") });
}

void SearchDialog::searchPhoneNumbers(const QString& keyword)
{
    QVector<QStringList> results;
    QList<PhoneNumber> numbers;

    if (m_currentUser.isAdmin()) {
        QVector<User> users = m_userManager->getAllUsers();
        for (const User& user : users) {
            numbers.append(user.getPhoneNumbers());
        }
    }
    else {
        numbers = m_currentUser.getPhoneNumbers();
    }

    for (const PhoneNumber& number : numbers) {
        if (number.getNumber().contains(keyword) ||
            number.getType().contains(keyword, Qt::CaseInsensitive) ||
            number.getServiceType().contains(keyword, Qt::CaseInsensitive) ||
            number.getPlan().contains(keyword, Qt::CaseInsensitive)) {
            QStringList row;
            row << number.getNumber()
                << number.getType()
                << number.getServiceType()
                << number.getPlan();
            results.append(row);
        }
    }

    showResultsInTable(results, { tr("号码"), tr("类型"), tr("服务类型"), tr("套餐") });
}

void SearchDialog::searchCallRecords(const QString& keyword)
{
    QVector<QStringList> results;
    QMap<QString, std::shared_ptr<Billing::BillingHistory>> allHistory;

    if (m_currentUser.isAdmin()) {
        allHistory = m_billingSystem->getAllBillingHistory();
    }
    else {
        for (const PhoneNumber& number : m_currentUser.getPhoneNumbers()) {
            allHistory.insert(number.getNumber(),
                std::make_shared<Billing::BillingHistory>(m_billingSystem->getBillingHistory(number.getNumber())));
        }
    }

    for (auto it = allHistory.begin(); it != allHistory.end(); ++it) {
        for (const Billing::CallRecord& record : it.value()->Records) {
            if (record.number.contains(keyword) ||
                record.plan.contains(keyword, Qt::CaseInsensitive) ||
                record.callTime.toString().contains(keyword) ||
                QString::number(record.duration).contains(keyword) ||
                QString::number(record.charge).contains(keyword)) {
                QStringList row;
                row << it.key() // 主叫号码
                    << record.number // 被叫号码
                    << record.callTime.toString("yyyy-MM-dd hh:mm")
                    << QString::number(record.duration)
                    << (record.isLongDistance ? tr("长途") : tr("本地"))
                    << (record.answered ? tr("接通") : tr("未接"))
                    << QString::number(record.charge, 'f', 2);
                results.append(row);
            }
        }
    }

    showResultsInTable(results, { tr("主叫号码"), tr("被叫号码"), tr("时间"),
                               tr("时长"), tr("类型"), tr("状态"), tr("费用") });
}

void SearchDialog::showResultsInTable(const QVector<QStringList>& results, const QStringList& headers)
{
    QTableWidget* table = new QTableWidget(this);
    table->setRowCount(results.size());
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSortingEnabled(true);

    for (int row = 0; row < results.size(); ++row) {
        const QStringList& rowData = results[row];
        for (int col = 0; col < rowData.size(); ++col) {
            QTableWidgetItem* item = new QTableWidgetItem(rowData[col]);
            table->setItem(row, col, item);
        }
    }

    table->resizeColumnsToContents();

    // 清除旧结果并添加新表
    QLayoutItem* child;
    while ((child = ui->resultsLayout->takeAt(0))) {
        delete child->widget();
        delete child;
    }

    ui->resultsLayout->addWidget(table);

}
