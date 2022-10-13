#include "gloxiniaconfigurator.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QMetaType>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // register convertors
    QMetaType::registerConverter<GCNode, QString>(&GCNode::toString);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "GloxiniaConfigurator_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    GloxiniaConfigurator w;
    w.show();
    return a.exec();
}
