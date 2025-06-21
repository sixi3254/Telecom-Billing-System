#include "LoginDialog.h"
#include"QFile"
#include"QJsonDocument"
#include"QJsonObject"
#include"QJsonArray"
#include"QMessageBox"
#include<QRegularExpression>
#include<QSettings>
#include<QCoreApplication>
LoginDialog::LoginDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	// 设置窗口属性
	setWindowTitle("电信计费系统 - 登录");
	setFixedSize(400, 300);

	// 加载用户信息
	loadUsers();

	// 信号槽连接
	connect(ui.loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
	connect(ui.registerButton, &QPushButton::clicked, this, &LoginDialog::onRegisterClicked);
	connect(ui.showPasswordCheckBox, &QCheckBox::toggled,
		this, &LoginDialog::togglePasswordVisibility);

	QCoreApplication::setOrganizationName("keshi");
	QCoreApplication::setApplicationName("TelecomBillingSystem");

	loadRememberedUser();
	

}

LoginDialog::~LoginDialog()
{}


void LoginDialog::loadUsers()
{
	QFile file("users.json");
	if (!file.open(QIODevice::ReadOnly))
	{
		User admin("admin", "Admin123", User::ADMIN);
		users.append(admin);
		saveUsers();
		return;
	}
	QJsonDocument doc=QJsonDocument::fromJson(file.readAll());
	QJsonArray usersArray=doc.array();
	for (const QJsonValue& value : usersArray)
	{
		users.append(User::fromJson(value.toObject()));
	}
	file.close();
}



void LoginDialog::saveUsers()
{
	QFile file("users.json");
	if (!file.open(QIODevice::WriteOnly))
	{
		showError("无法保存用户信息！");
		return;
	}
	QJsonArray usersArray;
	for (const User& user : users)
	{
		usersArray.append(user.toJson());
	}
	QJsonDocument doc(usersArray);
	file.write(doc.toJson());
	file.close();

}


bool LoginDialog::validateUser(const QString& username, const QString& password)
{
	for (const User& user : users)
	{
		if (user.getUserName() == username && user.getPassWord() == password)
		{
			loggedInUser = user;
			return true;
		}
	}
	return false;
}


bool LoginDialog::registerUser(const QString& username, const QString& password, User::Role role)
{
	User newUser(username, password, role);

	users.append(newUser);
	saveUsers();
	return true;
}

void LoginDialog::onRegisterClicked()
{
	QString username = ui.usernameEdit->text().trimmed();
	QString password = ui.passwordEdit->text();
	if (username.isEmpty() || password.isEmpty())
	{
		showError("用户名或密码不能为空！");
		return;
	}
	//用户格式验证
	QRegularExpression usernameRegex("^[a-zA-Z0-9_]{4,16}$");//正则表达式
	if (!usernameRegex.match(username).hasMatch())
	{
		showError("用户名必须是4-16位字母、数字或下划线");
		return;
	}
	//密码检验
	QRegularExpression passwordRegex("^(?=.*[a-z])(?=.*[A-Z])(?=.*\\d)[a-zA-Z\\d]{8,}$");//正则表达式
	if (!passwordRegex.match(password).hasMatch())
	{
		showError("密码必须包含至少8个字符，至少1个大写字母，1个小写字母和1个数字");
		return;
	}
	//检验用户名是否已存在
	for (const User& user : users)
	{
		if (user.getUserName() == username)
		{
			showError("用户名已存在！");
			return;
		}
	}
	//注册用户
	if (registerUser(username, password, User::NORMAL))
	{
		QMessageBox::information(this, "注册成功", "账户注册成功，请登录");
		clearInputs();
	}

}

void LoginDialog::showError(const QString& message)
{
	QMessageBox::warning(this, "错误", message);
	ui.passwordEdit->clear();
	ui.passwordEdit->setFocus();

}
void LoginDialog::clearInputs()
{
	// 不清除记住的用户名
	if (!ui.rememberCheckBox->isChecked()) {
		ui.usernameEdit->clear();
	}
	ui.passwordEdit->clear();

	// 设置焦点
	if (ui.usernameEdit->text().isEmpty()) {
		ui.usernameEdit->setFocus();
	}
	else {
		ui.passwordEdit->setFocus();
	}
}


void LoginDialog::togglePasswordVisibility()
{
	ui.passwordEdit->setEchoMode(
		ui.showPasswordCheckBox->isChecked() ?
		QLineEdit::Normal : QLineEdit::Password
	);
}

void LoginDialog::onLoginClicked()
{
	QString username = ui.usernameEdit->text().trimmed();
	QString password = ui.passwordEdit->text();
	if (username.isEmpty() || password.isEmpty())
	{
		showError("用户名或密码不能为空！");
		return;
	}

	saveRememberedUser();

	if (validateUser(username, password))
	{
		QMessageBox::information(this, "登录成功", "欢迎回来，" + loggedInUser.getUserName());
		accept();
	}
	else
	{
		showError("用户名或密码错误！");
		clearInputs();
	}
}

void LoginDialog::loadRememberedUser()
{
	// 创建一个QSettings对象，用于处理应用程序的设置
	QSettings settings;

	// 开始读取“LoginSettings”组中的设置
	settings.beginGroup("LoginSettings");

	// 读取“rememberUser”键的值，如果该键不存在，则默认为false
	bool remember = settings.value("rememberUser", false).toBool();

	// 读取“username”键的值，并将其转换为QString类型
	QString username = settings.value("username").toString();

	// 结束读取“LoginSettings”组中的设置
	settings.endGroup();

	// 根据设置中的“rememberUser”值，设置ui中的复选框状态
	ui.rememberCheckBox->setChecked(remember);

	// 如果“rememberUser”为true且“username”不为空，则执行以下操作
	if (remember && !username.isEmpty()) {
		// 将“username”设置为用户名编辑框的文本
		ui.usernameEdit->setText(username);

		// 将焦点设置到密码编辑框，以便用户可以直接输入密码
		ui.passwordEdit->setFocus();
	}
	// 如果“rememberUser”为false或“username”为空，则执行以下操作
	else {
		// 将焦点设置到用户名编辑框，以便用户可以输入用户名
		ui.usernameEdit->setFocus();
	}
}

void LoginDialog::saveRememberedUser() // 定义一个名为saveRememberedUser的成员函数，用于保存用户登录信息
{
	QSettings settings; // 创建一个QSettings对象，用于处理应用程序的配置信息
	settings.beginGroup("LoginSettings"); // 开始处理名为"LoginSettings"的配置组

	if (ui.rememberCheckBox->isChecked()) { // 检查ui.rememberCheckBox复选框是否被选中
		settings.setValue("username", ui.usernameEdit->text()); // 如果被选中，则将用户名编辑框中的文本保存到配置中，键名为"username"
		settings.setValue("rememberUser", true); // 同时设置"rememberUser"键的值为true，表示记住用户
	}
	else { // 如果复选框未被选中
		settings.remove("username"); // 移除配置中的"username"键，即不保存用户名
		settings.setValue("rememberUser", false); // 设置"rememberUser"键的值为false，表示不记住用户
	}

	settings.endGroup(); // 结束对"LoginSettings"配置组的处理
}
