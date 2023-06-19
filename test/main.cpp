#include "functions.h"
#include <QApplication>
#include <QtCore>
#include "functions.h"
#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

TEST_CASE("Encryption test")
{
	std::string aes_12345 = "6b09oYGQY/CtaY5L2ofTtg==";
	std::string aes_qwerty = "hzFZG4aM6x7RKIf0AB7xeg==";

	REQUIRE_EQ(aes_12345,aes128_encryption(QByteArray(("12345"))));
	REQUIRE_EQ(aes_qwerty,aes128_encryption(QByteArray(("qwerty"))));

	// Отрицательный результат, провераяющий, что при случайных данных выходная строка ненулевая
	srand(QTime::currentTime().msec());
	for (int i = 0; i < 50; ++i) {
		qint32 random_digit = rand();
		std::string encrypted = aes128_encryption(QByteArray::number(random_digit));
		CHECK_FALSE(encrypted.empty());
	}
}

TEST_CASE("Decryption test")
{
	REQUIRE_EQ("12345", aes128_decryption("6b09oYGQY/CtaY5L2ofTtg=="));
	REQUIRE_EQ("qwerty", aes128_decryption("hzFZG4aM6x7RKIf0AB7xeg=="));


	// Отрицательный результат, проверяющий, что не результат при невалидном вводе пустой
	std::string decrypted = aes128_decryption("some invalid text").toStdString();
	CHECK_FALSE(!decrypted.empty());
}


TEST_CASE("Hash test")
{
	QString sha_12345 = "5994471abb01112afcc18159f6cc74b4f511b99806da59b3caf5a9c173cacfc5";
	QString sha_qwerty = "65e84be33532fb784c48129675f9eff3a682b27168c0ea744b2cf58ee02337c5";
	REQUIRE_EQ(sha_12345,sha256_hash("12345"));
	REQUIRE_EQ(sha_qwerty,sha256_hash("qwerty"));

	// Отрицательный результат, провераяющий, что при случайных данных выходная строка ненулевая
	srand(QTime::currentTime().msec());
	for (int i = 0; i < 50; ++i) {
		qint32 random_digit = rand();
		QString encrypted = sha256_hash(QString::number(random_digit));
		CHECK_FALSE(encrypted.isEmpty());
	}

}

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	doctest::Context context;
	context.applyCommandLine(argc, argv);

	int res = context.run();

	return res;
}

