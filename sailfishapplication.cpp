
#include <QApplication>
#include <QDir>
#include <QGraphicsObject>

#include <QDeclarativeComponent>
#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#include <QDeclarativeView>
#include <QDBusConnection>
#include <QTranslator>
#include <QLocale>

#include <MDeclarativeCache>

#include <signonuiservice.h>

#include "sailfishapplication.h"
#include "config.h"
#include "models/documentlistmodel.h"
#include "models/trackerdocumentprovider.h"
#include "models/documentproviderplugin.h"
#include "models/documentproviderlistmodel.h"

QSharedPointer<QApplication> Sailfish::createApplication(int &argc, char **argv)
{
    auto app = QSharedPointer<QApplication>(MDeclarativeCache::qApplication(argc, argv));

    QTranslator* engineeringEnglish = new QTranslator( app.data() );
    if( !engineeringEnglish->load("sailfish-office_eng_en", TRANSLATION_INSTALL_DIR) )
        qWarning( "Could not load engineering english translation file!");
    QCoreApplication::installTranslator( engineeringEnglish );

    QTranslator* translator = new QTranslator( app.data() );
    if( !translator->load( QLocale::system(), "sailfish-office", "-", TRANSLATION_INSTALL_DIR) )
        qWarning( ("Could not load translations for " + QLocale::system().name()).toAscii() );
    QCoreApplication::installTranslator( translator );

    return app;
}
    
QSharedPointer<QDeclarativeView> Sailfish::createView(const QString &file)
{
    qmlRegisterType< DocumentListModel >( "Sailfish.Office.Files", 1, 0, "DocumentListModel" );
    qmlRegisterType< DocumentProviderListModel >( "Sailfish.Office.Files", 1, 0, "DocumentProviderListModel" );
    qmlRegisterType< TrackerDocumentProvider >( "Sailfish.Office.Files", 1, 0, "TrackerDocumentProvider" );
    qmlRegisterInterface< DocumentProviderPlugin >( "DocumentProviderPlugin" );

    QSharedPointer<QDeclarativeView> view(MDeclarativeCache::qDeclarativeView());
    view->engine()->addImportPath(CALLIGRA_QML_PLUGIN_DIR);
    view->setSource(QUrl::fromLocalFile(QML_INSTALL_DIR + file));

    // We want to have SignonUI in process, if user wants to create account from Documents
    SignonUiService* ssoui = new SignonUiService( view.data(), true ); // in process
    ssoui->setInProcessServiceName( DBUS_SERVICE );
    ssoui->setInProcessObjectPath( SIGNON_DBUS_OBJECT );

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    bool registeredService = sessionBus.registerService( DBUS_SERVICE );
    bool registeredObject = sessionBus.registerObject( SIGNON_DBUS_OBJECT, ssoui, QDBusConnection::ExportAllContents );

    if( !registeredService || !registeredObject )
        qWarning( "Warning: Unable to register signon dbus object for Sailfish Office. Is another instance running?" );

    view->rootContext()->setContextProperty( "jolla_signon_ui_service", ssoui );

    return view;
}

void Sailfish::showView(const QSharedPointer<QDeclarativeView> &view) 
{
    view->setResizeMode(QDeclarativeView::SizeRootObjectToView);
    view->setAttribute(Qt::WA_OpaquePaintEvent);
    view->setAttribute(Qt::WA_NoSystemBackground);
    view->viewport()->setAttribute(Qt::WA_OpaquePaintEvent);
    view->viewport()->setAttribute(Qt::WA_NoSystemBackground);
    view->showFullScreen();
}

