USE_BREW = $$(USE_BREW)

macx:!isEmpty(USE_BREW):equals(USE_BREW, true) {
    message("use qwt from brew")
    CONFIG += qwt release

    QMAKE_TARGET_BUNDLE_PREFIX = org.bavc
    QT_CONFIG -= no-pkg-config

    include ( $$system(brew --prefix qwt)/features/qwt.prf )

    CONFIG += link_pkgconfig
} else {

    QWT_ROOT = $$(QWT_ROOT)
    isEmpty(QWT_ROOT) {
        QWT_ROOT = $$absolute_path($$PWD/../../../../../qwt)
    }

    message("use external qwt: QWT_ROOT = " $$QWT_ROOT)

    include( $${QWT_ROOT}/qwtconfig.pri )
    !win32 {
        include( $${QWT_ROOT}/qwtbuild.pri )
    }
    include( $${QWT_ROOT}/qwtfunctions.pri )

    macx {
        macx:LIBS       += -F$${QWT_ROOT}/lib -framework qwt
    }

    win32-msvc* {
        DEFINES += QWT_DLL
    }

    !macx: {
        win32:CONFIG(release, debug|release): LIBS += -L$${QWT_ROOT}/lib -lqwt
        else:win32:CONFIG(debug, debug|release): LIBS += -L$${QWT_ROOT}/lib -lqwtd
        else: LIBS += -L$${QWT_ROOT}/lib -lqwt
    }

    INCLUDEPATH += $$QWT_ROOT/src
}
