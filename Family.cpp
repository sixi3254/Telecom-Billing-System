#include "Family.h"
#include <QJsonObject>
#include <QJsonArray>

Family::Family(const QString& familyName) : familyName(familyName) {}


QJsonObject Family::toJson() const {
    QJsonObject json;
    json["familyName"] = familyName;

    QJsonArray parentsArray;
    for (const auto& parent : parents) {
        parentsArray.append(parent);
    }
    json["parents"] = parentsArray;

    QJsonArray childrenArray;
    for (const auto& child : children) {
        childrenArray.append(child);
    }
    json["children"] = childrenArray;

    QJsonArray chargesArray;
    for (const auto& record : chargeRecords) {
        QJsonObject charge;
        charge["payer"] = record.first;
        charge["amount"] = record.second;
        chargesArray.append(charge);
    }
    json["charges"] = chargesArray;
    json["familyTotalCharge"] = familyTotalCharge;
    //保存时间
    json["time"] = createTime.toString(Qt::ISODate);
    return json;
}


Family Family::fromJson(const QJsonObject& json) {
    Family family(json["familyName"].toString());

    for (const auto& parent : json["parents"].toArray()) {
        family.parents.insert(parent.toString());
    }

    for (const auto& child : json["children"].toArray()) {
        family.children.insert(child.toString());
    }

    for (const auto& charge : json["charges"].toArray()) {
        QJsonObject c = charge.toObject();
        family.chargeRecords.append(qMakePair(
            c["payer"].toString(),
            c["amount"].toDouble()
        ));
    }
family.familyTotalCharge = json["familyTotalCharge"].toDouble();
    //读取时间
    family.createTime = QDate::fromString(json["time"].toString(), Qt::ISODate);
    if (!family.createTime.isValid()) {
        // 处理无效日期的情况
        qWarning() << "Invalid date format for creation time.";

    }
    return family;
}


bool Family::addMember(const QString& username, bool isParent) {
    if (isParent) {
        parents.insert(username);

        return true;
    }
    else {
        children.insert(username);
        return true;
    }
    return false;
}


void Family::removeMember(const QString& username) {
    parents.remove(username);
    children.remove(username);
}


bool Family::contains(const QString& username) const {
    return parents.contains(username) || children.contains(username);
}


void Family::addCharge(const QString& payer, double amount) {
    if (!parents.contains(payer) && !children.contains(payer)) {
        return;
    }
    chargeRecords.append(qMakePair(payer, amount));
}


double Family::getFamilyTotalCharge() const {
    return familyTotalCharge;
}


QVector<QPair<QString, double>> Family::getChargeDistribution() const {
    QMap<QString, double> distribution;

    // 初始化所有家长的分配金额
    for (const auto& parent : parents) {
        distribution[parent] = 0.0;
    }

    // 计算每个家长应该分摊的金额
    if (!parents.isEmpty()) {
        double perParentCharge = getFamilyTotalCharge() / parents.size();
        for (auto it = distribution.begin(); it != distribution.end(); ++it) {
            it.value() = perParentCharge;
        }
    }

    // 转换为向量返回
    QVector<QPair<QString, double>> result;
    for (auto it = distribution.begin(); it != distribution.end(); ++it) {
        result.append(qMakePair(it.key(), it.value()));
    }

    return result;
}


bool Family::isParent(const QString& username) const
{
    return parents.contains(username);
}
bool Family::isEmpty() const
{
    return parents.isEmpty() && children.isEmpty();
}

void Family::setCreateTime(const QDate& time)
{
    createTime = time;
}
