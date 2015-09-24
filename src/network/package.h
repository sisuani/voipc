/*
 *
 *
 * Author: Samuel Isuani <sisuani@gmail.com>
 *
 *
 */

#ifndef PACKAGE_H
#define PACKAGE_H

#include <QFlags>
#include <QString>
#include <QVariantMap>

class Package
{

public:
    enum Type {
    };
    Q_DECLARE_FLAGS(Types, Type);

    Package();

    void serialize(const QVariantMap &data);
    void parse(const QByteArray &json);

    bool isValid();

    const QByteArray &json();
    const QVariantMap &data();

private:
    bool m_valid;
    QByteArray m_json;
    QVariantMap m_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Package::Types)

#endif // PACKAGE_H
