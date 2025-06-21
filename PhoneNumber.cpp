#include "PhoneNumber.h"

PhoneNumber::PhoneNumber(const QString& number, Type type, ServiceType serviceType, const QString& plan ):
	number(number), type(type), serviceType(serviceType), plan(plan){}
//获取函数
QString PhoneNumber::getNumber() const{return number;}
QString PhoneNumber::getPlan() const{return plan;}
PhoneNumber::Type PhoneNumber::getTypeEnum() const{return type;}

QString PhoneNumber::getServiceType() const
{
	switch (type) {
	case MAIN: return "主号";
	case SUB: return "副号";
	case FAMILY: return "家庭号";
	default: return "未知类型";
	}
}

QString PhoneNumber::getType() const {
	switch (serviceType) {
	case FIXED: return "固定";
	case WIRELESS: return "无线";
	default: return "未知类型";
	}
}

//序列化
QJsonObject PhoneNumber::toJson() const
{
	QJsonObject json;
	json["number"] = number;
	json["type"] = type;
	json["serviceType"] = serviceType;
	json["plan"] = plan;
	return json;
}

PhoneNumber PhoneNumber::fromJson(const QJsonObject& json)
{
		PhoneNumber phoneNumber;
		phoneNumber.number = json["number"].toString();
		phoneNumber.type = static_cast<Type>(json["type"].toInt());
		phoneNumber.serviceType = static_cast<ServiceType>(json["serviceType"].toInt());
		phoneNumber.plan = json["plan"].toString();
		return phoneNumber;
}

bool PhoneNumber::isValid() const
{
	return validateNumberFormat(number) && !plan.isEmpty();
}



// 正则表达式！！！
bool PhoneNumber::validateNumberFormat(const QString& number)
{
	// 中国手机号验证: 1开头，11位数字
	QRegularExpression mobileRegex("^1[3-9]\\d{9}$");

	// 固定电话验证: 区号(3-4位)-号码(7-8位)
	QRegularExpression fixedRegex("^(0\\d{2,3}-)?[2-9]\\d{6,7}$");

	return mobileRegex.match(number).hasMatch() ||
		fixedRegex.match(number).hasMatch();
}

PhoneNumber::ServiceType PhoneNumber::getServiceTypeEnum() const
{
	return serviceType;
}
