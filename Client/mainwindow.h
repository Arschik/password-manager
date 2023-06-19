#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>
#include <QtNetwork>
#include <QListWidget>
#include <QTableWidget>
#include <QMenu>
#include <QAction>

class MainWindow : public QWidget
{
	Q_OBJECT

public:
	MainWindow(QJsonObject input_json,QWidget* parent = nullptr);
private:
	QListWidget* group_list;
	QListWidget* account_list;
	QTableWidget* data_table;
	QPushButton* confirm_button;
	QPushButton* open_url_button;
	QHBoxLayout* button_layout;
	QGridLayout* main_layout;

	QMenu* group_menu;
	QMenu* group_item_menu;
	QMenu* account_menu;
	QMenu* account_item_menu;

	QAction* add_group;
	QAction* delete_group;
	QAction* add_account;
	QAction* delete_account;

	QListWidgetItem* selected_group;
	QListWidgetItem* selected_account;

	QNetworkAccessManager* manager;

	QJsonObject json_data;
	QString active_group;
	QString active_account;
	QUrl active_url;
	qint32 user_id;
private:
	void change_passwords_request(QString group);
	void turn_off_table();


private slots:
	void group_selected(QListWidgetItem* item);
	void account_selected(QListWidgetItem* item);
	void confirm_pressed();
	void open_url_pressed();
	void group_menu_requested(const QPoint &pos);
	void add_group_pressed();
	void delete_group_pressed();
	void account_menu_requested(const QPoint &pos);
	void add_account_pressed();
	void delete_account_pressed();

	void reply_finished(QNetworkReply* reply);

};

#endif // MAINWINDOW_H
