#ifndef INFERENCETHREAD_H
#define INFERENCETHREAD_H

#include <QObject>

class InferenceThread : public QObject
{
    Q_OBJECT
public:
    explicit InferenceThread(QObject *parent = nullptr);

signals:

};

#endif // INFERENCETHREAD_H
