#include "greeting.h"
#include "functions.h"
#include "mainwindow.h"
Greeting::Greeting(QWidget* parent)
	: QWidget(parent)
{
	// Задаем параметры окна
	this->setWindowTitle("Authentication");
	this->setGeometry(0,0,250,280);
	this->setFixedSize(250,280);

	// Настраиваем кнопки аутентификации, регистрации
	button_layout = new QHBoxLayout();
	button_layout->setSpacing(0);
	sign_in_button = new QPushButton("Sign in", this);
	sign_in_button->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
	sign_up_button = new QPushButton("Sign up", this);
	sign_up_button->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
	button_layout->addWidget(sign_in_button);
	button_layout->addWidget(sign_up_button);

	// Настраиваем поля ввода
	line_layout = new QVBoxLayout();
	line_layout->setSpacing(12);
	name_line = new QLineEdit(this);
	name_line->setPlaceholderText("Enter your name");
	name_line->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
	password_line = new QLineEdit(this);
	password_line->setPlaceholderText("Enter your password");
	password_line->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
	confirm_line = new QLineEdit(this);
	confirm_line->setPlaceholderText("Retype your password");
	confirm_line->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
	line_layout->addWidget(name_line);
	line_layout->addWidget(password_line);
	line_layout->addWidget(confirm_line);

	// Настраиваем надпись для сообщения об ошибке
	error_label = new QLabel(this); // Создаем QLabel
	error_label->setStyleSheet("QLabel { color : red; }");
	error_label->hide();
	error_label->setAlignment(Qt::AlignCenter);
	line_layout->addWidget(error_label);
	line_layout->setContentsMargins(0,25,0,40);

	// Настариваем кнопку подтверждения
	confirm_button = new QPushButton("Confirm", this);
	confirm_button->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Expanding);
	confirm_button->setMinimumWidth(150);

	// Настриваем основной шаблон
	main_layout = new QVBoxLayout(this);
	main_layout->addLayout(button_layout);
	main_layout->addLayout(line_layout);
	main_layout->addWidget(confirm_button,0,Qt::AlignRight);
	main_layout->setStretchFactor(button_layout, 15);
	main_layout->setStretchFactor(line_layout, 70);
	main_layout->setStretchFactor(confirm_button, 15);
	main_layout->setContentsMargins(3,3,3,3);


	// Устанавливаем режим аутентификации по умолчанию
	confirm_line->hide();
	mode = 'a';

	// Инициализируем сетевого менеджера
	manager = new QNetworkAccessManager(this);


	// Соединяем сигналы со слотами
	connect(sign_in_button, &QPushButton::clicked, this, &Greeting::sign_in_clicked);
	connect(sign_up_button, &QPushButton::clicked, this, &Greeting::sign_up_clicked);
	connect(confirm_button, &QPushButton::clicked, this, &Greeting::confirm_clicked);
	connect(manager, &QNetworkAccessManager::finished, this, &Greeting::reply_finished);
}

void Greeting::sign_in_clicked()
{
	confirm_line->hide();
	mode = 'a';
	error_label->hide();
}

void Greeting::sign_up_clicked()
{
	confirm_line->show();
	mode = 'r';
	if (error_label->text() != "Passwords don't match!")
	{
		error_label->hide();
	}
}

void Greeting::confirm_clicked()
{
	if (name_line->text() != "" && password_line->text() != "")
	{
		QUrl url;
		if (mode == 'r') // Режим регистрации
		{
			if (confirm_line->text() == "")
			{
				error_label->setText("No field can be empty");
				error_label->show();
				return;
			}
			else if (password_line->text() != confirm_line->text()) // Если пароли не совпадают
			{
				error_label->setText("Passwords don't match!");
				error_label->show();
				return;
			}
			else
			{
				url.setPath("/registration");
			}
		}
		else // Режим аутентификации
		{
			url.setPath("/authentication");
		}
		url.setScheme("http");
		url.setHost("127.0.0.2");
		url.setPort(80);
		QJsonObject body;
		body["name"] = name_line->text();
		body["password"] = password_line->text();
		QNetworkRequest request;
		request.setUrl(url);
		request.setHeader(QNetworkRequest::ContentTypeHeader,"text/plain");

		std::string encrypted = aes128_encryption(QJsonDocument(body).toJson());
		QByteArray byte_body(encrypted.c_str());

		manager->put(request,byte_body);
	}
	else
	{
		error_label->setText("No field can be empty");
		error_label->show();
	}
}

void Greeting::reply_finished(QNetworkReply* reply)
{

	QJsonObject body (QJsonDocument::fromJson(aes128_decryption(reply->readAll().toStdString())).object());
	qint32 status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	qDebug() << body;
	if (status == 200)
	{
		MainWindow* mainWindow = new MainWindow(body,nullptr);
		mainWindow->show();
		this->close();
	}

	else if (status == 401)
	{
		error_label->setText("Incorrect data was entered!");
		error_label->show();
	}
	else if (status == 409)
	{
		error_label->setText("The login is already exists!");
		error_label->show();
	}
}
