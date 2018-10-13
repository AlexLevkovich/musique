#ifndef MPVSEEKSLIDER_H
#define MPVSEEKSLIDER_H

#include <QBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QPointer>
#include <QWidget>
#include "libmpv_def.h"
#include "mpvmediaobject.h"

namespace Libmpv {

class SeekSlider: public QWidget {
    Q_OBJECT
public:
    SeekSlider(QWidget * parent = 0);
    SeekSlider(MediaObject * mo,QWidget * parent = 0);

    bool hasTracking() const;
    MediaObject * mediaObject() const;
    Qt::Orientation orientation() const;
    int pageStep() const;
    void setPageStep(int milliseconds);
    void setSingleStep(int milliseconds);
    void setTracking(bool tracking);
    int	singleStep() const;

private slots:
    void on_seek(int value);
    void on_stateChanged(Libmpv::State state);
    void on_length(qint64 len);
    void on_tick(qint64 tick);
    void on_seekableChanged(bool flag);
    void on_currentSourceChanged();

public slots:
    void setMediaObject(MediaObject *media);
    void setOrientation(Qt::Orientation);

private:
    void init();

    QBoxLayout layout;
    QSlider slider;
    QPointer<MediaObject> media;
    bool ticking;
};

}

#endif // MPVSEEKSLIDER_H
