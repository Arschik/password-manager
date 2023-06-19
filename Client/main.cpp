#include "greeting.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	Greeting authorization;
	authorization.show();

	return app.exec();
}
