#include "app/Session.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QStandardPaths>

#ifdef Q_OS_WIN
#include <windows.h>
#include <lmcons.h>
#else
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#endif

QString Session::reportPath() {
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/sessions.txt";
}

bool Session::begin() {
    if (!timeIn_.isNull())
        return false;

    timeIn_ = QDateTime::currentDateTime();
    login_ = systemLogin();
    return true;
}

QString Session::systemLogin() {
#ifdef Q_OS_WIN
    wchar_t name[UNLEN + 1]{};
    DWORD size = UNLEN + 1;
    if (GetUserNameW(name, &size))
        return QString::fromWCharArray(name);
#else
    if (const passwd* user = getpwuid(getuid()))
        return QString::fromUtf8(user->pw_name);
#endif
    return {};
}

bool Session::finish(QString& error) {
    if (!timeOut_.isNull())
        return true;

    if (login_.isEmpty())
        login_ = systemLogin();
    if (login_.isEmpty()) {
        error = "Не удалось определить имя пользователя ОС.";
        return false;
    }

    QString path = reportPath();
    if (!QDir().mkpath(QFileInfo(path).absolutePath())) {
        error = "Не удалось создать папку журнала.";
        return false;
    }

    QByteArray previous;
    QFile existing(path);
    if (existing.exists()) {
        if (!existing.open(QIODevice::ReadOnly)) {
            error = existing.errorString();
            return false;
        }
        previous = existing.readAll();
        if (existing.error() != QFile::NoError) {
            error = existing.errorString();
            return false;
        }
        existing.close();
    }

    QDateTime finishTime = QDateTime::currentDateTime();
    int width = qMax(24, int(login_.size()));
    QString border =
        "+" + QString(width + 2, '-') + "+---------------------+---------------------+\n";
    QString table = border + "| " + QString("login").leftJustified(width) + " | " +
                    QString("time_in").leftJustified(19) + " | " +
                    QString("time_out").leftJustified(19) + " |\n" + border;
    table += "| " + login_.leftJustified(width) + " | " + timeIn_.toString("dd.MM.yyyy HH:mm:ss") +
             " | " + finishTime.toString("dd.MM.yyyy HH:mm:ss") + " |\n" + border;

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        error = file.errorString();
        return false;
    }

    QByteArray contents = previous + table.toUtf8();
    if (file.write(contents) != contents.size() || !file.commit()) {
        error = file.errorString();
        return false;
    }

    timeOut_ = finishTime;
    return true;
}

QString Session::repr() const {
    return "Session(" + login_ + ";" + timeIn_.toString(Qt::ISODateWithMs) + ";" +
           timeOut_.toString(Qt::ISODateWithMs) + ")";
}

QString Session::toString() const {
    return login_ + ", вход: " + timeIn_.toString("dd.MM.yyyy HH:mm:ss");
}
