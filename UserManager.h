#ifndef USERMANAGER_H
#define USERMANAGER_H

#include "User.h"
#include <QObject>
#include <QVector>
#include <QJsonObject>
#include <QJsonArray>

class FamilyManager;

class UserManager : public QObject
{
    Q_OBJECT
public:
    explicit UserManager(QObject* parent = nullptr);
    UserManager(FamilyManager* familyManager, QObject* parent = nullptr);
    void init(FamilyManager* familyManager);
    // 用户管理核心功能
    bool addUser(const User& user);
    bool deleteUser(const QString& username);
    bool updateUser(const User& user);
    User& getUser(const QString& username) ;
    QVector<User>& getAllUsers();
    bool resetPassword(const QString& username, const QString& newPassword);
    void clearAllUsers();
    User getUserByNumber(const QString& number);
    // 持久化操作
    bool loadUsers();
    bool saveUsers() const;

    // 验证功能
    bool validateUser(const QString& username, const QString& password) const;
    bool usernameExists(const QString& username) const;


    // 家庭相关方法
    QVector<User> getFamilyMembers(const QString& familyName) const;
    QVector<User> getFamilyParents(const QString& familyName) const;
    QVector<User> getFamilyChildren(const QString& familyName) const;

    // 修改用户角色时需要同步更新家庭关系
    bool setUserRole(const QString& username, User::Role newRole);

signals:
    void usersChanged(); // 用户数据变更信号

private:
    QVector<User> m_users;
    QString m_dataFile = "users.json";
    FamilyManager* m_familyManager ;
    int findUserIndex(const QString& username) const;
};

#endif // USERMANAGER_H
