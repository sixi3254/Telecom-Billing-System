#pragma once

#include <QDialog>
#include "ui_AddEditUserDialog.h"
#include "User.h"
#include"FamilyManager.h"
#include"UserManager.h"
class AddEditUserDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AddEditUserDialog(QWidget* parent = nullptr);
    explicit AddEditUserDialog(const User& user, QWidget* parent = nullptr);
    ~AddEditUserDialog() = default;

    User getUser() const { return m_user; }

private slots:
    void onAccept();
    void validateInput();

private:
    Ui::AddEditUserDialog ui;
    User m_user;
    FamilyManager* m_familyManager;
    bool m_editMode = false;

    void setupUI();
    void populateRoleComboBox();
};
