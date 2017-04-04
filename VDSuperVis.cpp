#include <QApplication>

#include "UserGui.h"


int main(int argc, char* argv[]) {

	QApplication app(argc, argv);
	UserGui userGui;
	userGui.show();

    return app.exec();
}
