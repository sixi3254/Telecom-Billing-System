#pragma once

#include <QDialog>
#include "UserManager.h"
#include "FamilyManager.h"
#include "Billing.h"
#include "ui_SearchDialog.h"

class SearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SearchDialog(UserManager *userManager, 
                         FamilyManager *familyManager,
                         Billing *billingSystem,
                         const User &currentUser,
                         QWidget *parent = nullptr);
    ~SearchDialog();

private slots:
    void onSearchClicked();
    void onExitClicked();

private:
    Ui::SearchDialog *ui;
    UserManager *m_userManager;
    FamilyManager *m_familyManager;
    Billing *m_billingSystem;
    User m_currentUser;

    void searchUsers(const QString &keyword);
    void searchPhoneNumbers(const QString &keyword);
    void searchCallRecords(const QString &keyword);
    void showResultsInTable(const QVector<QStringList> &results, const QStringList &headers);
};
