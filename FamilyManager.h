#pragma once
#include "Family.h"
#include <QObject>
#include <QMap>
#include <QJsonObject>
#include <QJsonArray>

class UserManager;

class FamilyManager: public QObject
{
    Q_OBJECT

public:
    explicit FamilyManager(QObject* parent = nullptr);
    explicit FamilyManager(UserManager* userManager, QObject* parent = nullptr);
    void init(UserManager* userManager);
    FamilyManager& operator=(const FamilyManager& other);


    // 家庭管理
    bool createFamily(const QString& familyName, const QString& creator);
    bool disbandFamily(const QString& familyName);
    bool renameFamily(const QString& oldName, const QString& newName);
    void clearAllFamilies();

    // 成员管理
    bool addMember(const QString& familyName, const QString& username, bool asParent = false);
    bool removeMember(const QString& familyName, const QString& username);
    bool setMemberRole(const QString& familyName, const QString& username, bool asParent);

    // 费用管理
    bool addChargeRecord(const QString& familyName, const QString& payer, double amount);
    bool settleCharge(const QString& familyName, const QString& payer, double amount);
    void updateFamilyCharge(const QString& familyName, const QString& member, double amount);
    void addFamilyTotalCharge(const QString& familyName, double amount);

    // 查询接口
    Family& getFamily(const QString& familyName) ;
    QString getUserFamily(const QString& username) const;
    QVector<Family> getAllFamilies() const;
    QVector<QString> getFamilyMembers(const QString& familyName) ;
    QVector<QString> getFamilyParents(const QString& familyName) ;
    QVector<QString> getFamilyChildren(const QString& familyName);
    double getFamilyTotalCharge(const QString& familyName) ;
    QVector<QPair<QString, double>> getChargeDistribution(const QString& familyName);
    //是否是家庭成员
    bool isFamilyMember(const QString& familyName, const QString& username) const;
    bool isFamilyMember( const QString& username) const;
    //是否是家庭父母
    bool isFamilyParent(const QString& familyName, const QString& username) const;
    //是否是家庭子女
    bool isFamilyChild(const QString& familyName, const QString& username) const;


    // 持久化
    bool loadFamilies();
    bool saveFamilies() const;

    // 用户管理集成
    void setUserManager(UserManager* userManager);

signals:
    void familyCreated(const QString& familyName);
    void familyDisbanded(const QString& familyName);
    void memberAdded(const QString& familyName, const QString& username, bool asParent);
    void memberRemoved(const QString& familyName, const QString& username);
    void chargeRecordAdded(const QString& familyName, const QString& payer, double amount);
    void chargeUpdated(const QString& familyName, const QString& member, double amount);
private:
    
    QMap<QString, Family> families;//QString为家庭名称，Family为家庭信息
    //初始化
    UserManager* userManager ;
    QString dataFile = "families.json";

    // 验证家庭姓氏是否有效
    bool validateFamilyName(const QString& familyName) const;
    // 内部方法，用于向家庭添加成员
    bool internalAddMember(Family& family, const QString& username, bool asParent);
};

