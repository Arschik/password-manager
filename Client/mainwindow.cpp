#include "mainwindow.h"
#include "functions.h"
#include <QDesktopServices>

MainWindow::MainWindow(QJsonObject input_json,QWidget* parent)
	: QWidget(parent)
{
	/*
		Конструктор класса MainWindow, который инициализирует главное окно приложения.
		Принимает параметры:
		input_json: объект QJsonObject, содержащий входные данные.
		parent: указатель на родительский виджет (по умолчанию равен nullptr)
	 */
	this->setWindowTitle("Password manager");
	this->setGeometry(0, 0,700,480);
	//this->setFixedSize(700, 480);

	group_list = new QListWidget(this);
	group_list->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
	group_list->setLayoutDirection(Qt::LeftToRight);

	account_list = new QListWidget(this);
	account_list->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

	data_table = new QTableWidget(3,1,this);
	data_table->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
	data_table->horizontalHeader()->setVisible(false);
	data_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	data_table->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	data_table->setVerticalHeaderLabels(QStringList({"URL","login","password"}));
	// Отключаем возможность редактирования строк
	turn_off_table();

	confirm_button = new QPushButton("Confirm changes",this);
	confirm_button->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

	open_url_button = new QPushButton("Open URL",this);
	open_url_button->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

	button_layout = new QHBoxLayout;
	button_layout->addWidget(confirm_button);
	button_layout->addWidget(open_url_button);


	main_layout = new QGridLayout(this);
	main_layout->addWidget(group_list,0,0,3,1);
	main_layout->addWidget(account_list,0,1);
	main_layout->addWidget(data_table,1,1);
	main_layout->addLayout(button_layout,2,1);

	main_layout->setRowStretch(0,63);
	main_layout->setRowStretch(1,24);
	main_layout->setRowStretch(2,8);
	main_layout->setColumnStretch(0,35);
	main_layout->setColumnStretch(1,65);

	// Настройка контекстного меню для левой панели
	group_list->setContextMenuPolicy(Qt::CustomContextMenu);
	group_menu = new QMenu(group_list);
	add_group = new QAction("Add new group",group_menu);
	group_menu->addAction(add_group);

	group_item_menu = new QMenu(group_list);
	delete_group = new QAction("Delete",group_item_menu);
	group_item_menu->addAction(delete_group);

	// Настройка контекстного меню для верхней правой панели
	account_list->setContextMenuPolicy(Qt::CustomContextMenu);
	account_menu = new QMenu(account_list);
	add_account = new QAction("Add account",account_menu);
	account_menu->addAction(add_account);

	account_item_menu = new QMenu(account_list);
	delete_account = new QAction("Delete",account_item_menu);
	account_item_menu->addAction(delete_account);


	// Установка значений по умолчанию
	active_group = "";
	active_account = "";
	manager = new QNetworkAccessManager(this);
	user_id = input_json["id"].toInt();
	json_data = input_json["data"].toObject();
	active_url = QUrl();

	// Добавление групп паролей на левую панель
	QJsonArray group_order = json_data["user's order"].toArray();
	for (QJsonValueRef group : group_order)
	{
		group_list->addItem(group.toString());
	}

	// Соединение сигналов и слотов
	connect(group_list,&QListWidget::itemClicked,this,&MainWindow::group_selected);
	connect(account_list,&QListWidget::itemClicked,this,&MainWindow::account_selected);
	connect(confirm_button,&QPushButton::clicked,this,&MainWindow::confirm_pressed);
	connect(open_url_button,&QPushButton::clicked,this,&MainWindow::open_url_pressed);

	connect(group_list,&QListWidget::customContextMenuRequested,this,&MainWindow::group_menu_requested);
	connect(add_group,&QAction::triggered,this,&MainWindow::add_group_pressed);
	connect(delete_group,&QAction::triggered,this,&MainWindow::delete_group_pressed);

	connect(account_list,&QListWidget::customContextMenuRequested,this,&MainWindow::account_menu_requested);
	connect(add_account,&QAction::triggered,this,&MainWindow::add_account_pressed);
	connect(delete_account,&QAction::triggered,this,&MainWindow::delete_account_pressed);
}

void MainWindow::turn_off_table()
{
	/*
		Отключает возможность редактирования строк в таблице data_table.
		Не принимает параметров и не возвращает значений.
	*/
	for (quint32 i = 0; i < 3; i++)
	{
		QTableWidgetItem* item = new QTableWidgetItem;
		item->setFlags(item->flags() & ~Qt::ItemIsEditable);
		data_table->setItem(i, 0, item);
	}
}

void MainWindow::change_passwords_request(QString group)
{
	/*
		Отправляет запрос на изменение паролей для указанной группы на сервер.
		Принимает параметр:
		group: имя группы паролей, для которой нужно изменить пароли.
		Не возвращает значений.
	*/

	QUrl url;
	url.setScheme("http");
	url.setHost("127.0.0.2");
	url.setPort(80);
	url.setPath("/change_passwords");

	QNetworkRequest request;
	request.setUrl(url);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");

	QJsonObject body;
	body["passwords"] = json_data[group];
	body["group_name"] = group;
	body["id"] = user_id;
	std::string encrypted = aes128_encryption(QJsonDocument(body).toJson());
	QByteArray byte_body(encrypted.c_str());
	manager->put(request,byte_body);
}

void MainWindow::group_selected(QListWidgetItem* item)
{
	/*
		Вызывается при выборе элемента в списке групп паролей.
		Обновляет список аккаунтов в правой панели и устанавливает активную группу.
		Принимает параметр:
		item: указатель на выбранный элемент списка групп паролей.
		Не возвращает значений.
	*/
	if (active_group != item->text())
	{
		account_list->clear();
		active_group = item->text();
		for (QJsonValueRef data : json_data[active_group].toObject()["user's order"].toArray())
		{
			account_list->addItem(data.toString());
		}
	}
}
void MainWindow::account_selected(QListWidgetItem* item)
{
	/*
		Вызывается при выборе элемента в списке аккаунтов.
		Загружает данные аккаунта в таблицу data_table и устанавливает активный аккаунт и URL.
		Принимает параметр:
		item: указатель на выбранный элемент списка аккаунтов.
		Не возвращает значений.
	*/
	if (active_account != item->text())
	{
		active_account = item->text();
		quint32 i = 0;
		QJsonArray account_data = json_data[active_group].toObject()[active_account].toArray();
		active_url.setUrl(account_data[0].toString());
		for (QJsonValueRef data : account_data)
		{
			data_table->setItem(i++,0,new QTableWidgetItem(data.toString()));
		}
	}
}

void MainWindow::confirm_pressed()
{
	/*
		Вызывается при нажатии кнопки "Confirm changes".
		Проверяет, были ли внесены изменения в данные аккаунта, и отправляет изменения на сервер при необходимости.
		Не принимает параметров и не возвращает значений.
	*/
	if (!active_group.isEmpty() && !active_account.isEmpty())
	{
		QJsonArray new_data;

		for (qint32 i = 0; i < 3; i++)
		{
			new_data.append(data_table->item(i,0)->text());
		}
		if (new_data != json_data[active_group].toObject()[active_account].toArray())
		{
			// Изменяем json_data в соответсвии с пользовательским вводом
			QJsonObject changed_group = json_data[active_group].toObject();
			changed_group[active_account] = new_data;
			json_data[active_group] = changed_group;
			active_url.setUrl(json_data[active_group].toObject()[active_account].toArray()[0].toString());

			// Отправляем измененные данные на сервер
			change_passwords_request(active_group);
		}
	}
}
void MainWindow::open_url_pressed()
{
	/*
		Вызывается при нажатии кнопки "Open URL".
		Открывает активный URL в браузере.
	*/
	QDesktopServices::openUrl(active_url);
}

void MainWindow::group_menu_requested(const QPoint& pos)
{
	/*
		Вызывается при вызове контекстного меню
		Открывает контекстное меню в левой панели
		Принимает параметр:
		pos: координаты мыши пользователя в момент вызова меню
	*/
	selected_group = group_list->itemAt(pos);
	if (selected_group)
	{
		group_item_menu->popup(group_list->mapToGlobal(pos));
	}
	else
	{
		group_menu->popup(group_list->mapToGlobal(pos));
	}
}

void MainWindow::add_group_pressed()
{
	/*
		Вызывается при нажатии кнопки "Add Account".
		Открывает диалоговое окно для добавления нового аккаунта.
		Не принимает параметров и не возвращает значений.
	*/

	bool ok = false;
	QString group_name = QInputDialog::getText(this, "Add Group", "Enter group name:", QLineEdit::Normal, "", &ok);
	while (ok)
	{
		QStringList group_names;
		qint32 quantity = group_list->count();
		for (qint32 i = 0; i < quantity; i++)
		{
			group_names.append(group_list->item(i)->text());
		}
		if (group_name.isEmpty())
		{
			QMessageBox::critical(this, "Error", "Group name cannot be empty!");
		}
		else if (group_names.contains(group_name))
		{
			QMessageBox::critical(this, "Error", "The same group already exists!");
		}
		else
		{
			group_list->addItem(group_name);

			// Добавляем новую группу в json_data
			QJsonArray new_order = json_data["user's order"].toArray();
			new_order.append(group_name);
			json_data["user's order"] = new_order;
			QJsonObject new_group;
			new_group["user's order"] = QJsonArray();
			// Отправляем на сервер новую группу
			change_passwords_request(group_name);
			break;
		}

		// Выводим окно ввода имени новой группы ввиду появления ошибок
		group_name = QInputDialog::getText(this, "Add Group", "Enter group name:", QLineEdit::Normal, "", &ok);
	}
}

void MainWindow::delete_group_pressed()
{
	/*
		Вызывается при выборе пункта "Delete" из контекстного меню левой панели
		Удаляет выбранную группу и отправляет данные с запросом об удалении на сервер
		Не принимает параметров и не возвращает значений
	*/
	QMessageBox::StandardButton reply;
	reply = QMessageBox::question(this,"Confirmation","Are you sure you want to delete group \"" + selected_group->text() + "\"?");
	if (reply == QMessageBox::Yes)
	{
		QString selected_name = selected_group->text();
		QJsonArray new_order = json_data["user's order"].toArray();
		for (qint32 i = 0; i < new_order.count(); i++)
		{
			if (new_order[i].toString() == selected_name)
			{
				new_order.removeAt(i);
				json_data["user's order"] = new_order;
				group_list->takeItem(group_list->row(selected_group));
				QUrl url;
				url.setScheme("http");
				url.setHost("127.0.0.2");
				url.setPort(80);
				url.setPath("/delete_group");

				QNetworkRequest request;
				request.setUrl(url);
				request.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");
				QJsonObject body;
				body["id"] = user_id;
				body["group_name"] = selected_name;
				std::string encrypted = aes128_encryption(QJsonDocument(body).toJson());
				QByteArray byte_body(encrypted.c_str());
				manager->put(request,byte_body);

				active_group.clear();
				active_account.clear();
				active_url.clear();
				turn_off_table();
				return;
			}
		}
	}
}

void MainWindow::account_menu_requested(const QPoint& pos)
{
	/*
		Вызывается при вызове контекстного меню в правйо верхней панели
		Открывает контекстное меню в месте, где расположен курсор пользователя
		Принимает параметр:
		pos: координаты мыши пользователя в момент вызова меню
		Не возвращает значений
	*/
	if (!active_group.isEmpty())
	{
		selected_account = account_list->itemAt(pos);
		if (selected_account)
		{
			account_item_menu->popup(account_list->mapToGlobal(pos));
		}
		else
		{
			account_menu->popup(account_list->mapToGlobal(pos));
		}
	}
}
void MainWindow::add_account_pressed()
{
	/*
		Вызывается при выборе действия "add account" из контекстного меню левой верхней панели
		Добавлет новый аккаунт и отправляет на сервер запрос для добавления аккаунта
		Не принимает параметров и не возвращает значений
	*/

	bool ok = false;
	QString account_name = QInputDialog::getText(this,"Add account","Enter account name:",QLineEdit::Normal,"",&ok);
	while (ok)
	{
		QStringList group_names;
		qint32 quantity = account_list->count();
		for (qint32 i = 0; i < quantity; i++)
		{
			group_names.append(account_list->item(i)->text());
		}
		if (account_name.isEmpty())
		{
			QMessageBox::critical(account_list,"Error","Account name con not be empty!");
		}
		else if (group_names.contains(account_name))
		{
			QMessageBox::critical(account_list,"Error","The same account already exists!");
		}
		else
		{
			QJsonObject new_group;
			new_group = json_data[active_group].toObject();
			QJsonArray new_order = new_group["user's order"].toArray();
			new_order.append(account_name);
			new_group[account_name] = QJsonArray({"","",""});
			new_group["user's order"] = new_order;
			account_list->addItem(account_name);
			json_data[active_group] = new_group;
			//qDebug() << json_data;
			change_passwords_request(active_group);
			break;
		}
		account_name = QInputDialog::getText(this,"Add account","Enter account name:",QLineEdit::Normal,"",&ok);
	}

}

void MainWindow::delete_account_pressed()
{
	/*
		Вызывается при выборе пункта "Delete" из контекстного меню из левой верхней панели
		Удаляет аккаунт и отправляет запрос с удалением аккаунта на сервер
	*/
	QString account_name = selected_account->text();
	QMessageBox::StandardButton reply;
	reply = QMessageBox::question(this,"Confirmation","Are you sure you want to delete account \"" + selected_account->text() + "\"?");
	if (reply == QMessageBox::Yes)
	{
		QJsonObject new_group;
		new_group = json_data[active_group].toObject();
		QJsonArray new_order = new_group["user's order"].toArray();

		qint32 quantity = new_order.count();

		for (qint32 i = 0; i < quantity; i++)
		{
			if (new_order[i] == account_name)
			{
				new_order.removeAt(i);
				new_group.remove(account_name);
				account_list->takeItem(account_list->row(selected_account));
				new_group["user's order"] = new_order;
				json_data[active_group] = new_group;
				change_passwords_request(active_group);
				active_account.clear();
				active_url.clear();
				turn_off_table();
				return;
			}
		}
	}
}
