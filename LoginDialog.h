#pragma once

#include <QDialog>
#include "ui_LoginDialog.h"
#include<QList>
#include "User.h"
class LoginDialog : public QDialog
{
	Q_OBJECT

public:
	LoginDialog(QWidget *parent = nullptr);
	~LoginDialog();
	User getLoggedInUser() const { return loggedInUser; }

private slots:
	void onLoginClicked();//登录
	void onRegisterClicked();//注册
	void togglePasswordVisibility();//密码可见切换

private:
	Ui::LoginDialog ui;
	User loggedInUser;
	QList<User> users;

	void loadUsers();
	void saveUsers();
	bool validateUser(const QString& username, const QString& password);
	bool registerUser(const QString& username, const QString& password, User::Role role);
	void showError(const QString& message);
	void clearInputs();
	void loadRememberedUser();
	void saveRememberedUser();
};
