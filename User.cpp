#include "User.h"
#include<QJsonArray>
#include"FamilyManager.h"
User::User(const QString& username, const QString& password, Role role):
	userName(username), passWord(password), role(role){}

QJsonObject User::toJson() const
{
	QJsonObject json;
	json["username"] = userName;
	json["password"] = passWord;
	json["role"] = role;
	json["balance"] = balance;
	QJsonArray numbersArray;
	for (const auto& number : phoneNumbers)
	{
		numbersArray.append(number.toJson());
	}
	json["numbers"] = numbersArray;
	json["familyChargeShare"]= familyChargeShare;
	QJsonArray membersArray;
	json["lastUpdated"]=m_lastUpdate.toString("yyyy-MM-dd HH:mm:ss");
	return json;
}


User User::fromJson(const QJsonObject& json) {
    User user;
    user.userName = json["username"].toString();
    user.passWord = json["password"].toString();
    user.role = static_cast<Role>(json["role"].toInt());
	user.balance = json["balance"].toDouble();

    // 解析电话号码
    QJsonArray numbersArray = json["numbers"].toArray();
    for (const QJsonValue& value : numbersArray) {
        user.phoneNumbers.append(PhoneNumber::fromJson(value.toObject()));
    }
	user.familyChargeShare = json["familyChargeShare"].toDouble();

	user.m_lastUpdate = QDateTime::fromString(json["lastUpdated"].toString(), "yyyy-MM-dd HH:mm:ss");
    return user;
}

QString User::getRoleString() const
{
	switch (role)
	{
	case ADMIN:
		return "管理员";
	case FAMILY_CHILD:
		return "家庭孩子";
	case NORMAL:
		return "普通用户";
	case FAMILY_PARENT:
		return "家庭家长";
	default:
		return "未知";
	}
}


bool User::canManageNumber(const QString& number) const
{
	if (role == ADMIN)
	{
		return true;
	}
	else if (role == FAMILY_PARENT)
	{
		for (const auto& phoneNumber : phoneNumbers)
		{
			if (phoneNumber.getNumber() == number)
			{
				return true;
			}
		}
	}
	else if (role == FAMILY_CHILD)
	{
		for (const auto& phoneNumber : phoneNumbers)
		{
			if (phoneNumber.getNumber() == number && phoneNumber.getServiceTypeEnum() == PhoneNumber::SUB)
				return false;
		}

		for (const auto& phoneNumber : phoneNumbers)
		{
			if (phoneNumber.getNumber() == number)
				return true;
		}
		return false;
	}
}


bool User::canAddNumber() const
{
	return role == ADMIN;
}


void User::addNumber(const PhoneNumber& number)
{
		phoneNumbers.append(number);
}
bool User::removeNumber(const QString& number)
{
	if (isAdmin())
	{
		for (int i = 0; i < phoneNumbers.size(); i++)
		{
			if (phoneNumbers[i].getNumber() == number)
			{
				phoneNumbers.removeAt(i);
				return true;
			}
		}
	}
}

QList<PhoneNumber> User::getNumbersByType(PhoneNumber::Type type) const
{
	QList<PhoneNumber> result;
	std::copy_if(phoneNumbers.begin(), phoneNumbers.end(), std::back_inserter(result),
		[type](const PhoneNumber& num) { return num.getTypeEnum() == type; });
	return result;
}
