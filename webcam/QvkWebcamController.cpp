#include "QvkWebcamController.h"
#include "QvkWebcamWindow.h"
#include "QvkWebcamWatcher.h"
#include "QvkVideoSurface.h"

#include <QCameraInfo>
#include <QCameraViewfinder>

// http://doc.qt.io/qt-5/qgraphicsscene.html#addWidget

// Hint:
// /usr/local/Qt-5.5.0/bin/qmake GST_VERSION=1.0


// Für Zugriff auf die camera Bilder
// http://www.qtcentre.org/threads/57090-How-could-I-get-the-image-buffer-of-QCamera

QvkWebcamController::QvkWebcamController( Ui_screencast value )
{
  vkSettings.readAll();

  myUi = value;

  myUi.webcamCheckBox->setEnabled( false );

  myUi.webcamComboBox->setEnabled( false );

  myUi.rotateDial->setMinimum( 1 );
  myUi.rotateDial->setMaximum ( 360 );
  myUi.rotateDial->setValue( 1 );
  connect( myUi.rotateDial, SIGNAL( sliderPressed () ), this, SLOT( rotateDialclicked() ) );

  myUi.radioButtonTopMiddle->setChecked( vkSettings.getWebcamButtonTopMiddle() );

  myUi.radioButtonRightMiddle->setChecked( vkSettings.getWebcamButtonRightMiddle() );

  myUi.radioButtonBottomMiddle->setChecked( vkSettings.getWebcamButtonBottomMiddle() );

  myUi.radioButtonLeftMiddle->setChecked( vkSettings.getWebcamButtonLeftMiddle() );

  webcamWindow = new QvkWebcamWindow();

  mirrored = false;

  if ( myUi.webcamCheckBox->checkState() == Qt::Unchecked )
  {
    myUi.mirrorCheckBox->setEnabled( false );
    myUi.dialFrame->setEnabled( false );
    myUi.grayCheckBox->setEnabled( false );
    myUi.invertCheckBox->setEnabled( false );
  }
  else
  {
    myUi.mirrorCheckBox->setEnabled( true );
    myUi.dialFrame->setEnabled( true );
    myUi.grayCheckBox->setEnabled( true );
    myUi.invertCheckBox->setEnabled( true );
  }
  connect( myUi.mirrorCheckBox, SIGNAL( clicked( bool ) ), this, SLOT( setMirrorOnOff( bool ) ) );
  if ( vkSettings.getWebcamMirrored() == Qt::Checked )
  {
    myUi.mirrorCheckBox->setEnabled( true );
    myUi.mirrorCheckBox->click();
    myUi.mirrorCheckBox->setEnabled( false );
  }

  connect( myUi.webcamCheckBox, SIGNAL( stateChanged( int ) ), this, SLOT( webcamOnOff( int ) ) );
  connect( webcamWindow, SIGNAL( closeWebcamWindow() ), myUi.webcamCheckBox, SLOT( toggle() )  );

  videoSurface = new QvkVideoSurface( this );
  connect( videoSurface, SIGNAL( newPicture( QImage ) ), this, SLOT( setNewImage( QImage ) ) );

  QvkWebcamWatcher *webcamWatcher = new QvkWebcamWatcher();
  connect( webcamWatcher, SIGNAL( webcamDescription( QStringList, QStringList ) ), this, SLOT( addToComboBox( QStringList, QStringList ) ) );
  connect( webcamWatcher, SIGNAL( removedCamera( QString ) ), this, SLOT( ifCameraRemovedCloseWindow( QString ) ) );
}


QvkWebcamController::~QvkWebcamController()
{
}

void QvkWebcamController::setNewImage( QImage image )
{
  if ( mirrored == true )
    image = image.mirrored ( true, false );

  if ( myUi.radioButtonLeftMiddle->isChecked() == true )
     myUi.rotateDial->setValue( 90 );

  if ( myUi.radioButtonTopMiddle->isChecked() == true )
     myUi.rotateDial->setValue( 180 );

  if ( myUi.radioButtonRightMiddle->isChecked() == true )
     myUi.rotateDial->setValue( 270 );

  if ( myUi.radioButtonBottomMiddle->isChecked() == true )
      myUi.rotateDial->setValue( 360 );

  QTransform transform;
  transform.rotate( myUi.rotateDial->value() );
  QImage transformedImage = image.transformed( transform );

  if ( myUi.grayCheckBox->isChecked() == true )
  {
    transformedImage = transformedImage.convertToFormat( QImage::Format_Grayscale8 );
  }

  if ( myUi.invertCheckBox->isChecked() == true )
  {
   transformedImage.invertPixels( QImage::InvertRgb );
  }

  // Passt Bild beim resizen des Fensters an
  transformedImage = transformedImage.scaled( webcamWindow->webcamLabel->width(), webcamWindow->webcamLabel->height(), Qt::KeepAspectRatio, Qt::FastTransformation);
  webcamWindow->webcamLabel->setPixmap( QPixmap::fromImage( transformedImage, Qt::AutoColor) );
}

void QvkWebcamController::setMirrorOnOff( bool value )
{
  if ( value == true )
    mirrored = true;
  else
    mirrored = false;
}


void QvkWebcamController::rotateDialclicked()
{
  // Diese drei Befehle müssen sein damit der Radiobutton unchecked ist
  myUi.radioButtonTopMiddle->setCheckable( false );
  myUi.radioButtonTopMiddle->setChecked( false );
  myUi.radioButtonTopMiddle->setCheckable ( true );
  myUi.radioButtonTopMiddle->update();

  myUi.radioButtonRightMiddle->setCheckable ( false );
  myUi.radioButtonRightMiddle->setChecked( false );
  myUi.radioButtonRightMiddle->setCheckable ( true );
  myUi.radioButtonRightMiddle->update();

  myUi.radioButtonBottomMiddle->setCheckable ( false );
  myUi.radioButtonBottomMiddle->setChecked( false );
  myUi.radioButtonBottomMiddle->setCheckable ( true );
  myUi.radioButtonBottomMiddle->update();

  myUi.radioButtonLeftMiddle->setCheckable ( false );
  myUi.radioButtonLeftMiddle->setChecked( false );
  myUi.radioButtonLeftMiddle->setCheckable ( true );
  myUi.radioButtonLeftMiddle->update();
}

/*
 * If camera removed and have a display window, the diplay window will removed
 */
void QvkWebcamController::ifCameraRemovedCloseWindow( QString value )
{
   qDebug() << "[vokoscreen] camera removed" << value;
   qDebug(" ");
   if ( getActiveCamera() == value )
   {
     myUi.webcamCheckBox->setChecked( false );
   }
}

/*
 * set a flag which camera is active
 */
void QvkWebcamController::setActiveCamera( QString value )
{
   aktivCamera = value;
}

/*
 * get a flag which camera is active
 */
QString QvkWebcamController::getActiveCamera()
{
    return aktivCamera;
}


void QvkWebcamController::webcamOnOff( int value )
{
  if ( value == Qt::Checked)
  {
    myUi.webcamComboBox->setEnabled( false );
    myUi.mirrorCheckBox->setEnabled( true );
    myUi.dialFrame->setEnabled( true );
    myUi.grayCheckBox->setEnabled( true );
    myUi.invertCheckBox->setEnabled( true );

    // save the active camera to a flag
    setActiveCamera( myUi.webcamComboBox->currentData().toString() );

    QByteArray device = myUi.webcamComboBox->currentData().toByteArray();
    displayWebcam( device );
  }

  if ( value == Qt::Unchecked )
  {
    camera->unload();
    camera->stop();
    webcamWindow->hide();
    delete camera;
    myUi.webcamComboBox->setEnabled( true );
    myUi.mirrorCheckBox->setEnabled( false );
    myUi.dialFrame->setEnabled( false );
    myUi.grayCheckBox->setEnabled( false );
    myUi.invertCheckBox->setEnabled( false );
  }
}


void QvkWebcamController::addToComboBox( QStringList description, QStringList device )
{
    myUi.webcamComboBox->clear();

    if ( device.count()  > 0  )
    {
      myUi.webcamCheckBox->setEnabled( true );
      myUi.webcamComboBox->setEnabled( true );
      for ( int i = 0; i < description.count(); i++ )
      {
        QString descript =  description[i];
        if ( !descript.contains( "@device:pnp" ) ) // Geräte mit dieser Beschreibung aussortieren
          myUi.webcamComboBox->addItem( description[i], device[i] );
      }
    }
    else
    {
      myUi.webcamCheckBox->setEnabled( false );
      myUi.webcamComboBox->setEnabled( false );
    }

}

// http://doc.qt.io/qt-5/qmultimedia.html#AvailabilityStatus-enum
void QvkWebcamController::displayWebcam( QByteArray device )
{
    camera = new QCamera( device );

    connect( camera, SIGNAL( statusChanged( QCamera::Status ) ), this, SLOT( myStatusChanged( QCamera::Status ) ) );
    connect( camera, SIGNAL( stateChanged( QCamera::State   ) ), this, SLOT( myStateChanged( QCamera::State ) )  );

    QCameraViewfinderSettings viewfinderSettings;
    viewfinderSettings.setResolution( 640, 480 );
    viewfinderSettings.setMinimumFrameRate( 0.0 );
    viewfinderSettings.setMaximumFrameRate( 0.0 );
    camera->setViewfinderSettings( viewfinderSettings );

    camera->setViewfinder( videoSurface );

    webcamWindow->setGeometry( 800, 400, 320, 240 );
    webcamWindow->show();

    camera->start();
}


void QvkWebcamController::myStatusChanged( QCamera::Status status )
{
    switch ( status )
    {
      case QCamera::UnavailableStatus : { qDebug() << "[vokoscreen]" << status; break; }// 0
      case QCamera::UnloadedStatus    : { qDebug() << "[vokoscreen]" << status; break; }// 1
      case QCamera::LoadingStatus     : { qDebug() << "[vokoscreen]" << status; break; }// 2
      case QCamera::UnloadingStatus   : { qDebug() << "[vokoscreen]" << status; break; }// 3
      case QCamera::LoadedStatus      : { qDebug() << "[vokoscreen]" << status; break; }// 4
      case QCamera::StandbyStatus     : { qDebug() << "[vokoscreen]" << status; break; }// 5
      case QCamera::StartingStatus    : { qDebug() << "[vokoscreen]" << status; break; }// 6
      case QCamera::StoppingStatus    : { qDebug() << "[vokoscreen]" << status; break; }// 7
      case QCamera::ActiveStatus      : { qDebug() << "[vokoscreen]" << status; break; }// 8
    }
}

void QvkWebcamController::myStateChanged( QCamera::State state )
{
    switch ( state )
    {
      case QCamera::UnloadedState : { qDebug() << "[vokoscreen]" << state; break;  };// 0
      case QCamera::LoadedState   : { qDebug() << "[vokoscreen]" << state; break;  };// 1
      case QCamera::ActiveState   : { qDebug() << "[vokoscreen]" << state; break;  };// 2
    }
}
