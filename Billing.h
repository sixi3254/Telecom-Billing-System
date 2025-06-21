#pragma once
#include <QObject>
#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QVector>
#include <QMap>
#include <memory>
#include "PhoneNumber.h"
#include "RateStrategy.h"
#include "RateStrategyFactory.h"
#include"FamilyManager.h"
#include"Family.h"

class Billing : public QObject {
    Q_OBJECT
public:
    struct CallRecord {
        QString number;//被叫号码
        std::shared_ptr<RateStrategy> callerrateStrategy;//计费策略
        std::shared_ptr<RateStrategy> answerrateStrategy;
        int duration;
        QDateTime callTime;
        double charge;
        double wirelessanwerscharge;
        QString plan;
        PhoneNumber::ServiceType serviceType;
        bool isLongDistance;
 

        bool answered;         // 新增是否接通状态
        bool isFamilyCall;     // 新增是否家庭内通话
        QString callerNumber;  // 新增主叫号码

        QJsonObject toJson() const;
        static CallRecord fromJson(const QJsonObject& json);
    };
    struct BillingHistory {
        PhoneNumber NumberInfo;
        QList<CallRecord> Records;
        double TotalCharge;
        
        QJsonObject toJson() const;
        static BillingHistory fromJson(const QJsonObject& json);
    };


    explicit Billing(QObject* parent = nullptr);

    Billing& operator=(const Billing& other);

    void processCallRecords(const QString& sourceFile, const QVector<PhoneNumber>& userNumbers);
    void processCallRecords( CallRecord& record);
    BillingHistory getBillingHistory(const QString& number) const;
    QMap<QString, std::shared_ptr<BillingHistory>>getAllBillingHistory() const;
        
    void saveBillingData();
    void loadBillingData();
    void applyPlanDiscount(double& charge, const QString& plan) const;  
    void clearBillingData();

    // 新增家庭费用处理方法
    void processFamilyCharges(const QString& familyName, FamilyManager& familyMgr);

    // 新增家庭账单查询
    QMap<QString, BillingHistory> getFamilyBillingHistory(const QString& familyName) const;

    void generateFamilyReport(const Family& family, const QString& fileName);
    QString generateFamilyHtml(const Family& family);


signals:
    void billingUpdated();
    void familyChargesProcessed(const QString& familyName, double totalCharge);

private:
    
    QMap<QString, std::shared_ptr<BillingHistory>> billingData;//前缀为用户号码，值为账单历史记录
    

};