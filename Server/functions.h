#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include <QCoreApplication>
#include <QHttpServer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QHttpMultiPart>
#include <windows.h>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

std::string aes128_encryption(const QByteArray& passwords);
QByteArray aes128_decryption(const std::string& encryptedString);
QString sha256_hash (const QString& password);
#endif // FUNCTIONS_H
