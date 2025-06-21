#pragma once
#include <QString>
#include <QSet>
#include <QJsonObject>
#include <QVector>
class Family
{
public:
    Family() = default;
    explicit Family(const QString& familyName);

    // 序列化方法
    QJsonObject toJson() const;
    static Family fromJson(const QJsonObject& json);

    // 家庭成员管理
    bool addMember(const QString& username, bool isParent = false);
    void removeMember(const QString& username);
    bool contains(const QString& username) const;

    // 费用管理
    void addCharge(const QString& payer, double amount);
    double getFamilyTotalCharge() const;
    QVector<QPair<QString, double>> getChargeDistribution() const;
    void addFamilyTotalCharge(double amount){ familyTotalCharge += amount; }

    // 信息获取
    const QString& getFamilyName() const { return familyName; }
    const QSet<QString>& getParents() const { return parents; }
    const QSet<QString>& getChildren() const { return children; }
    bool isParent(const QString& username) const;
    bool isEmpty() const;
    QDate getCreateTime() const { return createTime; }

    void setFamilyName(const QString& name) { familyName = name; }
    void setCreateTime(const QDate& time);
private:
    QString familyName;
    QSet<QString> parents;    // 家长集合
    QSet<QString> children;   // 孩子集合
    QVector<QPair<QString, double>> chargeRecords; // 缴费记录 <缴费人,金额>
    double familyTotalCharge = 0;
    QDate createTime;
};

