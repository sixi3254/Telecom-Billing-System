#include "UserManager.h"
#include"FamilyManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QDebug>




UserManager::UserManager(QObject* parent) : QObject(parent)
{
    loadUsers();
}
UserManager::UserManager(FamilyManager* familyManager, QObject* parent ): QObject(parent)
{
    m_familyManager = familyManager;
    loadUsers();
}

void UserManager::init(FamilyManager* familyManager)
{
    this->m_familyManager = familyManager;
}


bool UserManager::addUser(const User& user)
{
    if (usernameExists(user.getUserName())) {
        qWarning() << "Username already exists:" << user.getUserName();
        return false;
    }

    m_users.append(user);
    if (saveUsers()) {
        emit usersChanged();
        return true;
    }
    return false;
}

bool UserManager::deleteUser(const QString& username)
{
    int index = findUserIndex(username);
    if (index == -1) {
        qWarning() << "User not found:" << username;
        return false;
    }

    m_users.remove(index);
    if (saveUsers()) {
        emit usersChanged();
        return true;
    }
    return false;
}

bool UserManager::updateUser(const User& user)//更新用户信息 用户名不变
{
    int index = findUserIndex(user.getUserName());
    if (index == -1) {
        qWarning() << "User not found:" << user.getUserName();
        return false;
    }

    m_users.replace(index, user);//将修改后的用户替换原来的用户
    if (saveUsers()) {
        emit usersChanged();
        return true;
    }
    return false;
}

User& UserManager::getUser(const QString& username)
{
    for (auto& user : m_users) {
        if (user.getUserName() == username) {
            return user;
        }
    }
    throw std::out_of_range("User not found");
}

 QVector<User>& UserManager::getAllUsers() 
{
    return m_users;
}

bool UserManager::resetPassword(const QString& username, const QString& newPassword)
{
    int index = findUserIndex(username);
    if (index == -1) {
        qWarning() << "User not found:" << username;
        return false;
    }

    User user = m_users.at(index);
    user.setPassword(newPassword);
    m_users.replace(index, user);//更新

    return saveUsers();
}

bool UserManager::loadUsers()
{
    QFile file(m_dataFile);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open users file:" << m_dataFile;
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isNull()) {
        qWarning() << "Invalid JSON format in users file";
        return false;
    }

    m_users.clear();
    QJsonArray usersArray = doc.array();
    for (const QJsonValue& value : usersArray) {
        m_users.append(User::fromJson(value.toObject()));
    }
    file.close();
    return true;
}

bool UserManager::saveUsers() const
{
    QFile file(m_dataFile);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open users file for writing:" << m_dataFile;
        return false;
    }

    QJsonArray usersArray;
    for (const User& user : m_users) {
        usersArray.append(user.toJson());
    }

    QJsonDocument doc(usersArray);
    if (file.write(doc.toJson()) == -1) {
        qWarning() << "Failed to write users data";
        return false;
    }
    qDebug() << "save users data success";
    return true;
}

bool UserManager::validateUser(const QString& username, const QString& password) const
{
    int index = findUserIndex(username);
    if (index == -1) {
        return false;
    }

    // 实际项目中应该使用加密验证
    return m_users[index].getPassWord() == password;
}

bool UserManager::usernameExists(const QString& username) const
{
    return findUserIndex(username) != -1;
}

int UserManager::findUserIndex(const QString& username) const
{
    for (int i = 0; i < m_users.size(); ++i) {
        if (m_users[i].getUserName() == username) {
            return i;
        }
    }
    return -1;
}

void UserManager::clearAllUsers()
{
    m_users.clear();
}


//获取一个家庭中的所有成员user
QVector<User> UserManager::getFamilyMembers(const QString& familyName) const {
    QVector<User> members;
    for (const User& user : m_users) {
        if (m_familyManager->getUserFamily(user.getUserName()) == familyName) {
            members.append(user);
           
        }
    }
    return members;
}
//获取一个家庭中的所有家长user
QVector<User> UserManager::getFamilyParents(const QString& familyName) const {
    QVector<User> parents;
    for (const User& user : m_users) {
        if (m_familyManager->getUserFamily(user.getUserName()) == familyName && m_familyManager->isFamilyParent(m_familyManager->getUserFamily(user.getUserName()),user.getUserName())) {
            parents.append(user);
        }
    }
    return parents;
}
//获取一个家庭中的所有孩子user
QVector<User> UserManager::getFamilyChildren(const QString& familyName) const {
    QVector<User> children;
    for (const User& user : m_users) {
        if (m_familyManager->getUserFamily(user.getUserName()) == familyName && !m_familyManager->isFamilyParent(m_familyManager->getUserFamily(user.getUserName()), user.getUserName())) {
            children.append(user);
        }
    }
    return children;
}

bool UserManager::setUserRole(const QString& username, User::Role newRole) {
    int index = findUserIndex(username);
    if (index == -1) return false;

    User& user = m_users[index];
    User::Role oldRole = user.getRole();

    // 角色未变化直接返回成功
    if (oldRole == newRole) return true;

    // 处理家庭角色变更
    if (m_familyManager->isFamilyMember(user.getUserName())) {
        QString familyName = m_familyManager->getUserFamily(user.getUserName());

        // 从旧角色中移除
        if (oldRole == User::FAMILY_PARENT) {
            m_familyManager->setMemberRole(familyName, username, false);
        }

        // 添加到新角色
        if (newRole == User::FAMILY_PARENT) {
            if (!m_familyManager->setMemberRole(familyName, username, true)) {
                return false;
            }
        }
    }

    // 更新用户角色
    user.setRole(newRole);

    // 特殊处理：如果从家长角色改为非家长，需要检查家庭是否还有家长
    if (oldRole == User::FAMILY_PARENT &&
        (newRole != User::FAMILY_PARENT && newRole != User::ADMIN)) {


        QString familyName = m_familyManager->getUserFamily(user.getUserName());

        // 如果没有其他家长，自动指定第一个孩子为家长
        auto parents = m_familyManager->getFamilyParents(familyName);
        if (parents.isEmpty()) {
            auto children = m_familyManager->getFamilyChildren(familyName);
            if (!children.isEmpty()) {
                m_familyManager->setMemberRole(familyName, children.first(), true);

                // 更新被提升的用户角色
                int childIndex = findUserIndex(children.first());
                if (childIndex != -1) {
                    m_users[childIndex].setRole(User::FAMILY_PARENT);
                }
            }
            else {
                // 没有成员则解散家庭
                m_familyManager->disbandFamily(familyName);
            }
        }
    }

    emit usersChanged();
    return true;
}

User UserManager::getUserByNumber(const QString& number)
{
    // 遍历所有用户，查找号码匹配的用户
    for (const User& user : m_users)
    {
        // 该用户的号码列表
        QList<PhoneNumber> numbers = user.getPhoneNumbers();
        //遍历此用户的所有号码
        for (const PhoneNumber& phone : numbers)
        {
            if (phone.getNumber() == number)
            {
                return user;
            }
        }
    }
    return User();
}