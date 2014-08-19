QT += widgets

INCLUDEPATH += \
	$$PWD/../../interpreterBase/include \
	$$PWD/../../../utils/include \
	$$PWD/../../../../../ \
	$$PWD/../../../../../qrgui/ \
	$$PWD/../../../../../qrtext/include \

LIBS += -L$$PWD/../../../../../bin -linterpreterBase -lqrtext

HEADERS += \
	$$PWD/nullKitInterpreterPlugin.h \
	$$PWD/nullRobotModel.h \

SOURCES += \
	$$PWD/nullKitInterpreterPlugin.cpp \
	$$PWD/nullRobotModel.cpp \

TRANSLATIONS = $$PWD/../nullKitInterpreter_ru.ts

RESOURCES += $$PWD/../nullKitInterpreter.qrc
