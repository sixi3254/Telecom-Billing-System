#include "AddEditUserDialog.h"
#include "ui_AddEditUserDialog.h"
#include<QPushButton>
#include"User.h"
#include<QDialogButtonBox>
#include<QLineEdit>
AddEditUserDialog::AddEditUserDialog(QWidget* parent)
    : QDialog(parent), m_editMode(false)
{

    setupUI();
}






AddEditUserDialog::AddEditUserDialog(const User& user, QWidget* parent)
    : QDialog(parent)
    , m_user(user)
    , m_editMode(true)
{
    setupUI();
    m_familyManager = new FamilyManager(this);
    m_familyManager->loadFamilies();
    // 填充现有用户数据
    ui.usernameLineEdit->setText(user.getUserName());
    ui.usernameLineEdit->setEnabled(false); // 编辑模式下不能修改用户名
    ui.passwordLineEdit->setText(user.getPassWord());
  

    // 设置角色
    int index = ui.roleComboBox->findData(static_cast<int>(user.getRole()));
    if (index != -1) {
        ui.roleComboBox->setCurrentIndex(index);
    }
}


void AddEditUserDialog::setupUI()
{
    ui.setupUi(this);
    setWindowTitle(m_editMode ? tr("编辑用户") : tr("添加用户"));

    populateRoleComboBox();

    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &AddEditUserDialog::onAccept);
    connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // 实时验证输入
    connect(ui.usernameLineEdit, &QLineEdit::textChanged, this, &AddEditUserDialog::validateInput);
    connect(ui.passwordLineEdit, &QLineEdit::textChanged, this, &AddEditUserDialog::validateInput);
    validateInput();
}

void AddEditUserDialog::populateRoleComboBox()
{
    ui.roleComboBox->clear();
    ui.roleComboBox->addItem(tr("普通用户"), static_cast<int>(User::NORMAL));
    ui.roleComboBox->addItem(tr("管理员"), static_cast<int>(User::ADMIN));
    ui.roleComboBox->addItem(tr("家庭子用户"), static_cast<int>(User::FAMILY_CHILD));
}

void AddEditUserDialog::onAccept()
{
    m_user.setUserName(ui.usernameLineEdit->text());
    m_user.setPassword(ui.passwordLineEdit->text());
    m_user.setRole(static_cast<User::Role>(ui.roleComboBox->currentData().toInt()));

    accept();
}

void AddEditUserDialog::validateInput()
{
    bool valid = !ui.usernameLineEdit->text().isEmpty() &&
        !ui.passwordLineEdit->text().isEmpty();

    ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(valid);
}
