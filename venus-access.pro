QT = core dbus

unix {
    bindir = $$(bindir)
    DESTDIR = $$(DESTDIR)
    isEmpty(bindir) {
        bindir = /opt/victronenergy/venus-access
    }
    target.path = $${DESTDIR}$${bindir}
}
!isEmpty(target.path): INSTALLS += target

equals(QT_MAJOR_VERSION, 6): QMAKE_CXXFLAGS += -std=c++17

HEADERS = \
    src/application.hpp \

SOURCES = \
    src/application.cpp \
    src/main.cpp \

QMAKE_CXXFLAGS *= -ffunction-sections
QMAKE_LFLAGS *= -Wl,--gc-sections

!lessThan(QT_VERSION, 5) {
    QMAKE_CXXFLAGS += "-Wsuggest-override"
    CONFIG(debug, debug|release) {
        QMAKE_CXXFLAGS += "-Werror=suggest-override"
    }
}

include("ext/veutil/veutil.pri")
