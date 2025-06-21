#include"Billing.h"
#include<QFile>
#include<QJsonDocument>
#include<QJsonArray>
#include<QDir>
#include"UserManager.h"
#include"Family.h"
#include"FamilyManager.h"
 Billing::Billing(QObject* parent )
{

}
 Billing& Billing::operator=(const Billing& other)
 {
     if (this == &other) {
         return *this;
     }
 }
 void Billing::clearBillingData()
 {
     billingData.clear();
 }

 QJsonObject Billing::CallRecord::toJson() const
 {
     QJsonObject obj;
     obj["number"] = number;
     obj["caller"] = callerNumber;
     obj["duration"] = duration;
     obj["callTime"] = callTime.toString(Qt::ISODate);
     obj["charge"] = charge;
     obj["wirelessanwerscharge"] = wirelessanwerscharge;
     obj["plan"] = plan;
     obj["service"] = serviceType;
     obj["answered"] = answered;
     obj["isFamilyCall"] = isFamilyCall;
     return obj;
 }
Billing::CallRecord Billing::CallRecord::fromJson(const QJsonObject& json)
 {
    CallRecord record;
    record.number = json["number"].toString();
    record.callerNumber = json["caller"].toString();
    record.duration = json["duration"].toInt();
    record.callTime = QDateTime::fromString(json["callTime"].toString(), Qt::ISODate);
    record.charge = json["charge"].toDouble();
    record.wirelessanwerscharge=json["wirelessanwerscharge"].toDouble();
    record.plan = json["plan"].toString();
    QString serviceType = json["service"].toString();
    record.serviceType = serviceType == "无线" ? PhoneNumber::WIRELESS : PhoneNumber::FIXED;
    record.answered = json["answered"].toBool();
    record.isFamilyCall = json["isFamilyCall"].toBool();
    record.callerrateStrategy = RateStrategyFactory::createStrategy(
        record.serviceType, record.isLongDistance);
    record.answerrateStrategy=RateStrategyFactory::createanswerStrategy(
        record.serviceType, record.answered);
    return record;
 }


QJsonObject Billing::BillingHistory::toJson() const
{
    QJsonObject json;
    json["numberinfo"]= NumberInfo.toJson();
    json["totalcharge"]= TotalCharge;
    QJsonArray recordsArray;
    for (const auto& record : Records) {
        recordsArray.append(record.toJson());
    }
    json["records"] = recordsArray;
    return json;
}


Billing::BillingHistory Billing::BillingHistory::fromJson(const QJsonObject& json)
{
    BillingHistory history;
    history.NumberInfo = PhoneNumber::fromJson(json["numberinfo"].toObject());
    history.TotalCharge = json["totalcharge"].toDouble();
    QJsonArray recordsArray = json["records"].toArray();
    for (const QJsonValue& value : recordsArray) {
        history.Records.append(CallRecord::fromJson(value.toObject()));
    }
    return history;
}

void Billing::processCallRecords(const QString& sourceFile, const QVector<PhoneNumber>& userNumbers)
{
	QFile file(sourceFile);
	if (!file.open(QIODevice::ReadOnly))
	{
		qWarning() << "Failed to open file"<<sourceFile;
		return;
	}


    QMap<QString, PhoneNumber> numberInfo;
    for (const auto& num : userNumbers) {
        numberInfo[num.getNumber()] = num;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonArray recordsArray = doc.array();

    for (const QJsonValue& value : recordsArray) {
        CallRecord record = CallRecord::fromJson(value.toObject());

        if (numberInfo.contains(record.number)) {
            const PhoneNumber& info = numberInfo[record.number];
            record.plan = info.getPlan();
            record.callerrateStrategy = RateStrategyFactory::createStrategy(
                info.getServiceTypeEnum(), record.isLongDistance);
            record.answerrateStrategy = RateStrategyFactory::createanswerStrategy(
                info.getServiceTypeEnum(), record.answered);
        }

        if (record.answerrateStrategy != nullptr && record.answerrateStrategy->getName() == "无线接受电话") {
            record.wirelessanwerscharge = record.answerrateStrategy->calculateCharge(record.duration, record.plan);
        }
        else { record.wirelessanwerscharge = 0; }
        record.charge = record.callerrateStrategy->calculateCharge(record.duration, record.plan);

        //applyPlanDiscount(record.charge, record.plan);

        if (!billingData.contains(record.callerNumber)) {
            billingData[record.callerNumber]->NumberInfo = numberInfo.value(record.callerNumber);
        }
        billingData[record.callerNumber]->Records.append(std::move(record));
        billingData[record.callerNumber]->TotalCharge += record.charge;
        
        if (!billingData.contains(record.number)) {
            billingData[record.number]->NumberInfo = numberInfo.value(record.number);
        }
        if(record.serviceType==PhoneNumber::WIRELESS)
            billingData[record.number]->Records.append(std::move(record));
        billingData[record.number]->TotalCharge += record.wirelessanwerscharge;
    }

    //saveBillingData();
    emit billingUpdated();
}

void Billing::processCallRecords(CallRecord& record)
{
    // 检查是否家庭内通话
    FamilyManager familyMgr;
    familyMgr.loadFamilies();



    CallRecord processedRecord = record;

    if (!billingData.contains(record.callerNumber)) {
        auto history = std::make_shared<BillingHistory>();
        history->NumberInfo = PhoneNumber(record.callerNumber, PhoneNumber::MAIN,
            record.serviceType,
            record.plan);
        billingData[record.callerNumber] = history;

    }
    if (record.isFamilyCall) {
        record.charge = 0;
        record.wirelessanwerscharge = 0;
    }
    billingData[record.callerNumber]->Records.append(record);
    billingData[record.callerNumber]->TotalCharge += record.charge;

    if (!billingData.contains(record.number))
    {
        auto history = std::make_shared<BillingHistory>();
        history->NumberInfo = PhoneNumber(record.number, PhoneNumber::MAIN,
            record.serviceType,
            record.plan);
        billingData[record.number] = history;

    }
    if (record.isFamilyCall) {
        record.charge = 0;
        record.wirelessanwerscharge = 0;
    }
    if (record.serviceType == PhoneNumber::WIRELESS)
    { 
    billingData[record.number]->Records.append(record);
	billingData[record.number]->TotalCharge += record.wirelessanwerscharge;
    }

    //测试数据生成不需要此过程
    // 正式系统需要此过程
    // 
    //// 如果是家庭号码，需要分摊费用
    //if (processedRecord.isFamilyCall) {
    //    familyMgr.updateFamilyCharge(calleeFamily,
    //        processedRecord.number,
    //        processedRecord.charge);

    //    // 获取最新家庭总费用
    //    Family family = familyMgr.getFamily(calleeFamily);
    //    emit familyChargesProcessed(calleeFamily, family.getFamilyTotalCharge());
    //    
    //}

    emit billingUpdated();
}



Billing::BillingHistory Billing::getBillingHistory(const QString& number) const
{
    if (!billingData.contains(number))
    {
        return BillingHistory();
    }
    return *billingData[number];
}



/*
Billing::BillingHistory Billing::getBillingHistory(const QString& number) const
{
    if (!billingData.contains(number))
    {
        return BillingHistory();
    }
    BillingHistory history = *billingData[number];
    //遍历整个记录
    for (auto& History : billingData) {
        for (auto& record : History->Records) {
            if (record.number == number) {
                history.Records.append(record);
            }
        }
    }
    return history;
}*/


QMap<QString, std::shared_ptr<Billing::BillingHistory>>Billing::getAllBillingHistory() const
{
    return billingData;
}





void Billing::loadBillingData()
{
    QFile file("billing_data.json");
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = doc.object();

    for (auto it = root.begin(); it != root.end(); ++it) {
        billingData[it.key()] = std::make_shared<BillingHistory>(
            BillingHistory::fromJson(it.value().toObject())
        );
    }
}
void Billing::saveBillingData()
{
    QFile file("billing_data.json");
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to save billing data";
        return;
    }

    QJsonObject root;
    for (auto it = billingData.begin(); it != billingData.end(); ++it) {
        root[it.key()] = it.value()->toJson();
    }

    file.write(QJsonDocument(root).toJson());
}



//暂时这样写
void Billing::applyPlanDiscount(double& charge, const QString& plan) const
{
    if (plan == "VIP") {
        charge *= 0.9; // 10% discount
    }
    else if (plan == "Family") {
        charge *= 0.8; // 20% discount
    }
    charge = std::round(charge * 100) / 100; // Round to 2 decimal places
}


// 处理家庭费用分摊
void Billing::processFamilyCharges(const QString& familyName, FamilyManager& familyMgr) {
    Family family = familyMgr.getFamily(familyName);
    if (family.isEmpty()) return;

    auto distribution = family.getChargeDistribution();
    for (const auto& [parent, amount] : distribution) {
        if (billingData.contains(parent)) {
            // 添加分摊记录
            CallRecord chargeRecord;
            chargeRecord.number = parent;
            chargeRecord.callerNumber = "System";
            chargeRecord.duration = 0;
            chargeRecord.callTime = QDateTime::currentDateTime();
            chargeRecord.charge = amount;
            chargeRecord.plan = "family";
            chargeRecord.serviceType =PhoneNumber::WIRELESS;
            chargeRecord.answered = true;
            chargeRecord.isFamilyCall = false;

            billingData[parent]->Records.append(chargeRecord);
            billingData[parent]->TotalCharge += amount;
        }
    }

}


// 获取家庭账单
QMap<QString, Billing::BillingHistory> Billing::getFamilyBillingHistory(const QString& familyName) const {
    QMap<QString, BillingHistory> result;
    FamilyManager familyMgr;
    familyMgr.loadFamilies();

    Family family = familyMgr.getFamily(familyName);
    if (family.isEmpty()) return result;

    for (const QString& member : family.getParents() + family.getChildren()) {
        if (billingData.contains(member)) {
            result.insert(member, *billingData[member]);
        }
    }

    return result;
}


void Billing::generateFamilyReport(const Family& family, const QString& fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "无法打开文件:" << fileName;
        return;
    }

    QTextStream out(&file);
    out << generateFamilyHtml(family);
    file.close();
}

QString Billing::generateFamilyHtml(const Family& family) {
    QString html;//用于储存HTML内容
    QDateTime now = QDateTime::currentDateTime();

    // 1. 构建HTML头部和样式
    html = QString(R"(
<!DOCTYPE html>        <!-- 声明文档类型 -->
<html lang="zh-CN">     <!-- 声明语言 -->
<head>                    <!-- 头部 -->
    <meta charset="UTF-8">              <!-- 编码UTF-8 -->
    <meta name="viewport" content="width=device-width, initial-scale=1.0">      <!--设置视口以适应不同设备的屏幕宽度，初始缩放比例为1.0-->
    <title>家庭账单</title>         <!-- 标题 -->
    <style>                         <!-- 样式表 -->
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            font-family: 'Microsoft YaHei', Arial, sans-serif;
        }
        body {
            background-color: #f5f7fa;
            color: #333;
            line-height: 1.6;
            padding: 20px;
        }
        .bill-container {
            max-width: 1000px;
            margin: 0 auto;
            background: white;
            box-shadow: 0 0 20px rgba(0,0,0,0.1);
            border-radius: 10px;
            overflow: hidden;
        }
        .header {
            background: linear-gradient(135deg, #4a7bc8, #2c3e50);
            color: white;
            padding: 30px;
            text-align: center;
        }
        .header h1 {
            font-size: 28px;
            margin-bottom: 5px;
        }
        .header p {
            opacity: 0.9;
        }
        .info-card {
            display: flex;
            justify-content: space-between;
            padding: 20px;
            background: #f8f9fa;
            border-bottom: 1px solid #eee;
        }
        .info-group {
            flex: 1;
            padding: 0 10px;
        }
        .info-label {
            font-weight: 600;
            color: #7f8c8d;
            margin-bottom: 5px;
            font-size: 14px;
        }
        .info-value {
            font-size: 16px;
        }
        .highlight {
            color: #e74c3c;
            font-weight: 600;
        }
        .badge {
            display: inline-block;
            padding: 3px 8px;
            border-radius: 12px;
            font-size: 12px;
            font-weight: 600;
        }
        .badge-primary {
            background: #4a7bc8;
            color: white;
        }
        .badge-success {
            background: #2ecc71;
            color: white;
        }
        table {
            width: 100%;
            border-collapse: collapse;
            margin: 20px 0;
        }
        th {
            background: #34495e;
            color: white;
            padding: 12px;
            text-align: left;
        }
        td {
            padding: 12px;
            border-bottom: 1px solid #eee;
        }
        tr:nth-child(even) {
            background: #f8f9fa;
        }
        .total-row {
            font-weight: 600;
            background: #f1f8fe !important;
        }
        .call-type-family {
            background: #e8f5e9;
        }
        .call-missed {
            color: #e74c3c;
        }
        .footer {
            text-align: center;
            padding: 20px;
            color: #7f8c8d;
            font-size: 12px;
        }
    </style>
</head>
<body>
    <div class="bill-container">
        <div class="header">
            <h1>家庭通话账单</h1>
            <p>账单周期: %1 至 %2</p>
        </div>
    )").arg(now.addMonths(-1).toString("yyyy年MM月dd日")),
    now.toString("yyyy年MM月dd日");
  // 使用.arg()方法替换占位符%1和%2为当前时间的前一个月和当前时间的日期字符串，格式为“yyyy年MM月dd日”

    // 2. 添加家庭基本信息卡片
    html += QString(R"(
        <div class="info-card">
            <div class="info-group">
                <div class="info-label">家庭名称</div>
                <div class="info-value highlight">%1</div>
            </div>
            <div class="info-group">
                <div class="info-label">家长数量</div>
                <div class="info-value">%2人</div>
            </div>
            <div class="info-group">
                <div class="info-label">孩子数量</div>
                <div class="info-value">%3人</div>
            </div>
        </div>
    )").arg(family.getFamilyName())
        .arg(family.getParents().size())
        .arg(family.getChildren().size());

    // 3. 添加家庭成员通话汇总
    html += R"(
        <div style="padding: 0 20px;">
            <h3 style="color: #4a7bc8; margin-top: 20px;">成员通话统计</h3>
            <table class="summary-table">
                <tr>
                    <th>成员</th>
                    <th>角色</th>
                    <th>通话次数</th>
                    <th>总时长</th>
                    <th>总费用</th>
                </tr>
    )";

    double familyTotalCharge = 0;
    //获取电话号码信息
    UserManager userMgr;
    userMgr.loadUsers();
    auto members = family.getParents() + family.getChildren();
    for (const QString& member : members) {
 
        QList<PhoneNumber> numbers = userMgr.getUser(member).getPhoneNumbers();
        for (const PhoneNumber& number : numbers) {
            if (billingData.contains(number.getNumber())) {
                std::shared_ptr<BillingHistory> history = billingData[number.getNumber()];
                int callCount = history->Records.size();
                int totalDuration = 0;
                double memberTotal = 0;

                for (const auto& record : history->Records) {
                    totalDuration += record.duration;
                    memberTotal += record.charge;
                }
                familyTotalCharge += memberTotal;

                html += QString(R"(
                <tr>
                    <td>%1</td>
                    <td><span class="badge %2">%3</span></td>
                    <td>%4</td>
                    <td>%5 分钟</td>
                    <td>¥%6</td>
                </tr>
            )").arg(member)
                    .arg(family.isParent(member) ? "badge-primary" : "badge-success")
                    .arg(family.isParent(member) ? "家长" : "孩子")
                    .arg(callCount)
                    .arg(totalDuration / 60.0, 0, 'f', 1)
                    .arg(memberTotal, 0, 'f', 2);
            }
        }
    }

    // 4. 添加家庭总费用
    html += QString(R"(
                <tr class="total-row">
                    <td colspan="4">家庭总费用</td>
                    <td class="highlight">¥%1</td>
                </tr>
            </table>
        </div>
    )").arg(familyTotalCharge, 0, 'f', 2);

    // 5. 添加详细通话记录（按成员分组）
    html += QString(R"(
        <div style="padding: 0 20px 20px;">
            <h3 style="color: #4a7bc8;">详细通话记录</h3>
    )");

    for (const QString& member : members) {
        QList<PhoneNumber> numbers = userMgr.getUser(member).getPhoneNumbers();
        for (const PhoneNumber& number : numbers) {
            if (billingData.contains(number.getNumber())) {
                html += QString(R"(
                <h4 style="margin: 15px 0 5px; color: #4a7bc8;">%1 (%2)</h4>
                <table class="call-table">
                    <tr>
                        <th>时间</th>
                        <th>对方号码</th>
                        <th>时长</th>
                        <th>状态</th>
                        <th>费用</th>
                        <th>类型</th>
                    </tr>
            )").arg(member).arg(family.isParent(member) ? "家长" : "孩子");

                QList<CallRecord> records = billingData[number.getNumber()]->Records;
                std::sort(records.begin(), records.end(), [](const CallRecord& a, const CallRecord& b) {
                    return a.callTime > b.callTime;
                    });

                for (const CallRecord& record : records) {
                    html += QString(R"(
                    <tr>
                        <td>%1</td>
                        <td>%2</td>
                        <td>%3 秒</td>
                        <td class="%4">%5</td>
                        <td>¥%6</td>
                        <td class="%7">%8</td>
                    </tr>
                )").arg(record.callTime.toString("MM-dd hh:mm"))
                        .arg(record.number)
                        .arg(record.duration)
                        .arg(record.answered ? "" : "call-missed")
                        .arg(record.answered ? "✓ 接通" : "✗ 未接")
                        .arg(record.charge, 0, 'f', 2)
                        .arg(record.isFamilyCall ? "call-type-family" : "")
                        .arg(record.isFamilyCall ? "家庭通话" : "外部通话");
                }

                html += "</table>";
            }
        }
    }

    // 6. 添加费用分摊方案
    html += R"(
            <h3 style="color: #4a7bc8; margin-top: 30px;">费用分摊方案</h3>
            <table class="distribution-table">
                <tr>
                    <th>家长</th>
                    <th>分摊金额</th>
                    <th>已支付</th>
                    <th>状态</th>
                </tr>
    )";

    auto distribution = family.getChargeDistribution();
    for (const auto& [parent, amount] : distribution) {
        double paid = 0.0;
        QList<PhoneNumber> numbers = userMgr.getUser(parent).getPhoneNumbers();
        for (const PhoneNumber& number : numbers) {
            if (billingData.contains(number.getNumber())) {

                QList<CallRecord> records = billingData[number.getNumber()]->Records;
                for (const auto& record : records) {
                    if (record.plan == "family_payment") {
                        paid += record.charge;
                    }

                }
            }
        }

        QString status;
        QString statusClass;
        if (qFuzzyCompare(paid, amount)) {
            status = "已结清";
            statusClass = "badge-success";
        }
        else if (paid > 0) {
            status = QString("部分支付 (¥%1)").arg(paid, 0, 'f', 2);
            statusClass = "badge-warning";
        }
        else {
            status = "待支付";
            statusClass = "badge-danger";
        }

        html += QString(R"(
                <tr>
                    <td>%1</td>
                    <td>¥%2</td>
                    <td>¥%3</td>
                    <td><span class="badge %4">%5</span></td>
                </tr>
        )").arg(parent)
            .arg(amount, 0, 'f', 2)
            .arg(paid, 0, 'f', 2)
            .arg(statusClass)
            .arg(status);
    }

    // 7. 添加页脚
    html += QString(R"(
            </table>
        </div>
        <div class="footer">
            <p>账单生成时间: %1 | 版权所有 © 2023 电话计费系统</p>
        </div>
    )").arg(now.toString("yyyy-MM-dd hh:mm:ss"));

    // 8. 闭合HTML标签
    html += R"(
    </div>
</body>
</html>
    )";

    return html;
}


