#pragma once
#include <QString>
#include<QList>
#include<QJsonObject>
#include "PhoneNumber.h"
#include<QSet>
class User
{
public:
	enum Role{ NORMAL, ADMIN, FAMILY_CHILD, FAMILY_PARENT };

	User()=default;  //要求编译器自动生成默认构造函数
	User(const QString& username, const QString& password, Role role);

	//序列化方法
	QJsonObject toJson() const;
	static User fromJson(const QJsonObject& json);

	//权限检查
	bool canManageNumber(const QString& number) const;
	bool isAdmin() const{return role==ADMIN;}
	bool canAddNumber() const;

	//操作
	void addNumber(const PhoneNumber& number);
	bool removeNumber(const QString& number);

	void setRole(Role role){this->role=role;}
	void setUserName(const QString& username){userName=username;}
	void setPassword(const QString& password){passWord=password;}
	void setPhoneNumbers(const QList<PhoneNumber>& numbers) {
		phoneNumbers = numbers;
	}
	// 更新时间戳（精确到毫秒）
	void setLastUpdate(const QDateTime& timestamp) {
		m_lastUpdate = timestamp.toUTC(); // 统一存储为UTC时间
	}

	// 余额管理
	void setBalance(double balance) { this->balance = balance; }
	void addBalance(double amount) { balance += amount; }
	double getBalance() const { return balance; }

	double getFamilyChargeShare() const { return familyChargeShare; }
	void setFamilyChargeShare(double share) { familyChargeShare = share; }



	//获取信息
	const QString& getUserName() const{return userName;}
	const QString& getPassWord() const{return passWord;}
	const QList<PhoneNumber>& getPhoneNumbers() const{return phoneNumbers;}
	Role getRole() const{return role;}
	QDateTime getLastUpdate() const {
		return m_lastUpdate.toLocalTime(); // 返回本地时间
	}
	QDateTime getLastUpdateUTC() const {
		return m_lastUpdate;
	}
	QString getRoleString() const;

	// 获取号码类型
	QList<PhoneNumber> getNumbersByType(PhoneNumber::Type type) const;
private:
	QString userName;
	QString passWord;
	QDateTime m_lastUpdate;
	double balance = 0.0;//余额
	Role role=NORMAL;
	QList<PhoneNumber> phoneNumbers;
	bool isFamilyParentFlag = false;
	double familyChargeShare = 0.0;//家庭共享费用

	

};

