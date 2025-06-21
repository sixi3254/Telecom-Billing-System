#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include"LoginDialog.h"
#include"PhoneNumber.h"
#include"User.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
     //创建并显示登录对话框
    User user("admin", "Admin123", User::ADMIN);
    UserManager userManager;
    userManager.loadUsers();
    userManager.addUser(user);
    userManager.saveUsers();
    LoginDialog loginDialog;
    if (loginDialog.exec() == QDialog::Accepted) {
        // 登录成功
        User currentUser = loginDialog.getLoggedInUser();
        qDebug() << "Logged in as:" << currentUser.getUserName();

     /*    进入主界面*/

        MainWindow mainWindow(currentUser);
        mainWindow.show();
        return a.exec();
    }
    else {
        // 登录取消或失败
        QApplication::quit();
    }
    return 0;
}
