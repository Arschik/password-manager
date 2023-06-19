#include "functions.h"

// Функция для зашифрования строки с помощью AES-128 и ключа SuperSecretKey!
std::string aes128_encryption(const QByteArray& passwords) {
	// Создаем контекст для шифрования
	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

	// Инициализируем контекст с алгоритмом AES-128-CBC и ключом SuperSecretKey!
	const unsigned char* key = (const unsigned char*)"SuperSecretKey!";
	const unsigned char* iv = (const unsigned char*)"0123456789abcdef"; // Вектор инициализации
	EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv);

	// Вычисляем размер буфера для зашифрованных данных
	int outlen = 0;
	int ciphertext_len = 0;
	int plaintext_len = passwords.size();
	unsigned char* ciphertext = new unsigned char[plaintext_len + EVP_CIPHER_block_size(EVP_aes_128_cbc())];

	// Шифруем данные из массива байтов в буфер
	EVP_EncryptUpdate(ctx, ciphertext, &outlen, (const unsigned char*)passwords.data(), plaintext_len);

	ciphertext_len += outlen;

	// Добавляем дополнительные байты для завершения шифрования
	EVP_EncryptFinal_ex(ctx, ciphertext + outlen, &outlen);
	ciphertext_len += outlen;

	// Освобождаем контекст
	EVP_CIPHER_CTX_free(ctx);

	// Преобразуем буфер в строку с помощью base64
	QByteArray ba = QByteArray::fromRawData((const char*)ciphertext, ciphertext_len);
	std::string encryptedString = ba.toBase64().toStdString();

	// Освобождаем буфер
	delete[] ciphertext;

	// Возвращаем зашифрованную строку
	return encryptedString;
}

// Функция для расшифрования строки с помощью AES-128 и ключа SuperSecretKey!
QByteArray aes128_decryption(const std::string& encryptedString) {
	// Создаем контекст для расшифровки
	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

	// Инициализируем контекст с алгоритмом AES-128-CBC и ключом SuperSecretKey!
	const unsigned char* key = (const unsigned char*)"SuperSecretKey!";
	const unsigned char* iv = (const unsigned char*)"0123456789abcdef"; // Вектор инициализации
	EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv);

	// Преобразуем строку в буфер с помощью base64
	QByteArray ba = QByteArray::fromStdString(encryptedString);
	ba = QByteArray::fromBase64(ba);
	int ciphertext_len = ba.size();
	unsigned char* ciphertext = (unsigned char*)ba.data();

	// Вычисляем размер буфера для расшифрованных данных
	int outlen = 0;
	int plaintext_len = 0;
	unsigned char* plaintext = new unsigned char[ciphertext_len];

	// Расшифровываем данные из буфера в массив байтов
	EVP_DecryptUpdate(ctx, plaintext, &outlen, ciphertext, ciphertext_len);
	plaintext_len += outlen;

	// Добавляем дополнительные байты для завершения расшифровки
	EVP_DecryptFinal_ex(ctx, plaintext + outlen, &outlen);
	plaintext_len += outlen;

	// Освобождаем контекст
	EVP_CIPHER_CTX_free(ctx);

	// Преобразуем буфер в массив байтов
	QByteArray passwords = QByteArray((const char*)plaintext, plaintext_len);

	// Освобождаем буфер
	delete[] plaintext;

	// Возвращаем массив байтов
	return passwords;
}

QString sha256_hash(const QString& password)
{
	QByteArray data = password.toUtf8();

	unsigned char hash[SHA256_DIGEST_LENGTH];
	SHA256((const unsigned char*)data.constData(), data.length(), hash);

	QByteArray result((const char*)hash, SHA256_DIGEST_LENGTH);
	return QString(result.toHex());
}

