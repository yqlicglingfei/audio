#pragma once

#include <QtWidgets/QWidget>
#include <QMediaPlayer>
#include <QMediaPlayList>
#include <QMap>
#include <QStandardItemModel>
#include <QMenu>
#include <QSharedPointer>
#include "ui_audio.h"

class audio : public QWidget
{
	Q_OBJECT

public:
	audio(QWidget *parent = Q_NULLPTR);
    ~audio();
	void initWidget();

private slots:
	void onPlayMedia();
	void onReplayMedia();
	void onUpdateSlider(qint64);
	void onSetSliderLen(qint64);
	void onSetProgress(int);
	void onAddFileToPlayList();
	void onPlayItem(QModelIndex itemIndex);
	void onPlayNext();
	void onPlayPrevious();
	void onAutoPlayNext(QMediaPlayer::MediaStatus);
	void OnShowContextMenu(const QPoint&);
	void slotAddItem();
	void slotDelList();
	void slotDelItem();

private:
	Ui::audioClass ui;
    QSharedPointer<QMediaPlayer>m_pMusicPlayer;
    QSharedPointer <QStandardItemModel> m_pModel;
	QMap<QString, QSharedPointer<QMediaPlaylist>> m_playLists;
	QSharedPointer<QMediaPlaylist>m_pCurrentPlayList;
};
