#include "audio.h"
#include <QFileDialog>
#include <QAction>
#include <qinputdialog.h>
#include <qlineedit.h>

//获取当前exe所在路径
const QString runPath = QCoreApplication::applicationDirPath();

audio::audio(QWidget *parent)
	: QWidget(parent)
	, m_pMusicPlayer(nullptr)
	//, playList(nullptr)
{
	ui.setupUi(this);

	initWidget();
}

void audio::onPlayMedia()
{
	if (QMediaPlayer::PausedState == m_pMusicPlayer->state() || QMediaPlayer::StoppedState == m_pMusicPlayer->state())
	{
		m_pMusicPlayer->play();
	}
	else
	{
		m_pMusicPlayer->pause();
	}
}

void audio::onReplayMedia()
{
	m_pMusicPlayer->stop();
	m_pMusicPlayer->play();
}

void audio::onUpdateSlider(qint64 position)
{
	ui.horizontalSlider_progress->setSliderPosition(position);
}

void audio::onSetSliderLen(qint64 range)
{
	ui.horizontalSlider_progress->setRange(0, range);
}

void audio::onSetProgress(int cur)
{
	m_pMusicPlayer->setPosition(cur);
}

void audio::onAddFileToPlayList()
{
	bool isOK;
	QString text = QInputDialog::getText(NULL, QString::fromLocal8Bit("新建播放列表"), QString::fromLocal8Bit("播放列表名称"),	QLineEdit::Normal, "", &isOK);
	if (!isOK || text.isEmpty())
	{
		return;
	}
	else
	{
		m_playLists.insert(text, nullptr);
	}

	QStringList allFiles = QFileDialog::getOpenFileNames(this, "Select one or more to open", nullptr, "mp3(*.mp3)");

	if (allFiles.empty())
	{
		return;
	}

	QSharedPointer<QMediaPlaylist> playList = QSharedPointer<QMediaPlaylist>(new QMediaPlaylist);
	QList<QMediaContent> itemList;
	for (auto file : allFiles)
	{
		file.replace(QString("/"), QString("\\"));
		itemList.append(QMediaContent(QUrl::fromLocalFile(file)));
	}

	// add playlist
	playList->addMedia(itemList);
	m_playLists[text] = playList;

	QStandardItem* list = new QStandardItem(text);
	m_pModel->appendRow(list);

	for (auto file : allFiles)
	{
		QString fileName = file.split("/").back();
		QStandardItem* item = new QStandardItem(fileName);
		list->appendRow(item);
        //m_pModel->appendRow(item);
	}

	m_pMusicPlayer->setPlaylist(playList.data());
	m_pCurrentPlayList = playList;
	ui.treeView->expandAll();
}

void audio::onPlayItem(QModelIndex itemIndex)
{
    QModelIndex parentItem = itemIndex.parent();
    if (parentItem.isValid())
    {
        //auto item = m_pModel->findItems(parentItem.data().toString()).at(0);
        auto it = m_playLists.find(parentItem.data().toString());
        auto item = it.value();
        qDebug() << it.key();

        m_pCurrentPlayList = item;
        m_pMusicPlayer->setPlaylist(m_pCurrentPlayList.data());

        int index = itemIndex.row();
        m_pMusicPlayer->setMedia(m_pCurrentPlayList->media(index));
        m_pMusicPlayer->play();
    }
}

void audio::onPlayNext()
{
	if (NULL == m_pCurrentPlayList)
	{
		return;
	}
	QModelIndex modelIndex = ui.treeView->currentIndex();

	//计算ui和数据中的当前index
	int index = m_pCurrentPlayList->nextIndex();
	index = index < 0 ? 0 : index;

	m_pCurrentPlayList->setCurrentIndex(index); 

	ui.treeView->setCurrentIndex(modelIndex.sibling((modelIndex.row() + 1) % m_pCurrentPlayList->mediaCount(), 0));

	m_pMusicPlayer->setMedia(m_pCurrentPlayList->media(index));
	m_pMusicPlayer->play();
}

void audio::onPlayPrevious()
{
	QModelIndex modelIndex = ui.treeView->currentIndex();

	//计算ui和数据中的当前index
	int index = m_pCurrentPlayList->currentIndex();
	index = (index == 0) ? m_pCurrentPlayList->mediaCount() - 1 : index - 1;

	m_pCurrentPlayList->setCurrentIndex(index);

	ui.treeView->setCurrentIndex(modelIndex.sibling(index, 0));

	m_pMusicPlayer->setMedia(m_pCurrentPlayList->media(index));
	m_pMusicPlayer->play();
}

void audio::onAutoPlayNext(QMediaPlayer::MediaStatus status)
{
	if (status == QMediaPlayer::EndOfMedia)
	{
		onPlayNext();
	}
}

audio::~audio()
{

}

void audio::initWidget()
{
	// 初始化播放器
	m_pMusicPlayer = QSharedPointer<QMediaPlayer>(new QMediaPlayer());
	m_pMusicPlayer->setVolume(1);

	m_pModel = QSharedPointer <QStandardItemModel>(new QStandardItemModel(ui.treeView));
	ui.treeView->setHeaderHidden(true);
	ui.treeView->setEditTriggers(QTreeView::NoEditTriggers);

	ui.treeView->setModel(m_pModel.data());
	ui.treeView->setContextMenuPolicy(Qt::CustomContextMenu);

	// 设置音量范围
	ui.horizontalSlider_sound->setRange(0, 10);

	connect(ui.treeView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnShowContextMenu(const QPoint&)));

	// sound change
	connect(ui.horizontalSlider_sound, SIGNAL(valueChanged(int)), m_pMusicPlayer.data(), SLOT(setVolume(int)));
	connect(m_pMusicPlayer.data(), SIGNAL(volumeChanged(int)), ui.horizontalSlider_sound, SLOT(setValue(int)));

	// player progress change
	connect(m_pMusicPlayer.data(), SIGNAL(positionChanged(qint64)), this, SLOT(onUpdateSlider(qint64)));
	connect(m_pMusicPlayer.data(), SIGNAL(durationChanged(qint64)), this, SLOT(onSetSliderLen(qint64)));
	connect(ui.horizontalSlider_progress, SIGNAL(sliderMoved(int)), this, SLOT(onSetProgress(int)));

	// player start, stop and pause
	connect(ui.pushButton_startorstop, SIGNAL(clicked()), this, SLOT(onPlayMedia()));
	connect(ui.pushButton_restart, SIGNAL(clicked()), this, SLOT(onReplayMedia()));

	// load player list
	connect(ui.toolButton_fileList, SIGNAL(clicked()), this, SLOT(onAddFileToPlayList()));
	connect(ui.treeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onPlayItem(QModelIndex)));
	//connect(ui.listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(onPlayItem(QListWidgetItem *)));

	// play next
	connect(ui.pushButton_next, SIGNAL(clicked()), this, SLOT(onPlayNext()));

	// play previous
	connect(ui.pushButton_previous, SIGNAL(clicked()), this, SLOT(onPlayPrevious()));

	// auto play next
	connect(m_pMusicPlayer.data(), SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(onAutoPlayNext(QMediaPlayer::MediaStatus)));
}

void audio::OnShowContextMenu(const QPoint& pos)
{
	QModelIndex modelIndex = ui.treeView->currentIndex();
	
	if (!modelIndex.parent().isValid())
	{
		QMenu projectMenu;
		QAction* addAction = nullptr;
		addAction = new QAction(QStringLiteral("添加歌曲"), this);
		connect(addAction, SIGNAL(triggered()), this, SLOT(slotAddItem()));
		projectMenu.addAction(addAction);

		QAction* deleteAction = nullptr;
		deleteAction = new QAction(QStringLiteral("删除列表"), this);
		connect(deleteAction, SIGNAL(triggered()), this, SLOT(slotDelList()));
		projectMenu.addAction(deleteAction);

		//! 显示该菜单，进入消息循环
		projectMenu.exec(QCursor::pos());
	}
	else
	{
		QMenu itemMenu;
		QAction* deleteAction = nullptr;
		deleteAction = new QAction(QStringLiteral("删除歌曲"), this);
		connect(deleteAction, SIGNAL(triggered()), this, SLOT(slotDelItem()));
		itemMenu.addAction(deleteAction);
		//! 显示该菜单，进入消息循环
		itemMenu.exec(QCursor::pos());
	}
}

void audio::slotAddItem()
{
    QString currentPlayListName;
    QModelIndex modelIndex = ui.treeView->currentIndex();
    //auto item = m_pModel->findItems(modelIndex.data().toString(), Qt::MatchRecursive);
    //item.at(0)->row();

    auto list = std::find_if(m_playLists.begin(), m_playLists.end(), [&](QSharedPointer<QMediaPlaylist> it) { return (it == m_pCurrentPlayList); });
    if (list != m_playLists.end())
    {
        currentPlayListName = list.key();
    }

    QStringList allFiles = QFileDialog::getOpenFileNames(this, "Select one or more to open", nullptr, "mp3(*.mp3)");

	if (allFiles.empty())
	{
		return;
	}

	QList<QMediaContent> itemList;
	for (auto file : allFiles)
	{
		file.replace(QString("/"), QString("\\"));
		itemList.append(QMediaContent(QUrl::fromLocalFile(file)));
	}

    if (modelIndex.data().toString() == currentPlayListName)
    {
        m_pCurrentPlayList->addMedia(itemList);
        //m_pMusicPlayer->setPlaylist(m_pCurrentPlayList.data());
    }
    else
    {
        m_playLists.find(modelIndex.data().toString()).value().data()->addMedia(itemList);
    }

	//QString text;
	//for (auto it = m_playLists.begin(); it != m_playLists.end(); ++it)
	//{
	//	if (it.value() == m_pCurrentPlayList)
	//	{
	//		text = it.key();
	//		break;
	//	}
	//}
	QList<QStandardItem*> listItems = m_pModel->findItems(modelIndex.data().toString());
	QStandardItem *headItem = listItems.first();
	
	for (auto file : allFiles)
	{
		QString fileName = file.split("/").back();
		QStandardItem* item = new QStandardItem(fileName);
		headItem->appendRow(item);
	}

	//m_pMusicPlayer->setPlaylist(m_pCurrentPlayList.data());
    //m_pMusicPlayer->play();
}

void audio::slotDelList()
{
    QModelIndex modelIndex = ui.treeView->currentIndex();
    auto item = m_pModel->findItems(modelIndex.data().toString());
    qDebug() << modelIndex.data();
    auto it = m_playLists.find(modelIndex.data().toString());
    if (it != m_playLists.end())
    {
        m_playLists.erase(it);
    }
    m_pModel->removeRow(item.at(0)->row());
    m_pMusicPlayer->stop();
}

void audio::slotDelItem()
{
    QModelIndex modelIndex = ui.treeView->currentIndex();
    //找到当前节点的父节点
    QModelIndex parentModel = modelIndex.parent();

    auto item = m_pModel->findItems(modelIndex.data().toString(), Qt::MatchRecursive);
    qDebug() << modelIndex.data() << m_pModel.data()->itemFromIndex(modelIndex);

    if (!item.empty())
    {
        if (item.at(0)->parent()->index() == parentModel)
        {
            if (m_pCurrentPlayList->currentIndex() == modelIndex.row())
            {
                m_pMusicPlayer->stop();
            }
        }
        m_pCurrentPlayList->removeMedia(modelIndex.row());
    }

    if (parentModel.isValid())
    {
        //QModelIndex index = parentModel.model()->findChild<QModelIndex>(modelIndex.data().toString());
        //if (index.isValid())
        //{
        //    if (m_pCurrentPlayList->currentIndex() == modelIndex.row())
        //    {
        //        m_pMusicPlayer->stop();
        //    }
        //    m_pCurrentPlayList->removeMedia(modelIndex.row());
        //}

        //m_pMusicPlayer->media();
        m_pModel->removeRow(modelIndex.row(), parentModel);
    }
    //m_pMusicPlayer->stop();
}