#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonArray>
#include <QUrl>

struct Assignment {
    QString name;
    QString dueAt;
    bool submitted;
};

struct Course {
    QString id;
    QList<Assignment> m_assignments;
};

Q_DECLARE_METATYPE(Course)

class CanvasClient : public QObject {
    Q_OBJECT
public:
    explicit CanvasClient(const QUrl &apiBase, const QString &token);

    // start fetching the todo list for the current user
    void fetchCourses();
    void fetchAssignmentsFromCourse(Course course);
    void addAssignmentsToCourse(const QString &courseId, const QList<Assignment> &assignments);

signals:
    // emitted when one page is fetched (useful for progress)
    void pageFetched(int pageCount);

    // emitted when all todo items are loaded
    void coursesReady(const QList<Course> &items);
    void assignmentsReady(const QList<Course> &items);

    void errorOccurred(const QString &errorString);

private slots:
    void onNetworkFinished(QNetworkReply *reply);

private:
    void parseTodoJson(const QByteArray &data, QList<Course> &outList);
    void parseAssignmentsJson(const QByteArray &data, const QString &courseId);

    QUrl nextUrlFromLinkHeader(const QString &linkHeader);

    QNetworkAccessManager m_nam;
    QUrl m_apiBase;       // e.g. https://canvas.example.edu
    QString m_token;
    QUrl m_nextUrl;       // next page to fetch, if any
    QList<Course> m_courses;
    int m_pagesFetched = 0;
    QMap<QString, QList<Assignment>> m_assignmentsAccumulator;
};
