#include "audio.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	audio w;
	w.show();
	return a.exec();
}
