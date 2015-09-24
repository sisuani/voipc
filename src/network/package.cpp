/*
 *
 *
 * Author: Samuel Isuani <sisuani@gmail.com>
 *
 *
 */

#include "package.h"

#include <QJson/Serializer>
#include <QJson/Parser>

Package::Package()
{
    m_valid = false;
}

void Package::serialize(const QVariantMap &data)
{
    m_data = data;

    QJson::Serializer serializer;
    m_json = serializer.serialize(m_data, &m_valid);
}

void Package::parse(const QByteArray &json)
{
    m_json = json;

    QJson::Parser parser;
    m_data = parser.parse(m_json, &m_valid).toMap();
}

bool Package::isValid()
{
    return m_valid;
}

const QByteArray &Package::json()
{
    return m_json;
}

const QVariantMap &Package::data()
{
    return m_data;
}
