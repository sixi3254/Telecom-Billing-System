#pragma once
#include<QString>
#include<QJsonObject>
class PhoneNumber
{
public:
	enum Type{MAIN,SUB,FAMILY};
	enum ServiceType{FIXED,WIRELESS};
	PhoneNumber() = default;
	PhoneNumber(const QString& number, Type type, ServiceType serviceType, const QString& plan = "standard");

	//序列化方法
	QJsonObject toJson() const;
	static PhoneNumber fromJson(const QJsonObject& json);

	//获取方法
	QString getNumber() const;
	QString getType() const;
	QString getServiceType() const;
	QString getPlan() const;
	ServiceType getServiceTypeEnum() const;
	Type getTypeEnum() const;

	//检验
	bool isValid() const;
	static bool validateNumberFormat(const QString& number);

	bool operator==(const QString& other) const {
		return this->number == other;
	}


	bool operator==(const PhoneNumber& other) const {
		return this->number == other.number;
	}

private:
	QString number;
	Type type=MAIN;
	ServiceType serviceType=FIXED;
	QString plan = "standard";
};

