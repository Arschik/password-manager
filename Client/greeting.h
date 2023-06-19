#ifndef GREETING_H
#define GREETING_H

#include <QtWidgets>
#include <QtNetwork>
#include <QJsonDocument>
#include <QJsonObject>

class Greeting : public QWidget
{
	Q_OBJECT

public:
	Greeting(QWidget *parent = nullptr);


private:
	QHBoxLayout* button_layout;
	QPushButton* sign_in_button;
	QPushButton* sign_up_button;
	QVBoxLayout* line_layout;
	QLineEdit* name_line;
	QLineEdit* password_line;
	QLineEdit* confirm_line;
	QLabel* error_label;
	QPushButton* confirm_button;
	QVBoxLayout* main_layout;

	QChar mode;

	QNetworkAccessManager* manager;
private slots:
	void sign_in_clicked();
	void sign_up_clicked();
	void confirm_clicked();
	void reply_finished(QNetworkReply* reply);


};
#endif // GREETING_H
