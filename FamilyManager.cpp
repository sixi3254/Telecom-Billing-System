#include "FamilyManager.h"
#include "UserManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>



FamilyManager::FamilyManager(QObject* parent) : QObject(parent) {loadFamilies();}
FamilyManager::FamilyManager(UserManager* userManager, QObject* parent)
    : QObject(parent), userManager(userManager) 
{

    loadFamilies();

}


void FamilyManager::init(UserManager* userManager)
{
    this->userManager = userManager;
}



//创建家庭
bool FamilyManager::createFamily(const QString& familyName, const QString& creator) {

    userManager->loadUsers();
    if (familyName.isEmpty() || families.contains(familyName)) {
        return false;
    }


    if (userManager && !userManager->getUser(creator).getUserName().isEmpty()) {
        Family newFamily(familyName);
        if (newFamily.addMember(creator, true)) {
            families.insert(familyName, newFamily);

            // 更新用户信息
            User user = userManager->getUser(creator);

            user.setRole(User::FAMILY_PARENT);

            userManager->updateUser(user);
           
            saveFamilies();
            emit familyCreated(familyName);
            return true;
        }
    }
    return false;
}


//解散家庭
bool FamilyManager::disbandFamily(const QString& familyName) {
    if (!families.contains(familyName)) {
        return false;
    }

    // 更新所有成员信息
    Family family = families.value(familyName);
    for (const QString& member : family.getParents() + family.getChildren()) {
        User user = userManager->getUser(member);

        user.setRole(User::NORMAL);
        userManager->updateUser(user);
    }

    families.remove(familyName);
    saveFamilies();
    emit familyDisbanded(familyName);
    return true;
}


bool FamilyManager::renameFamily(const QString& oldName, const QString& newName) {
    if (!families.contains(oldName) || families.contains(newName)) {
        return false;
    }

    Family family = families.take(oldName);
    family.setFamilyName(newName);
    families.insert(newName, family);

    // 更新所有成员的家庭名称
    for (const QString& member : family.getParents() + family.getChildren()) {
        User user = userManager->getUser(member);

        userManager->updateUser(user);
    }

    return true;
}


//内部添加成员
bool FamilyManager::addMember(const QString& familyName, const QString& username, bool asParent) {
    if (!families.contains(familyName) || !userManager) {
        return false;
    }

    User user = userManager->getUser(username);
    if (user.getUserName().isEmpty()) {
        return false;
    }

    Family& family = families[familyName];
    if (family.contains(username)) {
        return false;
    }

    if (internalAddMember(family, username, asParent)) {
        // 更新用户信息
        user.setRole(asParent ? User::FAMILY_PARENT : User::FAMILY_CHILD);
        userManager->updateUser(user);
        addChargeRecord(familyName, username, userManager->getUser(username).getFamilyChargeShare());
        emit memberAdded(familyName, username, asParent);
        return true;
    }
    return false;
}


bool FamilyManager::removeMember(const QString& familyName, const QString& username) {
    if (!families.contains(familyName)) {
        return false;
    }

    Family& family = families[familyName];
    if (!family.contains(username)) {
        return false;
    }

    family.removeMember(username);

    // 更新用户信息
    if (userManager) {
        User user = userManager->getUser(username);
        user.setRole(User::NORMAL);
        userManager->updateUser(user);
    }

    emit memberRemoved(familyName, username);

    // 如果家庭为空则自动解散
    if (family.isEmpty()) {
        disbandFamily(familyName);
    }

    return true;
}



bool FamilyManager::setMemberRole(const QString& familyName, const QString& username, bool asParent) {
    if (!families.contains(familyName) || !userManager) {
        return false;
    }

    Family& family = families[familyName];
    if (!family.contains(username)) {
        return false;
    }

    // 先移除再重新添加
    family.removeMember(username);
    if (internalAddMember(family, username, asParent)) {
        // 更新用户信息
        User user = userManager->getUser(username);
        user.setRole(asParent ? User::FAMILY_PARENT : User::FAMILY_CHILD);
        userManager->updateUser(user);
        return true;
    }
    return false;
}



bool FamilyManager::addChargeRecord(const QString& familyName, const QString& payer, double amount) {
    if (!families.contains(familyName) || amount <= 0) {
        return false;
    }

    Family& family = families[familyName];
    if (!family.contains(payer)) {
        return false;
    }

    family.addCharge(payer, amount);
    emit chargeRecordAdded(familyName, payer, amount);
    return true;
}

bool FamilyManager::settleCharge(const QString& familyName, const QString& payer, double amount) {
    if (!families.contains(familyName) || amount <= 0) {
        return false;
    }

    Family& family = families[familyName];
    if (!family.isParent(payer)) {
        return false;
    }

    // 在实际应用中，这里会更新支付状态
    // 简化版只记录支付金额
    family.addCharge(payer, -amount); // 负值表示支付
    return true;
}

Family& FamilyManager::getFamily(const QString& familyName) {
    return families[familyName];
}

QString FamilyManager::getUserFamily(const QString& username) const {
    for (auto it = families.begin(); it != families.end(); ++it) {
        if (it.value().contains(username)) {
            return it.key();
        }
    }
    return QString();
}

QVector<Family> FamilyManager::getAllFamilies() const {
    QVector<Family> result;
    for (auto it = families.begin(); it != families.end(); ++it) {
        result.append(it.value());
    }
    return result;
}

QVector<QString> FamilyManager::getFamilyMembers(const QString& familyName)  {
    Family& family = getFamily(familyName);
    QVector<QString> members;
    for (const QString& parent : family.getParents()) {
        members.append(parent);
    }
    for (const QString& child : family.getChildren()) {
        members.append(child);
    }
    return members;
}

QVector<QString> FamilyManager::getFamilyParents(const QString& familyName)  {
    Family& family = getFamily(familyName);
    QVector<QString> parents;
    for (const QString& parent : family.getParents()) {
        parents.append(parent);
    }
    return parents;
}

QVector<QString> FamilyManager::getFamilyChildren(const QString& familyName) {
    Family& family = getFamily(familyName);
    QVector<QString> children;
    for (const QString& child : family.getChildren()) {
        children.append(child);
    }
    return children;
}

double FamilyManager::getFamilyTotalCharge(const QString& familyName)  {
    return getFamily(familyName).getFamilyTotalCharge();
}

QVector<QPair<QString, double>> FamilyManager::getChargeDistribution(const QString& familyName){
    return getFamily(familyName).getChargeDistribution();
}

bool FamilyManager::loadFamilies() {
    QFile file(dataFile);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        return false;
    }

    families.clear();
    QJsonObject root = doc.object();
    for (auto it = root.begin(); it != root.end(); ++it) {
        Family family = Family::fromJson(it.value().toObject());
        families.insert(it.key(), family);
    }

    return true;
}

bool FamilyManager::saveFamilies() const {
    QFile file(dataFile);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QJsonObject root;
    for (auto it = families.begin(); it != families.end(); ++it) {
        root.insert(it.key(), it.value().toJson());
    }

    QJsonDocument doc(root);
    file.write(doc.toJson());
    return true;
}

void FamilyManager::setUserManager(UserManager* userManager) {
    this->userManager = userManager;
}

bool FamilyManager::validateFamilyName(const QString& familyName) const {
    return !familyName.isEmpty() && familyName.length() <= 50;
}

bool FamilyManager::internalAddMember(Family& family, const QString& username, bool asParent) {
    if (family.contains(username)) {
        return false;
    }

    family.addMember(username, asParent);
    return true;
}


void FamilyManager::updateFamilyCharge(const QString& familyName, const QString& member, double amount) {
    if (!families.contains(familyName)) return;

    Family& family = families[familyName];
    if (family.contains(member)) {
        family.addCharge(member, amount);
        emit chargeUpdated(familyName, member, amount);
    }
}

FamilyManager& FamilyManager::operator=(const FamilyManager& other)
{
    families = other.families;
    return *this;
}

//是否是家庭成员
bool FamilyManager::isFamilyMember(const QString& familyName, const QString& username) const
{
    return families.contains(familyName) && families[familyName].contains(username);
}
bool FamilyManager::isFamilyMember(const QString& username) const
{
    for (auto it = families.begin(); it != families.end(); ++it) {
        if (it.value().contains(username)) {
            return true;
        }
    }
    return false;
}

//是否是家庭父母
bool FamilyManager::isFamilyParent(const QString& familyName, const QString& username) const
{
    return families.contains(familyName) && families[familyName].isParent(username);
}
//是否是家庭子女
bool FamilyManager::isFamilyChild(const QString& familyName, const QString& username) const
{
    return families.contains(familyName) && (!families[familyName].isParent(username));
}

void FamilyManager::clearAllFamilies()
{
    families.clear();

}


void FamilyManager::addFamilyTotalCharge(const QString& familyName, double amount)
{
    if (families.contains(familyName)) {
        families[familyName].addFamilyTotalCharge(amount);
    }
}
