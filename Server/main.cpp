#include "functions.h"

int main(int argc, char *argv[])
{
	QCoreApplication app (argc, argv);
	// Изменяем кодировку в консоли на UTF-8
	SetConsoleOutputCP(CP_UTF8);

	// Устанавливаем сервер на 80 порт
	QHttpServer server = QHttpServer(nullptr);

	if (server.listen(QHostAddress::Any, 80))
	{
		qDebug() << "Server is listening port 80";
	}
	else
	{
		return -1;
	}

	// Подключаемся к базе данных
	QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
	db.setHostName("localhost");
	db.setPort(5432);
	db.setDatabaseName("password manager");
	db.setUserName("postgres");
	db.setPassword("admin");

	if (!db.open())
	{
		qDebug() << "Failed to connect to database:" << db.lastError().text();
		return -1;
	}
	else
	{
		qDebug() << "Successfully connected to database";
	}

	// Создаем множество авторизованных пользователей
	QSet<int> auth_id;


	// Получаем значение последнего идентификатора
	QSqlQuery query ("SELECT id FROM \"user\" ORDER BY id DESC LIMIT 1");

	query.next();

	qint32 last_id = query.value(0).toInt();


	server.route("/",[]()
	{
		QHttpServerResponse::StatusCode status = QHttpServerResponse::StatusCode::Ok;
		QString body = "Hey, Body!";
		QHttpServerResponse response(body,status);

		return response;
	});

	// Регистрация пользователя
	server.route("/registration",[&last_id, &auth_id] (const QHttpServerRequest& request)
	{
		// Считываем данные из запроса
		QJsonObject userdata = QJsonDocument::fromJson(aes128_decryption(request.body().toStdString())).object();
		QString name = userdata["name"].toString();
		QString password = sha256_hash(userdata["password"].toString());

		// Проверяем, существует ли пользователь с таким же именем
		QSqlQuery query;
		query.prepare("SELECT * FROM \"user\" WHERE name = :name");
		query.bindValue(":name",name);
		query.exec();
		if (query.next())
		{
			// Отправляем сообщение с ошибкой
			QHttpServerResponse::StatusCode status = QHttpServerResponse::StatusCode::Conflict;
			QJsonObject body;
			body["error"] = "The username already exists";
			std::string cryptobody = aes128_encryption(QJsonDocument(body).toJson());
			return QHttpServerResponse(QString(cryptobody.c_str()),status);
		}

		else
		{
			// Создаем нового пользователя
			query.clear();
			query.prepare("INSERT INTO \"user\" VALUES (:id, :name, :password)");
			query.bindValue(":id", ++last_id);
			query.bindValue(":name", name);
			query.bindValue(":password", password);
			query.exec();

			QHttpServerResponse::StatusCode status = QHttpServerResponse::StatusCode::Ok;
			QJsonObject body;
			auth_id.insert(last_id);
			body["id"] = last_id;
			body["user's order"] = QJsonArray();
			body["passwords"] = QJsonObject();
			std::string cryptobody = aes128_encryption(QJsonDocument(body).toJson());
			return QHttpServerResponse(QString(cryptobody.c_str()),status);
		}
	});

	// Аутентификация пользователя
	server.route("/authentication",[&auth_id] (const QHttpServerRequest& request)
	{
		// Получение данных пользователя из запроса
		QJsonObject userdata = QJsonDocument::fromJson(aes128_decryption(request.body().toStdString())).object();
		QString name = userdata["name"].toString();
		QString password = sha256_hash(userdata["password"].toString());

		// Проверяем валидность данных пользователя
		QSqlQuery query;
		query.prepare("SELECT * FROM \"user\" WHERE name = :name AND password = :password");
		query.bindValue(":name", name);
		query.bindValue(":password", password);
		query.exec();

		if (query.next())
		{
			// Включаем id пользователя в список авторизированных пользвателей
			qint32 user_id = query.value(0).toInt();
			auth_id.insert(user_id);

			// Загружаем из базы данных все группы паролей пользвоателя
			query.clear();
			query.prepare("SELECT * FROM user_passwords WHERE user_id = :user_id");
			query.bindValue(":user_id",user_id);
			query.exec();
			QJsonObject passwords_body;
			QJsonArray order;
			while (query.next())
			{
				QString group_name = query.value(1).toString();
				std::string passwords_json = query.value(2).toString().toStdString();
				passwords_body[group_name] = QJsonDocument::fromJson(aes128_decryption(passwords_json)).object();
				order.append(group_name);
			}
			passwords_body["user's order"] = order;
			QJsonObject body;
			body["id"] = user_id;
			body["data"] = passwords_body;
			QHttpServerResponse::StatusCode status = QHttpServerResponse::StatusCode::Ok;
			std::string cryptobody = aes128_encryption(QJsonDocument(body).toJson());
			qDebug() << body;
			return QHttpServerResponse(QString(cryptobody.c_str()),status);
		}
		else
		{
			// Отправляем сообщение с ошибкой аутентификации
			QHttpServerResponse::StatusCode status = QHttpServerResponse::StatusCode::Unauthorized;
			QJsonObject body;
			body["error"] = "Incorrect data was entered";
			std::string cryptobody = aes128_encryption(QJsonDocument(body).toJson());
			return QHttpServerResponse (QString(cryptobody.c_str()),status);
		}
	});
	server.route("/change_passwords",[&auth_id](const QHttpServerRequest& request)
	{
		QJsonObject userdata = QJsonDocument::fromJson(aes128_decryption(request.body().toStdString())).object();
		qint32 user_id = userdata["id"].toInt();
		QString group_name = userdata["group_name"].toString();
		QJsonObject passwords = userdata["passwords"].toObject();
		std::string encrypted_passwords = aes128_encryption(QJsonDocument(passwords).toJson());

		qDebug() << "Change passwords input data:";
		qDebug() << user_id;
		qDebug() << group_name;
		qDebug() << passwords;

		QSqlQuery query;
		if (auth_id.contains(user_id))
		{
			query.prepare("INSERT INTO user_passwords VALUES (:user_id, :group_name, :passwords)"
						  "ON CONFLICT (group_name) DO UPDATE SET passwords = EXCLUDED.passwords");
			query.bindValue(":user_id", user_id);
			query.bindValue(":group_name", group_name);
			query.bindValue(":passwords", QString(encrypted_passwords.c_str()));
			query.exec();

			QHttpServerResponse::StatusCode status = QHttpServerResponse::StatusCode::Ok;
			QJsonObject body;
			body["message"] = "New data was stored";
			std::string cryptobody = aes128_encryption(QJsonDocument(body).toJson());
			return QHttpServerResponse(QString(cryptobody.c_str()),status);
		}
		else
		{
			QHttpServerResponse::StatusCode status = QHttpServerResponse::StatusCode::Forbidden;
			QJsonObject body;
			body["error"] = "You have not authorized!";
			std::string cryptobody = aes128_encryption(QJsonDocument(body).toJson());
			return QHttpServerResponse(QString(cryptobody.c_str()),status);
		}
	});
	server.route("/delete_group",[&auth_id](const QHttpServerRequest& request)
	{
		QJsonObject userdata = QJsonDocument::fromJson(aes128_decryption(request.body().toStdString())).object();
		qint32 user_id = userdata["id"].toInt();
		QString group_name = userdata["group_name"].toString();

		QSqlQuery query;
		if (auth_id.contains(user_id))
		{
			query.prepare("DELETE FROM user_passwords WHERE group_name = :group_name AND user_id = :user_id");
			query.bindValue(":group_name",group_name);
			query.bindValue(":user_id",user_id);
			query.exec();
			QHttpServerResponse::StatusCode status = QHttpServerResponse::StatusCode::Ok;
			QJsonObject body;
			body["message"] = "Group " + group_name + " was successfuly deleted";
			std::string cryptobody = aes128_encryption(QJsonDocument(body).toJson());
			return QHttpServerResponse(QString(cryptobody.c_str()),status);
		}
		else
		{
			QHttpServerResponse::StatusCode status = QHttpServerResponse::StatusCode::Forbidden;
			QJsonObject body;
			body["error"] = "You have not authorized!";
			std::string cryptobody = aes128_encryption(QJsonDocument(body).toJson());
			return QHttpServerResponse(QString(cryptobody.c_str()),status);
		}
	});
	return app.exec();
}
