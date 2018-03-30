#ifndef LOCAL_RUNNER_CONFIG_H
#define LOCAL_RUNNER_CONFIG_H

#include <memory>
#include <QtCore/QSettings>
#include <QApplication>
#include <QtCore/QTextCodec>
#include <QFile>

class Config {
public:
    inline Config();
    inline int getInt(const QString& key, int defaultValue) const;
    inline uint64_t getBigUnsigned(const QString& key, uint64_t defaultValue) const;
    inline double getDouble(const QString& key, double defaultValue) const;
    inline QString getString(const QString& key, const QString& defaultValue) const;
    inline Config& setString(const QString& key, const QString& value);
private:
    template <class T, class FromString>
    T get(const QString& key, FromString transform, const T& defaultValue) const;
    inline QVariant getProperty(const QString& key) const;
    inline void setProperty(const QString& key, const QVariant& value) const;
private:
    std::unique_ptr<QSettings> settings;
};

Config::Config() {
    const auto args = QApplication::arguments();

    if (args.size() > 1) {
        settings = std::unique_ptr<QSettings>( new QSettings(args[1], QSettings::IniFormat) );
        settings->setIniCodec(QTextCodec::codecForName("UTF-8"));
        settings->sync();

        if (settings->status() != QSettings::Status::NoError) {
            settings.reset();
        }
    }
}

int Config::getInt(const QString& key, int defaultValue) const {
    return get(
        key,
        [] (const QVariant& value, bool* success) { return value.toInt(success); },
        defaultValue
    );
}

uint64_t Config::getBigUnsigned(const QString& key, uint64_t defaultValue) const {
    return get(
        key,
        [] (const QVariant& value, bool* success) { return value.toULongLong(success); },
        defaultValue
    );
}

double Config::getDouble(const QString& key, double defaultValue) const {
    return get(
        key,
        [] (const QVariant& value, bool* success) {return value.toDouble(success); },
        defaultValue
    );
}

QString Config::getString(const QString& key, const QString& defaultValue) const {
    return get(
        key,
        [] (const QVariant& value, bool* success) { return value.toString(); },
        defaultValue
    );
}

Config& Config::setString(const QString& key, const QString& value) {
    setProperty(key, value);
    return *this;
}

template<class T, class FromVariant>
T Config::get(const QString& key, FromVariant transform, const T& defaultValue) const {
    const auto variant = getProperty(key);
    bool success(true);
    T value = transform(variant, &success);

    if (!success) {
        value = defaultValue;
    }
    return value;
}

QVariant Config::getProperty(const QString& key) const {
    QVariant variant;
    if (settings) {
        variant = settings->value(key);
    }
    return variant;
}

void Config::setProperty(const QString& key, const QVariant& value) const {

    if (settings) {
        settings->setValue(key, value);
    }
}

#endif //LOCAL_RUNNER_CONFIG_H
