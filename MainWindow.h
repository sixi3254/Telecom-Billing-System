#pragma once

#include <QMainWindow>
#include"Billing.h"
#include "PhoneNumber.h"
#include "UserManager.h"
#include "ui_MainWindow.h" 
#include"QTableWidget"
#include<QComboBox>
#include<QLabel>
#include<QSystemTrayIcon>
#include <QStandardItemModel>
class User;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const User& currentUser, QWidget* parent = nullptr);
    ~MainWindow();


    enum class CallRecordColumn {
        Time = 0,
        Duration = 1,
        Type = 2,
        Family,
        Status ,
        Charge ,
        ColumnCount
    };

    enum class ReportType {
        FixedLongDistance,
        FixedLocal,
        WirelessLongDistance,
        WirelessLocal,
        WirelessAnswer,
        TelecomSummary
    };

private slots:
    // 用户管理相关槽函数
    void onAddUserClicked();
    void onDeleteUserClicked();
    void onEditUserClicked();

    // 电话号码管理相关槽函数
    void onAddNumberClicked();
    void onDeleteNumberClicked();
    void onEditNumberClicked();

    // 通话记录相关槽函数
    void onQueryCallRecordClicked();
    void onGenerateBillClicked();
    void onGenerateFixedLongFile();
    void onGenerateFixedLocalFile();
    void onGenerateWirelessLongFile();
    void onGenerateWirelessLocalFile();
    void onGenerateWirelessAnswerFile();
    void onGenerateChargeSummaryFile();
    QString generateChargeSummaryHtml();
    void generateCallReport(const QString& category, const QString& reportTitle, const QString& callType);
    QString generateCallReportHtml(const QString& category, const QString& reportTitle, const QString& callType);
    void askToShowOrSave(const QString& html, const QString& title);

    // 其他功能
    void onLogoutClicked();
    void onAboutClicked();
    void refreshUserList();
    void refreshNumberList();

    void addSummaryRow(const Billing::BillingHistory& history, const QBrush& background);
    
    void onGenerateTestDataClicked();


    //家庭
    void setupFamilyTab();
    void refreshFamilyList();
    void refreshMemberList(const QString& familyName);
    void onCreateFamily();
    void onDisbandFamily();
    void onAddMember();
    void onRemoveMember();
    void onPromoteMember();
    void onDemoteMember();
    void onFamilySelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void changeMemberRole(bool promote);

    void onGenerateFamilyBill();
    //void onCallRecordFilterChanged(int index);  // 通话记录过滤
    void refreshFamilyBillingInfo();
    void onFamilyChargeDistributed(const QString& familyName, double amount);

    void onUserDoubleClicked(const QModelIndex& index);
    void onNumberDoubleClicked(const QModelIndex& index);
    void showUserDetails(const QString& username);
    void showCallRecords(const QString& number, QWidget* parent = nullptr);

    //搜索
    void onSearchButtonClicked();
signals:
    //// 余额变更通知信号
    //void userBalanceUpdated(const QString& userName,
    //    double oldValue,
    //    double newValue,
    //    const QDateTime& updateTime = QDateTime::currentDateTime());


private:
    Ui::MainWindow ui;
    User currentUser;
    UserManager userManager=UserManager();
    QStandardItemModel* userModel;
    QStandardItemModel* numberModel;
    Billing billingSystem=Billing();
    QStandardItemModel* familyModel;
    QStandardItemModel* memberModel;
    FamilyManager familyManager=FamilyManager();
    QString currentFamily; // 当前选中的家庭

    QTableWidget* callRecordTable;


    QStandardItemModel* familyBillingModel;
    QComboBox* callRecordFilterCombo;
    QLabel* familyChargeLabel;

    QSystemTrayIcon* trayIcon;//托盘图标

    QComboBox* serviceTypeFilterCombo;
    QComboBox* phoneTypeFilterCombo;
    QPushButton* applyFilterButton;
    QPushButton* resetFilterButton;

    QToolBar* mainToolBar;  // 添加成员变量
 
   
    void setupBillingTab();
    void setupUI();
    void setupConnections();
    void initializeModels();
    void updateUIByRole();
    void showError(const QString& message);

    QStringList getAllPhoneNumbers();
    User findUserByNumber(const QString& number);
    //void updateUserCharges();


    void setupNumberFilterUI();
    void applyNumberFilters();
    void resetNumberFilters();

    //void showHtmlReport(const QString& html, const QString& title);
};
