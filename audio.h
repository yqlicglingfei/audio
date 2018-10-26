#pragma once

#include <QtWidgets/QWidget>
#include <QMediaPlayer>
#include <QMediaPlayList>
#include <QMap>
#include <QStandardItemModel>
#include <QMenu>
#include "ui_audio.h"

class audio : public QWidget
{
	Q_OBJECT

public:
	audio(QWidget *parent = Q_NULLPTR);
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

private:
	Ui::audioClass ui;
	QMediaPlayer *m_pMusicPlayer;
	QStandardItemModel* m_pModel;
	QMap<QString, QMediaPlaylist *> m_playLists;
	QMediaPlaylist *m_pCurrentPlayList;
};
