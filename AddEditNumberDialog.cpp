#include "AddEditNumberDialog.h"
#include "ui_AddEditNumberDialog.h"
#include <QMessageBox>
#include <QRegularExpressionValidator>
#include<QLineEdit>
#include<QPushButton>
AddEditNumberDialog::AddEditNumberDialog(QWidget* parent)
    : QDialog(parent)
    , m_editMode(false)
{
    setupUI();
}

AddEditNumberDialog::AddEditNumberDialog(const PhoneNumber& number, QWidget* parent)
    : QDialog(parent)
    , m_number(number)
    , m_editMode(true)
{
    setupUI();

    // 填充现有号码数据
    ui.numberLineEdit->setText(number.getNumber());
    ui.planLineEdit->setText(number.getPlan());

    // 设置类型
    int typeIndex = ui.typeComboBox->findData(static_cast<int>(number.getTypeEnum()));
    if (typeIndex != -1) {
        ui.typeComboBox->setCurrentIndex(typeIndex);
    }

    // 设置服务类型
    int serviceIndex = ui.serviceTypeComboBox->findData(static_cast<int>(number.getServiceTypeEnum()));
    if (serviceIndex != -1) {
        ui.serviceTypeComboBox->setCurrentIndex(serviceIndex);
    }
}

AddEditNumberDialog::~AddEditNumberDialog()
{

}

void AddEditNumberDialog::setupUI()
{
    ui.setupUi(this);
    setWindowTitle(m_editMode ? tr("编辑电话号码") : tr("添加电话号码"));

    populateTypeComboBox();
    populateServiceTypeComboBox();

    // 设置电话号码验证器
    QRegularExpressionValidator* validator = new QRegularExpressionValidator(
        QRegularExpression("\\d{7,15}"), this); // 7-15位数字
    ui.numberLineEdit->setValidator(validator);

    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &AddEditNumberDialog::onAccept);
    connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // 实时验证输入
    connect(ui.numberLineEdit, &QLineEdit::textChanged, this, &AddEditNumberDialog::validateInput);
    validateInput();
}

void AddEditNumberDialog::populateTypeComboBox()
{
    ui.typeComboBox->clear();
    ui.typeComboBox->addItem(tr("主号码"), static_cast<int>(PhoneNumber::MAIN));
    ui.typeComboBox->addItem(tr("副号码"), static_cast<int>(PhoneNumber::SUB));
    ui.typeComboBox->addItem(tr("家庭号码"), static_cast<int>(PhoneNumber::FAMILY));
}

void AddEditNumberDialog::populateServiceTypeComboBox()
{
    ui.serviceTypeComboBox->clear();
    ui.serviceTypeComboBox->addItem(tr("固话"), static_cast<int>(PhoneNumber::FIXED));
    ui.serviceTypeComboBox->addItem(tr("无线"), static_cast<int>(PhoneNumber::WIRELESS));
}

PhoneNumber AddEditNumberDialog::getPhoneNumber() const
{
    return m_number;
}

void AddEditNumberDialog::onAccept()
{
    QString number = ui.numberLineEdit->text();
    if (!PhoneNumber::validateNumberFormat(number)) {
        QMessageBox::warning(this, tr("错误"), tr("电话号码格式不正确"));
        return;
    }

    m_number = PhoneNumber(
        number,
        static_cast<PhoneNumber::Type>(ui.typeComboBox->currentData().toInt()),
        static_cast<PhoneNumber::ServiceType>(ui.serviceTypeComboBox->currentData().toInt()),
        ui.planLineEdit->text()
    );

    accept();
}

void AddEditNumberDialog::validateInput()
{
    bool valid = ui.numberLineEdit->hasAcceptableInput() &&
        !ui.planLineEdit->text().isEmpty();

    ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(valid);
}
