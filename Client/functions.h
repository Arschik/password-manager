#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <QtCore>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
std::string aes128_encryption(const QByteArray& passwords);
QByteArray aes128_decryption(const std::string& encryptedString);
QString sha256_hash (const QString& password);
#endif // FUNCTIONS_H
