#include "CanvasClient.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QRegularExpression>
#include <QDebug>
#include <QUrlQuery>

CanvasClient::CanvasClient(const QUrl &apiBase, const QString &token)
    : m_apiBase(apiBase), m_token(token)
{
    connect(&m_nam, &QNetworkAccessManager::finished, this, &CanvasClient::onNetworkFinished);
}

void CanvasClient::fetchCourses()
{
    m_courses.clear();
    m_pagesFetched = 0;
    // Build the initial URL
    QUrl url = m_apiBase;
    if (!url.path().endsWith("/")) url.setPath(url.path() + "/");
    url.setPath(url.path() + "/api/v1/users/self/enrollments");
    m_nextUrl = url;

    // kick off
    QNetworkRequest req(m_nextUrl);
    req.setRawHeader("Authorization", QString("Bearer %1").arg(m_token).toUtf8());
    req.setHeader(QNetworkRequest::UserAgentHeader, "QtCanvasClient/1.0");
    m_nam.get(req);
}

void CanvasClient::fetchAssignmentsFromCourse(Course course)
{
    QUrl url = m_apiBase;
    if (!url.path().endsWith("/")) url.setPath(url.path() + "/");
    url.setPath(url.path() + QString("api/v1/courses/%1/assignments").arg(course.id));

    QDate today = QDate::currentDate();
    int daysToMonday = today.dayOfWeek() - 1;
    QDate monday = today.addDays(-daysToMonday);
    QDate sunday = monday.addDays(6);

    QUrlQuery q;
    q.addQueryItem("include[]", "submission");
    q.addQueryItem("per_page", "100");
    q.addQueryItem("start_date", monday.toString(Qt::ISODate));
    q.addQueryItem("end_date", sunday.toString(Qt::ISODate));
    url.setQuery(q);

    QNetworkRequest req(url);
    QByteArray authHeader = "Bearer " + m_token.trimmed().toUtf8();
    req.setRawHeader("Authorization", authHeader);
    req.setRawHeader("User-Agent", "QtCanvasClient/1.0");

    m_assignmentsAccumulator[course.id].clear();
    m_nam.get(req);

    qDebug() << "Fetching assignments for course" << course.id << "from" << monday << "to" << sunday;
}

void CanvasClient::onNetworkFinished(QNetworkReply *reply)
{
    // ... existing error handling ...

    QByteArray body = reply->readAll();
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(body, &err);

    if (err.error != QJsonParseError::NoError) {
        qDebug() << "JSON parse error:" << err.errorString();
    } else {
        // qDebug().noquote() << doc.toJson(QJsonDocument::Indented);
    }

    // figure out which endpoint this reply is for
    QUrl reqUrl = reply->request().url();
    QString path = reqUrl.path();

    // If this response is for assignments (contains "/assignments")
    if (path.contains("/assignments")) {
        // extract course id from path: /api/v1/courses/<id>/assignments
        QRegularExpression re(R"(/api/v1/courses/(\d+)/assignments)");
        QRegularExpressionMatch m = re.match(path);
        QString courseId;
        if (m.hasMatch()) courseId = m.captured(1);

        // parse and append assignments to accumulator for this course
        parseAssignmentsJson(body, courseId);

        // check pagination Link header
        QString linkHeader = QString::fromUtf8(reply->rawHeader("Link"));
        QUrl next = nextUrlFromLinkHeader(linkHeader);
        reply->deleteLater();

        if (!next.isEmpty()) {
            // continue paging for this course
            QNetworkRequest req(next);
            req.setRawHeader("Authorization", QString("Bearer %1").arg(m_token).toUtf8());
            req.setHeader(QNetworkRequest::UserAgentHeader, "QtCanvasClient/1.0");
            m_nam.get(req);
        } else {
            // finished paging for this course: emit assignmentsReady
            QList<Assignment> result = m_assignmentsAccumulator.value(courseId);
            addAssignmentsToCourse(courseId, result);
            for (const Course &course : m_courses) {
                qDebug() << "Course ID:" << course.id;
                qDebug() << "Assignments:";

                for (const Assignment &a : course.m_assignments) {
                    qDebug() << "   -" << a.name << "Due:" << a.dueAt << a.submitted;
                }
            }
            emit assignmentsReady(m_courses);
            // optionally clear accumulator if you won't need it later:
            // m_assignmentsAccumulator.remove(courseId);
        }

        return; // done

    }

    // else assume enrollments/courses response (previous behavior)
    parseTodoJson(body, m_courses);
    m_pagesFetched++;
    emit pageFetched(m_pagesFetched);

    // check Link header for enrollment pagination
    QString linkHeader = QString::fromUtf8(reply->rawHeader("Link"));
    QUrl next = nextUrlFromLinkHeader(linkHeader);
    reply->deleteLater();

    if (!next.isEmpty()) {
        QNetworkRequest req(next);
        req.setRawHeader("Authorization", QString("Bearer %1").arg(m_token).toUtf8());
        req.setHeader(QNetworkRequest::UserAgentHeader, "QtCanvasClient/1.0");
        m_nam.get(req);
    } else {
        // done fetching enrollments
        emit coursesReady(m_courses);
    }
}

void CanvasClient::addAssignmentsToCourse(const QString &courseId, const QList<Assignment> &assignments){
    for (Course &course : m_courses){
        if (course.id == courseId){
            course.m_assignments = assignments;
            return;
        }
    }
}


void CanvasClient::parseTodoJson(const QByteArray &data, QList<Course> &outList)
{

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        emit errorOccurred(QString("JSON parse error: %1").arg(err.errorString()));
        return;
    }

    // Canvas returns an array of items (or sometimes object - check your instance)
    if (doc.isArray()) {
        QJsonArray arr = doc.array();
        for (const QJsonValue &val : arr) {
            if (!val.isObject()) continue;
            QJsonObject obj = val.toObject();
            Course ti;
            // fields vary by Canvas version; safely check
            if (obj.contains("course_id")) ti.id = QString::number(obj.value("course_id").toVariant().toLongLong());

            outList.append(ti);
        }
    } else if (doc.isObject()) {
        // in some Canvas installations the response might be an object containing items
        QJsonObject root = doc.object();
        if (root.contains("items") && root["items"].isArray()) {
            QJsonArray arr = root["items"].toArray();
            for (const QJsonValue &val : arr) {
                if (!val.isObject()) continue;
                QJsonObject obj = val.toObject();
                Course ti;
                ti.id = obj.value("course_id").toString();

                outList.append(ti);
            }
        }
    }
}

void CanvasClient::parseAssignmentsJson(const QByteArray &data, const QString &courseId)
{

    QDate today = QDate::currentDate();
    QDate monday = today.addDays(1 - today.dayOfWeek()); // Monday
    QDate sunday = monday.addDays(6);

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        emit errorOccurred(QString("JSON parse error: %1").arg(err.errorString()));
        return;
    }

    if (!doc.isArray()) {
        qWarning() << "Unexpected assignments JSON (not an array) for course" << courseId;
        return;
    }

    QJsonArray arr = doc.array();
    for (const QJsonValue &val : arr) {
        if (!val.isObject()) continue;
        QJsonObject obj = val.toObject();
        Assignment a;
        a.name = obj.value("name").toString();
        a.dueAt = obj.value("due_at").toString(); // make sure Assignment struct has dueAt

        // skip assignments without due date
        if (a.dueAt.isEmpty())
            continue;

        // convert due_at to QDate
        QDate dueDate = QDate::fromString(a.dueAt.left(10), Qt::ISODate);
        if (!dueDate.isValid())
            continue;

        // keep only assignments due this week
        if (dueDate >= monday && dueDate <= sunday) {

            // submission object (if requested)
            if (obj.contains("submission") && obj.value("submission").isObject()) {
                QJsonObject sub = obj.value("submission").toObject();
                a.submitted = false;

                // 1) check submitted_at (most reliable)
                QString submittedAt = sub.value("submitted_at").toString().trimmed();
                if (!submittedAt.isEmpty()) {
                    a.submitted = true;
                }

                // 2) workflow_state fallback
                if (!a.submitted) {
                    QString state = sub.value("workflow_state").toString().toLower();
                    if (state == "submitted" || state == "graded" || state == "pending_review") {
                        a.submitted = true;
                    } else if (state == "unsubmitted" || state.isEmpty()) {
                        a.submitted = false;
                    } else {
                        // unknown state: keep false, or log for debugging
                        qDebug() << "Unknown workflow_state:" << state << "for assignment" << a.name;
                    }
                }
            } else {
                a.submitted = false;
            }

            m_assignmentsAccumulator[courseId].append(a);
        }
    }
}


QUrl CanvasClient::nextUrlFromLinkHeader(const QString &linkHeader)
{
    // Link header format: <https://.../api/v1/...page=2>; rel="next", <...>; rel="first"
    if (linkHeader.isEmpty()) return QUrl();

    // regex to capture <URL>; rel="next"
    QRegularExpression re(R"(<([^>]+)>\s*;\s*rel\s*=\s*"?next"?)");
    QRegularExpressionMatch m = re.match(linkHeader);
    if (m.hasMatch()) {
        QString urlStr = m.captured(1);
        return QUrl(urlStr);
    }
    return QUrl();
}
