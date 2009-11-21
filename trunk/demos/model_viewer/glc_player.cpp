/****************************************************************************

 This file is part of GLC-Player.
 Copyright (C) 2007-2008 Laurent Ribon (laumaya@users.sourceforge.net)
 Version 2.1.0, packaged on September 2009.

 http://www.glc-player.net

 GLC-Player is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 GLC-Player is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with GLC-Player; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*****************************************************************************/

#include "glc_player.h"
#include "ui_class/SettingsDialog.h"
#include "ui_class/AboutPlayer.h"
#include "AlbumFile.h"
#include "ui_class/EditCamera.h"
#include "ui_class/SelectionProperty.h"
#include "ui_class/EditLightDialog.h"
#include "ui_class/AlbumManagerView.h"
#include "ui_class/ModelManagerView.h"
#include "ui_class/InstanceProperty.h"
#include "UserInterfaceSate.h"
#include "ui_class/MaterialProperty.h"
#include "ui_class/ChooseShaderDialog.h"
#include "ui_class/ListOfMaterial.h"
#include "ui_class/OpenAlbumOption.h"
#include "ui_class/SendFilesDialog.h"
#include "ui_class/ScreenshotDialog.h"
#include "ui_class/MultiScreenshotsDialog.h"
#include "ui_class/ExportWebDialog.h"
#include "ui_class/LeftSideDock.h"
#include <GLC_Exception>
#include <GLC_State>
#include <GLC_Distance>

#include <GLC_ExtendedMesh>

glc_player::glc_player(const bool useFrameBuffer, QWidget *parent)
: QMainWindow(parent)
, m_OpenglView(useFrameBuffer, this)
, m_CurrentPath(QDir::homePath())
, m_CurrentAlbumPath(QDir::homePath())
, m_pProgressBar(new QProgressBar())
, m_pQErrorMessage(NULL)
, m_RecentFilesList()
, m_RecentAlbumsList()
, m_CurrentFileName()
, m_CurrentAlbumName()
, m_OpenFileThread(GLC_Factory::instance(m_OpenglView.context()))
, m_FileEntryHash()
, m_modelName()
, m_FileLoadingInProgress(false)
, m_ListLoadingInProgress(false)
, m_ContinuListLoading(true)
, m_MakeFirstFileCurrent(true)
, m_pCurrentSelections(NULL)
, m_dislayInfoPanel()
, m_pSelectionProperty(NULL)
, m_pEditLightDialog(NULL)
, m_QuitConfirmation()
, m_pLeftSideDock(NULL)
, m_pAlbumManagerView(NULL)
, m_pModelManagerView(NULL)
, m_pInstanceProperty(NULL)
, m_pInstancePropertyVis(NULL)
, m_pMaterialProperty(NULL)
, m_pChooseShaderDialog(NULL)
, m_pListOfMaterial(NULL)
, m_pOpenAlbumOption(NULL)
, m_pSendFilesDialog(NULL)
, m_pScreenshotDialog(NULL)
, m_pMultiScreenshotsDialog(NULL)
, m_pExportWebDialog(NULL)
, m_UseSelectionShader(true)
, m_UseVbo(-1)
, m_UseShader(-1)
, m_DefaultLodValue(10)
, m_UsePixelCulling(true)
{
	ui.setupUi(this);
	//setUnifiedTitleAndToolBarOnMac(true);
	setCentralWidget(&m_OpenglView);
	m_OpenglView.setFocusPolicy(Qt::StrongFocus);

	// Album management dock area
	m_pAlbumManagerView= new AlbumManagerView(&m_OpenglView, &m_FileEntryHash, ui.albumManagementWindow);
	m_pModelManagerView= new ModelManagerView(&m_OpenglView, ui.action_Property, ui.actionHide_unselected, ui.albumManagementWindow);
	connect(m_pModelManagerView, SIGNAL(currentModelProperties()), m_pAlbumManagerView, SLOT(modelProperties()));
	// LeftSideDock
	m_pLeftSideDock= new LeftSideDock(m_pAlbumManagerView, m_pModelManagerView, &m_FileEntryHash, ui.albumManagementWindow);
	ui.albumManagementWindow->setWidget(m_pLeftSideDock);

	readSettings();
	GLC_State::setPixelCullingUsage(m_UsePixelCulling);
	GLC_3DViewInstance::setGlobalDefaultLod(m_DefaultLodValue);

	createRecentFileActionsArray();
	updateRecentsFiles();
	createRecentAlbumActionsArray();
	updateRecentsAlbums();

	// Default lighting
	m_OpenglView.getLight()->setTwoSided(ui.actionTwo_sided_Lightning->isChecked());

	// Display info panel
	m_OpenglView.setDisplayInfoPanel(m_dislayInfoPanel);

	// Resize Texture cache limite to 128 Mo
	QGLContext::setTextureCacheLimit(256 * 1024);

	// Accept drop event
	m_OpenglView.setAcceptDrops(false);
	setAcceptDrops(true);

	// Set the current file name
	addToRecentFiles(m_CurrentFileName);

	// Status bar
	ui.statusbar->addPermanentWidget(m_pProgressBar);
	m_pProgressBar->hide();
	connect(&m_OpenglView, SIGNAL(currentQuantum(int)), this, SLOT(updateProgressBar(int)), Qt::QueuedConnection);

	// QAction Signals and slots connection
	// Signals from the view
	connect(&m_OpenglView, SIGNAL(updateSelection(PointerNodeHash*)), this, SLOT(updateSelection(PointerNodeHash*)));
	connect(&m_OpenglView, SIGNAL(unselectAll()), this, SLOT(unselectAll()));
	connect(&m_OpenglView, SIGNAL(hideInfoPanel()), this, SLOT(hideInfoPanel()));
	connect(&m_OpenglView, SIGNAL(glInitialed()), this, SLOT(glInitialed()));
	//Menu File
	connect(ui.action_NewAlbum, SIGNAL(triggered()), this , SLOT(newAlbum()));
	connect(ui.action_OpenAlbum, SIGNAL(triggered()), this , SLOT(openAlbum()));
	connect(ui.action_SaveAlbum, SIGNAL(triggered()), this , SLOT(saveAlbum()));
	connect(ui.action_SaveAlbumAs, SIGNAL(triggered()), this , SLOT(saveAlbumAs()));
	connect(ui.action_Open, SIGNAL(triggered()), this, SLOT(open()));
	connect(ui.action_Quit, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));
	connect(ui.actionExport_To_Folder, SIGNAL(triggered()), this, SLOT(sendToFolder()));
	connect(ui.actionExport_to_web, SIGNAL(triggered()), this, SLOT(exportToWeb()));
	// Menu edit
	connect(ui.actionSelectAll, SIGNAL(triggered()), &m_OpenglView, SLOT(selectAll()));
	connect(ui.actionUnselectAll, SIGNAL(triggered()), &m_OpenglView, SLOT(unselectAllSlot()));
	connect(ui.action_Property, SIGNAL(triggered()), this, SLOT(instanceProperty()));
	connect(ui.actionAssign_Shader, SIGNAL(triggered()), this, SLOT(assignShader()));
	//Menu Window
	connect(ui.action_AlbumManagement, SIGNAL(triggered()), this, SLOT(albumManagementVisibilityToggle()));
	connect(ui.albumManagementWindow, SIGNAL(visibilityChanged(bool)), this, SLOT(albumManagementVisibilityChanged(bool)));
	connect(ui.action_CameraProperty, SIGNAL(triggered()), this, SLOT(cameraPropertyVisibilityToggle()));
	connect(ui.cameraProperties, SIGNAL(visibilityChanged(bool)), this, SLOT(cameraPropertyVisibilityChanged(bool)));
	connect(ui.action_SelectionProperty, SIGNAL(triggered()), this, SLOT(selectionPropertyVisibilityToggle()));
	connect(ui.selectionDockWidget, SIGNAL(visibilityChanged(bool)), this, SLOT(selectionPropertyVisibilityChanged(bool)));

	//Menu View
	connect(ui.actionChange_UP_Vector, SIGNAL(triggered()), &m_OpenglView, SLOT(changeDefaultUp()));
	connect(ui.action_Reframe, SIGNAL(triggered()), this, SLOT(reframe()));
	connect(ui.actionReframeOnSelection, SIGNAL(triggered()), &m_OpenglView, SLOT(reframeOnSelection()));
	QActionGroup* pGroup= new QActionGroup(this);
	pGroup->addAction(ui.actionTrackball);
	pGroup->addAction(ui.actionTurnTable);
	ui.actionTrackball->setChecked(true);
	connect(ui.actionTrackball, SIGNAL(triggered()), this, SLOT(changeCurrentMoverToTrackBall()));
	connect(ui.actionTurnTable, SIGNAL(triggered()), this, SLOT(changeCurrentMoverToTurnTable()));
	connect(ui.action_Select, SIGNAL(triggered()), this, SLOT(selectMode()));
	connect(ui.action_ViewCenter, SIGNAL(triggered()), this, SLOT(viewCenterMode()));
	connect(ui.action_Pan, SIGNAL(triggered()), this, SLOT(panMode()));
	connect(ui.action_Rotate, SIGNAL(triggered()), this, SLOT(rotateMode()));
	connect(ui.action_Zoom, SIGNAL(triggered()), this, SLOT(zoomMode()));
	connect(ui.action_ZoomIn, SIGNAL(triggered()), &m_OpenglView, SLOT(zoomIn()));
	connect(ui.action_ZoomOut, SIGNAL(triggered()), &m_OpenglView, SLOT(zoomOut()));
	connect(ui.actionShow_Hide, SIGNAL(triggered()), this, SLOT(showOrHide()));
	connect(ui.actionHide_unselected, SIGNAL(triggered()), this, SLOT(hideUnselected()));
	connect(ui.actionShow_all, SIGNAL(triggered()), this, SLOT(showAll()));
	connect(ui.actionSwap_visible_space, SIGNAL(triggered()), this, SLOT(swapVisibleSpace()));
	connect(ui.action_FullScreen, SIGNAL(triggered()), this, SLOT(fullScreen()));

	// predifined view
	connect(ui.action_IsoView1, SIGNAL(triggered()), &m_OpenglView, SLOT(isoView1()));
	connect(ui.action_IsoView2, SIGNAL(triggered()), &m_OpenglView, SLOT(isoView2()));
	connect(ui.action_IsoView3, SIGNAL(triggered()), &m_OpenglView, SLOT(isoView3()));
	connect(ui.action_IsoView4, SIGNAL(triggered()), &m_OpenglView, SLOT(isoView4()));
	connect(ui.action_FrontView, SIGNAL(triggered()), &m_OpenglView, SLOT(frontView()));
	connect(ui.action_RightView, SIGNAL(triggered()), &m_OpenglView, SLOT(rightView()));
	connect(ui.action_TopView, SIGNAL(triggered()), &m_OpenglView, SLOT(topView()));

	// Menu Render
	// Render Mode
	connect(ui.action_RenderPoints, SIGNAL(triggered()), this, SLOT(pointsRenderingMode()));
	connect(ui.action_RenderWireframe, SIGNAL(triggered()), this, SLOT(wireframeRenderingMode()));
	connect(ui.action_RenderShading, SIGNAL(triggered()), this, SLOT(shadingRenderingMode()));
	connect(ui.action_EditLight, SIGNAL(triggered()), this, SLOT(editLightDialog()));
	connect(ui.actionTwo_sided_Lightning, SIGNAL(triggered()), this, SLOT(twoSidedLightning()));
	connect(ui.actionSet_Shader, SIGNAL(triggered()), this, SLOT(chooseShader()));

	// Menu Tools
	connect(ui.action_Settings, SIGNAL(triggered()), this, SLOT(showSettings()));
	connect(ui.action_SnapShot, SIGNAL(triggered()), this, SLOT(takeSnapShot()));
	connect(ui.actionMultiShots, SIGNAL(triggered()), this, SLOT(takeMultiShots()));

	// Menu Help
	connect(ui.action_About, SIGNAL(triggered()), this, SLOT(aboutPlayer()));
	connect(ui.action_Help, SIGNAL(triggered()), this, SLOT(help()));

	// Open File thread
	connect(&m_OpenFileThread, SIGNAL(currentQuantum(int)), this, SLOT(updateProgressBar(int)), Qt::QueuedConnection);
	connect(&m_OpenFileThread, SIGNAL(finished()), this, SLOT(fileOpened()), Qt::QueuedConnection);
	connect(&m_OpenFileThread, SIGNAL(loadError()), this, SLOT(loadFileFailed()), Qt::QueuedConnection);

	// Album manager view
	connect(m_pAlbumManagerView, SIGNAL(computeIconInBackBuffer(int)), this , SLOT(computeIconInBackBuffer(int)));
	connect(m_pAlbumManagerView, SIGNAL(removeUnloadFileItem()), this , SLOT(removeUnloadFileItem()));
	connect(m_pAlbumManagerView, SIGNAL(removeOnErrorModels()), this , SLOT(removeItemOnError()));
	connect(m_pAlbumManagerView, SIGNAL(startLoading()), this , SLOT(startLoadingButton()));
	connect(m_pAlbumManagerView, SIGNAL(stopLoading()), this , SLOT(stopLoadingButton()));
	connect(m_pAlbumManagerView, SIGNAL(newAlbum(bool)), this, SLOT(newAlbum(bool)));
	connect(m_pAlbumManagerView, SIGNAL(deleteModel(GLC_uint)), this, SLOT(deleteItem(GLC_uint)));
	connect(m_pAlbumManagerView, SIGNAL(reloadCurrentModelSignal(const GLC_uint)), this, SLOT(reloadModel(const GLC_uint)));
	connect(m_pAlbumManagerView, SIGNAL(currentModelChanged(QListWidgetItem *, QListWidgetItem *))
			, this , SLOT(currentFileItemChanged(QListWidgetItem *, QListWidgetItem *)));
	connect(m_pAlbumManagerView, SIGNAL(displayMessage(QString)), this , SLOT(displayMessageInStatusBar(QString)));

	// Camera Property dock area
	EditCamera* pEditCamWidget= new EditCamera(m_OpenglView.getViewport(), ui.cameraProperties);
	ui.cameraProperties->setWidget(pEditCamWidget);
	connect(&m_OpenglView, SIGNAL(viewChanged()), pEditCamWidget, SLOT(updateValues()));
	connect(pEditCamWidget, SIGNAL(valueChanged()), this, SLOT(updateView()));

	// Current selection dock area
	m_pSelectionProperty= new SelectionProperty(ui.actionShow_Hide, ui.actionAssign_Shader, ui.action_Property, ui.selectionDockWidget);
	ui.selectionDockWidget->setWidget(m_pSelectionProperty);
	connect(m_pSelectionProperty, SIGNAL(updateView()), &m_OpenglView, SLOT(updateGL()));

	// Parse arguments command line
	QStringList args= QCoreApplication::arguments ();
	if (args.size() > 1)
	{
		QStringList argList(QFileInfo(args[1]).filePath());
		if (QFileInfo(argList[0]).suffix().toLower() == "album")
		{
			openAlbum(argList[0]);
		}
		else
		{
			addItems(argList);
			startLoading();
		}
	}
	else
	{
		// Update UI
		ui.action_NewAlbum->setEnabled(false);
		ui.action_SaveAlbumAs->setEnabled(false);
		ui.action_SaveAlbum->setEnabled(false);
		ui.actionExport_To_Folder->setEnabled(false);
		ui.actionExport_to_web->setEnabled(false);
		setWindowTitle(QCoreApplication::applicationName());
		ui.statusbar->showMessage(tr("Untiteled"));
	}
	// PFC MOD Starts here
	// Originalmente la imagen no se refresca a no ser que haya cambios, asi que necesitamos forzar el refresco
	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), &m_OpenglView, SLOT(updateGL()));
	// eñ reloj tickea cada 20ms ~= 50fps
	timer->start(20);
	// PFC MOD ends here


}

glc_player::~glc_player()
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	// delete the progress bar
	if (NULL != m_pProgressBar)
	{
		delete m_pProgressBar;
	}

	// delete error message dialog
	if (NULL != m_pQErrorMessage)
	{
		delete m_pQErrorMessage;
	}

	if (NULL != m_pScreenshotDialog)
	{
		delete m_pScreenshotDialog;
	}

	if (m_pEditLightDialog != NULL) delete m_pEditLightDialog;

	if (NULL != m_pChooseShaderDialog) delete m_pChooseShaderDialog;

	m_pAlbumManagerView->clear();
	m_FileEntryHash.clear();
	m_modelName.clear();
	QApplication::restoreOverrideCursor();
}
//////////////////////////////////////////////////////////////////////
// Virtual protected function
//////////////////////////////////////////////////////////////////////
// Close event handler
void glc_player::closeEvent(QCloseEvent* pEvent)
{
	int ret= QMessageBox::Yes;
	// Display confirmation msgBox only if there album is not empty
	// And Quit confirmation is set to yes
	if ((!m_FileEntryHash.empty()) and m_QuitConfirmation)
	{
		ret= QMessageBox::question(this, QCoreApplication::applicationName(),
				QString(tr("Quits ")) +  QCoreApplication::applicationName() + QString("?"), QMessageBox::Yes | QMessageBox::No);
	}
	if (ret == QMessageBox::Yes)
	{
		// Return to normal mode if needed
		if (UserInterfaceSate::globalState() == INSTANCE_STATE)
		{
			returnToNormalMode();
		}
		m_pAlbumManagerView->blockSignals(true);
		writeSettings();
		pEvent->accept();
		QCoreApplication::quit();
	}
	else
	{
		pEvent->ignore();
	}
}

//////////////////////////////////////////////////////////////////////
// public Methods
//////////////////////////////////////////////////////////////////////
void glc_player::openOnEvent(QString fileName)
{

	const bool fileIsAlbum= (QFileInfo(fileName).suffix().toLower() == "album");
	if (fileIsAlbum and ui.action_OpenAlbum->isEnabled())
	{
		openAlbum(fileName);
	}
	else if (not fileIsAlbum)
	{
		QStringList localFiles;
		localFiles.append(fileName);
		addItems(localFiles);
		startLoading();
	}

}

//////////////////////////////////////////////////////////////////////
// public Slots
//////////////////////////////////////////////////////////////////////
// The selection as been changed
void glc_player::updateSelection(PointerNodeHash* pSelections)
{
	if (pSelections->isEmpty())
	{
		unselectAll();
	}
	else
	{
		m_pCurrentSelections= pSelections;
		m_pSelectionProperty->setSelection(m_pCurrentSelections);
		if(pSelections->size() == 1 and not (m_ContinuListLoading and m_ListLoadingInProgress))
		{
			ui.action_Property->setEnabled(true);
			m_pSelectionProperty->editPropertySetEnabled(true);

		}
		else
		{
			ui.action_Property->setEnabled(false);
			m_pSelectionProperty->editPropertySetEnabled(false);
		}
		ui.actionUnselectAll->setEnabled(true);
		if (GLC_State::glslUsed()) ui.actionAssign_Shader->setEnabled(true);
		ui.actionShow_Hide->setEnabled(true);
		ui.actionHide_unselected->setEnabled(true);
		ui.actionReframeOnSelection->setEnabled(true);

	}
}

// Info panel not supported
void glc_player::hideInfoPanel()
{
	m_dislayInfoPanel= false;
	writeSettings();
	QCoreApplication::quit();
}
//////////////////////////////////////////////////////////////////////
// private Slots
//////////////////////////////////////////////////////////////////////

// The selection is Empty
void glc_player::unselectAll()
{
	m_pCurrentSelections= NULL;
	m_pSelectionProperty->unsetSelection();
	ui.action_Property->setEnabled(false);
	m_pSelectionProperty->editPropertySetEnabled(false);
	ui.actionUnselectAll->setEnabled(false);
	if (GLC_State::glslUsed()) ui.actionAssign_Shader->setEnabled(false);
	ui.actionShow_Hide->setEnabled(false);
	ui.actionHide_unselected->setEnabled(false);
	ui.actionReframeOnSelection->setEnabled(false);
}

// Set Instance visibility
void glc_player::showOrHide()
{
	if ((m_pAlbumManagerView->haveCurrentModel()) && (NULL!= m_pCurrentSelections))
	{
		GLC_uint modelId(m_pAlbumManagerView->currentModelId());
		// Take the entry from hash table
		PointerNodeHash::iterator iEntry= m_pCurrentSelections->begin();
	    while (iEntry != m_pCurrentSelections->constEnd())
	    {
			m_FileEntryHash.value(modelId).getWorld().collection()->setVisibility(iEntry.value()->id(), !iEntry.value()->isVisible());
			iEntry++;
	    }
		m_OpenglView.updateGL();
		m_pLeftSideDock->updateSelectedTreeShowNoShow();
	}

}
// Hide unseleted instance
void glc_player::hideUnselected()
{
	if ((m_pAlbumManagerView->haveCurrentModel()) && (NULL!= m_pCurrentSelections))
	{
		GLC_uint modelId(m_pAlbumManagerView->currentModelId());
		// Take the entry from hash table
		m_FileEntryHash.value(modelId).getWorld().collection()->hideAll();
		PointerNodeHash::iterator iEntry= m_pCurrentSelections->begin();
	    while (iEntry != m_pCurrentSelections->constEnd())
	    {
			m_FileEntryHash.value(modelId).getWorld().collection()->setVisibility(iEntry.value()->id(), true);
			iEntry++;
	    }
		m_OpenglView.updateGL();
		m_pLeftSideDock->updateTreeShowNoShow();
	}

}

// Show All instance
void glc_player::showAll()
{
	if (m_pAlbumManagerView->haveCurrentModel())
	{
		GLC_uint modelId(m_pAlbumManagerView->currentModelId());
		// Show all model of the entry collection
		m_FileEntryHash.value(modelId).getWorld().collection()->showAll();
		m_OpenglView.setDistMinAndMax();
		m_OpenglView.updateGL();
		m_pLeftSideDock->updateTreeShowNoShow();
	}

}

// Swap visible space
void glc_player::swapVisibleSpace()
{
	if (m_pAlbumManagerView->haveCurrentModel())
	{
		m_OpenglView.swapVisibleSpace();
		m_OpenglView.setDistMinAndMax();
		m_OpenglView.updateGL();
	}
}

// update the value of the progress bar
void glc_player::updateProgressBar(int value)
{
	if (value < 100)
	{
		ui.statusbar->showMessage(tr("Loading in Progress Please Wait"));
		m_pProgressBar->show();
		m_pProgressBar->setValue(value);
	}
	else
	{
		ui.statusbar->showMessage(tr("File Loaded"));
		m_pProgressBar->hide();
	}
}


//////////////////////////////////////////////////////////////////////
// private Slots
//////////////////////////////////////////////////////////////////////

// Remove all file item from the current album
bool glc_player::newAlbum(bool confirmation)
{
	if (!m_FileLoadingInProgress)
	{
		int ret= QMessageBox::No;
		if (confirmation)
		{
			ret= QMessageBox::question(this, tr("New Album Confirmation"),
								 	tr("Remove all models from the current album?"), QMessageBox::Yes | QMessageBox::No);
		}
		if (!confirmation || (ret == QMessageBox::Yes))
		{
			QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
			unselectAll();
			//ui.loadingList->blockSignals(true);
			m_pAlbumManagerView->clear();
			m_pModelManagerView->clear();
			//ui.loadingList->blockSignals(false);
			m_FileEntryHash.clear();
			m_modelName.clear();

			// Update Continu list loading flag
			m_ContinuListLoading= true;
			m_CurrentAlbumName.clear();

			// Update UI
			ui.action_NewAlbum->setEnabled(false);
			ui.action_SaveAlbumAs->setEnabled(false);
			ui.action_SaveAlbum->setEnabled(false);
			ui.actionExport_To_Folder->setEnabled(false);
			ui.actionExport_to_web->setEnabled(false);
			setWindowTitle(QCoreApplication::applicationName());
			ui.statusbar->showMessage(tr("Untiteled"));
			m_MakeFirstFileCurrent= true;
			QApplication::restoreOverrideCursor();
			return true;
		}
		else return false;
	}
	else return false;
}

// Open An existing file
void glc_player::open()
{
	// Define File Format filter
	QStringList filters;
	filters.append(tr("All Known format(*.obj *.OBJ *.3ds *.3DS *.stl *.STL *.off *.OFF *.3DXML *.3dxml *.DAE *.dae)"));
	filters.append(tr("Alias File Format OBJ (*.obj *.OBJ)"));
	filters.append(tr("3D Studio File Format 3DS (*.3ds *.3DS)"));
	filters.append(tr("STL File Format STL (*.stl *.STL)"));
	filters.append(tr("Object File Format OFF (*.off *.OFF)"));
	filters.append(tr("Dassault Systemes 3DXML(*.3dxml *.3DXML)"));
	filters.append(tr("Sony Collada(*.dae *.DAE)"));

	QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Select File(s) to Open and Add in album")
	, m_CurrentPath, filters.join("\n"));
	if (!fileNames.isEmpty())
	{
		addItems(fileNames);
		startLoading();
	}
}
// Open An existing album
void glc_player::openAlbum()
{
	if (!m_FileLoadingInProgress)
	{
		QString fileName = QFileDialog::getOpenFileName(this, tr("Select Album File to Open")
		, m_CurrentAlbumPath, "GLC_Player Album (*.album)");
		if (!fileName.isEmpty())
		{
			openAlbum(fileName);
		}
	}
}
// Save the current album
void glc_player::saveAlbum()
{
	if (m_CurrentAlbumName.isEmpty())
	{
		saveAlbumAs();
	}
	else
	{
		applySavingAlbum(m_CurrentAlbumName);
	}

}

// Save As a new album
void glc_player::saveAlbumAs()
{
	QString currentPath;
	if (!m_CurrentAlbumName.isEmpty())
	{
		currentPath= m_CurrentAlbumName;
	}
	else
	{
		currentPath= m_CurrentAlbumPath;
	}
	const QString suffix(".album");
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Album As ")
	, currentPath, tr("GLC_Player Album (*.album)"));
	if (!fileName.isEmpty())
	{
		if (!fileName.endsWith(suffix))
		{
			fileName.append(suffix);
		}
		applySavingAlbum(fileName);

	}
}

// Open a Recent File
void glc_player::openRecentFile()
{
	QAction* action= qobject_cast<QAction*>(sender());
	if (action)
	{
		addItems(QStringList(action->data().toString()));
		startLoading();
	}
}

// Open a Recent Album
void glc_player::openRecentAlbum()
{
	QAction* action= qobject_cast<QAction*>(sender());
	if (action)
	{
		const QString albumName(action->data().toString());
		qDebug() << albumName;
		openAlbum(albumName);
	}
}

// Send File to folder
void glc_player::sendToFolder()
{
	if (NULL == m_pSendFilesDialog)
	{
		m_pSendFilesDialog= new SendFilesDialog(&m_FileEntryHash, this);
	}
	m_pSendFilesDialog->updateView(m_CurrentAlbumName);
	if (m_pSendFilesDialog->exec() == QDialog::Accepted)
	{
		// OK we have to send some files
		if (m_pSendFilesDialog->sendFiles())
		{
			if (m_pSendFilesDialog->copyAlbum() and m_pSendFilesDialog->updateAlbum())
			{
				m_CurrentAlbumName= m_pSendFilesDialog->newAlbumFileName();
				saveAlbum();
			}
			QMessageBox::information(this, tr("Send To Folder"), tr("Files succesfuly send"));
		}
	}
}

// Export album to web
void glc_player::exportToWeb()
{
	// Update current entry
	FileEntryHash::iterator iEntry= m_FileEntryHash.find(m_pAlbumManagerView->currentModelId());

	iEntry.value().setCameraAndAngle(m_OpenglView.getCamera(), m_OpenglView.getViewAngle());
	iEntry.value().setPolygonMode(m_OpenglView.getMode());

	if (NULL == m_pExportWebDialog)
	{
		m_pExportWebDialog= new ExportWebDialog(this);
	}
	m_pExportWebDialog->initDialog(QFileInfo(m_CurrentAlbumName).baseName(), m_pAlbumManagerView->sortedFileEntryList(), &m_OpenglView);
	m_pExportWebDialog->exec();
}

// View and edit instance property
void glc_player::instanceProperty()
{
	// in Instance property mode selection is disable
	m_OpenglView.blockSelection(true);

	// Create instance world
	GLC_World instanceWorld;
	GLC_World CurrentWorld(m_FileEntryHash.value(m_pAlbumManagerView->currentModelId()).getWorld());
	instanceWorld.mergeWithAnotherWorld(CurrentWorld);
	// Get the instance
	GLC_3DViewInstance* pInstance= instanceWorld.collection()->selection()->begin().value();

	instanceWorld.collection()->hideAll();
	instanceWorld.collection()->setVisibility(pInstance->id(), true);
	instanceWorld.collection()->unselectAll();
	if (not m_OpenglView.getVisibleState())
	{
		m_OpenglView.setToVisibleState();
	}
	m_OpenglView.add(instanceWorld);


	// Save and Update user Interface state
	UserInterfaceSate::SetGlobalState(INSTANCE_STATE);
	UserInterfaceSate::albumMangerVisibility(ui.albumManagementWindow->isVisible());
	ui.albumManagementWindow->hide();
	UserInterfaceSate::cameraPropertyVisibility(ui.cameraProperties->isVisible());
	ui.cameraProperties->hide();
	UserInterfaceSate::selectionPropertyVisibility(ui.selectionDockWidget->isVisible());
	ui.selectionDockWidget->hide();

	// Create instance property dock window if needed or show it
	if (m_pInstanceProperty == NULL)
	{
		m_pInstanceProperty= new InstanceProperty(&m_OpenglView, ui.actionAssign_Shader, this);
	}

	addDockWidget(Qt::LeftDockWidgetArea, m_pInstanceProperty);
	m_pInstanceProperty->show();


//////////////////// Update Window menu////////////////////////////////////
	// Disable dockWindow visibility change
	ui.action_AlbumManagement->setEnabled(false);
	ui.action_CameraProperty->setEnabled(false);
	ui.action_SelectionProperty->setEnabled(false);
	// Menu edit
	ui.actionSelectAll->setEnabled(false);
	ui.actionUnselectAll->setEnabled(false);
	ui.action_Property->setEnabled(false);
	m_pSelectionProperty->editPropertySetEnabled(false);
	//Menu View
	ui.actionReframeOnSelection->setEnabled(false);
	ui.actionShow_Hide->setEnabled(false);
	ui.actionHide_unselected->setEnabled(false);
	ui.actionShow_all->setEnabled(false);
	ui.actionSwap_visible_space->setEnabled(false);
	//Menu File
	ui.action_NewAlbum->setEnabled(false);
	ui.action_OpenAlbum->setEnabled(false);
	ui.menuRecent_Album->setEnabled(false);
	ui.action_SaveAlbum->setEnabled(false);
	ui.action_SaveAlbumAs->setEnabled(false);
	ui.actionExport_To_Folder->setEnabled(false);
	ui.actionExport_to_web->setEnabled(false);
	ui.action_Open->setEnabled(false);
	ui.menuRecent_Models->setEnabled(false);

	// Add instanceProperty visibility action
	if (m_pInstancePropertyVis == NULL)
	{
		m_pInstancePropertyVis= new QAction(tr("Instance Property"), this);
		m_pInstancePropertyVis->setCheckable(true);
	}
	m_pInstancePropertyVis->setChecked(true);
	ui.menu_Window->addAction(m_pInstancePropertyVis);
	// Create action conection
	connect(m_pInstancePropertyVis, SIGNAL(triggered()), this, SLOT(instancePropertyVisibilityToggle()));
	connect(m_pInstanceProperty, SIGNAL(visibilityChanged(bool)), this, SLOT(instancePropertyVisibilityChanged(bool)));
	connect(m_pInstanceProperty, SIGNAL(doneSignal()), this, SLOT(returnToNormalMode()));
	connect(m_pInstanceProperty, SIGNAL(updateView()), &m_OpenglView, SLOT(updateGL()));
	connect(m_pInstanceProperty, SIGNAL(viewSubMaterialList()), this, SLOT(viewListOfMaterial()));
	m_pInstanceProperty->setInstance(pInstance);
////////////////////////////////////////////////////////////////////////////

	// Reframe
	m_OpenglView.reframe(pInstance->boundingBox(), true);
	// Disable edit menu action
	ui.actionSelectAll->setEnabled(false);
}

// View and edit material property
void glc_player::viewMaterialProperty(GLC_Material* pMaterial)
{
	m_OpenglView.doneCurrent();
	if (NULL == m_pMaterialProperty)
	{
		m_pMaterialProperty= new MaterialProperty(&m_OpenglView, pMaterial, this);
		addDockWidget(Qt::RightDockWidgetArea, m_pMaterialProperty);
		if ((NULL != m_pListOfMaterial) and m_pListOfMaterial->isVisible())
		{
			connect(m_pMaterialProperty, SIGNAL(materialUpdated(GLC_Material*)), m_pListOfMaterial, SLOT(updateRow()));
			connect(m_pMaterialProperty, SIGNAL(materialUpdated(GLC_Material*)), this, SLOT(updateCurrentEntryMaterial(GLC_Material*)));
		}
	}
	else
	{
		addDockWidget(Qt::RightDockWidgetArea, m_pMaterialProperty);
		m_pMaterialProperty->setMaterial(pMaterial);
	}

	m_pMaterialProperty->show();
	//m_pMaterialProperty->updatePreview();

}
// View the list of material
void glc_player::viewListOfMaterial()
{
	if (NULL == m_pListOfMaterial)
	{
		m_pListOfMaterial= m_pInstanceProperty->getListOfMaterials();
	}
	connect(m_pListOfMaterial, SIGNAL(updateMaterialSignal(GLC_Material*)), this, SLOT(viewMaterialProperty(GLC_Material*)));
	m_pInstanceProperty->createOrUpdateListOfMaterial();

}


// Use the toon shader
void glc_player::assignShader()
{
	if (NULL == m_pChooseShaderDialog)
	{
		m_pChooseShaderDialog= new ChooseShaderDialog(&m_OpenglView, this);
	}

// -------------Prepare world for the choose shader dialog----------------
	GLC_World ThumbnailsWorld;
	GLC_World CurrentWorld(m_OpenglView.getWorld());
	ThumbnailsWorld.mergeWithAnotherWorld(CurrentWorld);
	// Hide unselected object
	if (UserInterfaceSate::globalState() != INSTANCE_STATE)
	{
		ThumbnailsWorld.collection()->hideAll();
		PointerNodeHash* pSelection= ThumbnailsWorld.collection()->selection();
		PointerNodeHash::iterator iEntry= pSelection->begin();
	    while (iEntry != pSelection->constEnd())
	    {
	    	ThumbnailsWorld.collection()->setVisibility(iEntry.value()->id(), true);
	    	ThumbnailsWorld.collection()->changeShadingGroup(iEntry.value()->id(), 0);
			iEntry++;
	    }
	    ThumbnailsWorld.collection()->unselectAll();

	}
	else
	{
		const GLC_uint id= ThumbnailsWorld.collection()->visibleInstanceHandle().first()->id();
		ThumbnailsWorld.collection()->changeShadingGroup(id, 0);
	}
    m_OpenglView.add(ThumbnailsWorld);
    // Save previous global shader
    const GLuint oldShaderId= m_OpenglView.globalShaderId();
    m_OpenglView.setGlobalShaderId(0, QString());

    // Update choose shader dialog thumbnails
	m_pChooseShaderDialog->UpdateThumbnailsList();

	// Restore the world
	m_OpenglView.setGlobalShaderId(oldShaderId, QString());
	m_OpenglView.add(CurrentWorld);

// ------------------------------------------------------------------------

	CurrentWorld= m_FileEntryHash.value(m_pAlbumManagerView->currentModelId()).getWorld();
	if (m_pChooseShaderDialog->exec() == QDialog::Accepted)
	{
		GLC_World CurrentWorld(m_FileEntryHash.value(m_pAlbumManagerView->currentModelId()).getWorld());
		const GLuint shaderId= m_pChooseShaderDialog->shaderId();
		// Bind shader to main world

		if (0 != shaderId)
		{
			CurrentWorld.collection()->bindShader(shaderId);
		}

		// Assign shader to the selection
		PointerNodeHash* pSelection= CurrentWorld.collection()->selection();
		PointerNodeHash::iterator iEntry= pSelection->begin();
	    while (iEntry != pSelection->constEnd())
	    {
	    	CurrentWorld.collection()->changeShadingGroup(iEntry.value()->id(), shaderId);
	    	++iEntry;
	    }

	    if (UserInterfaceSate::globalState() == INSTANCE_STATE)
	    {
	        // Assign shader to instance world
	    	CurrentWorld= m_OpenglView.getWorld();
	    	GLC_uint id= CurrentWorld.collection()->visibleInstanceHandle().first()->id();
	    	if (0 != shaderId)
	    	{
	    		CurrentWorld.collection()->bindShader(shaderId);
	    	}
	    	CurrentWorld.collection()->changeShadingGroup(id, shaderId);
	    }

		m_OpenglView.updateGL();

	}

}

// Reframe the view
void glc_player::reframe()
{
	m_OpenglView.reframe(GLC_BoundingBox(), true);
}
// Select mode
void glc_player::selectMode()
{
	if (!ui.action_Select->isChecked())
	{
		ui.action_Select->setChecked(true);
	}
	ui.action_ViewCenter->setChecked(false);
	ui.action_Pan->setChecked(false);
	ui.action_Rotate->setChecked(false);
	ui.action_Zoom->setChecked(false);
	m_OpenglView.changeEnterState(VE_NORMAL);
}

// define view center
void glc_player::viewCenterMode()
{
	if (!ui.action_ViewCenter->isChecked())
	{
		ui.action_ViewCenter->setChecked(true);
	}
	ui.action_Select->setChecked(false);
	ui.action_Pan->setChecked(false);
	ui.action_Rotate->setChecked(false);
	ui.action_Zoom->setChecked(false);
	m_OpenglView.changeEnterState(VE_POINTING);
}

// Pan the view
void glc_player::panMode()
{
	if (!ui.action_Pan->isChecked())
	{
		ui.action_Pan->setChecked(true);
	}
	ui.action_Select->setChecked(false);
	ui.action_ViewCenter->setChecked(false);
	ui.action_Rotate->setChecked(false);
	ui.action_Zoom->setChecked(false);
	m_OpenglView.changeEnterState(VE_PANNING);
}

// Rotate the view
void glc_player::rotateMode()
{
	if (!ui.action_Rotate->isChecked())
	{
		ui.action_Rotate->setChecked(true);
	}
	ui.action_Select->setChecked(false);
	ui.action_ViewCenter->setChecked(false);
	ui.action_Pan->setChecked(false);
	ui.action_Zoom->setChecked(false);
	m_OpenglView.changeEnterState(VE_ORBITING);
}

// Zoom the view mode
void glc_player::zoomMode()
{
	if (!ui.action_Zoom->isChecked())
	{
		ui.action_Zoom->setChecked(true);
	}
	ui.action_Select->setChecked(false);
	ui.action_ViewCenter->setChecked(false);
	ui.action_Pan->setChecked(false);
	ui.action_Rotate->setChecked(false);
	m_OpenglView.changeEnterState(VE_ZOOMING);
}

// Show application settings dialog
void glc_player::showSettings()
{
	QSize iconSize= m_pAlbumManagerView->iconSize();
	const bool displayThumbnails= m_pAlbumManagerView->thumbnailsAreDisplay();

	SettingsDialog settingsDialog(displayThumbnails, iconSize, m_dislayInfoPanel, m_QuitConfirmation
			, m_OpenglView.useFrameBuffer(), m_UseSelectionShader, (m_UseVbo == 1), (m_UseShader == 1)
			, m_DefaultLodValue, m_UsePixelCulling, this);
	if (settingsDialog.exec() == QDialog::Accepted)
	{
		bool update= displayThumbnails != settingsDialog.thumbnailHaveToBeDisplay();
		update= update or (iconSize != settingsDialog.getThumbnailSize());
		if (update)
		{
			m_pAlbumManagerView->setThumbnailsDisplay(settingsDialog.thumbnailHaveToBeDisplay());
			m_pAlbumManagerView->setIconSize(settingsDialog.getThumbnailSize());
			settingsDialog.hide();
		}
		if (m_dislayInfoPanel != settingsDialog.infoHaveToBeShown())
		{
			m_dislayInfoPanel= settingsDialog.infoHaveToBeShown();
			m_OpenglView.setDisplayInfoPanel(m_dislayInfoPanel);
		}

		if ((m_UseVbo == 1) != settingsDialog.vboIsUsed())
		{
			m_UseVbo= static_cast<int>(settingsDialog.vboIsUsed());
		}
		// Shader Usage
		if ((m_UseShader == 1) != settingsDialog.shaderIsUsed())
		{
			m_UseShader= static_cast<int>(settingsDialog.shaderIsUsed());
			m_UseSelectionShader= (1 == m_UseShader);
		}
		else if (m_UseSelectionShader != settingsDialog.selectionShaderIsUsed())
		{
			m_UseSelectionShader= not m_UseSelectionShader;
			GLC_State::setSelectionShaderUsage(m_UseSelectionShader);
		}

		if (m_DefaultLodValue != settingsDialog.defaultLodValue())
		{
			m_DefaultLodValue= settingsDialog.defaultLodValue();
			GLC_3DViewInstance::setGlobalDefaultLod(m_DefaultLodValue);
			FileEntryHash::iterator iEntry= m_FileEntryHash.begin();
			while (m_FileEntryHash.constEnd() != iEntry)
			{
				iEntry.value().setDefaultLodValue(m_DefaultLodValue);
				++iEntry;
			}
		}

		if (m_UsePixelCulling != settingsDialog.pixelCullingIsUsed())
		{
			m_UsePixelCulling= settingsDialog.pixelCullingIsUsed();
			GLC_State::setPixelCullingUsage(m_UsePixelCulling);
		}

		m_QuitConfirmation= settingsDialog.quitConfirmation();
	}

}
// A file as been Opened
void glc_player::fileOpened()
{
	GLC_World world(m_OpenFileThread.getWorld());
	const GLC_uint modelId= m_OpenFileThread.getModelId();
	const QString fileName(m_FileEntryHash[modelId].getFileName());
	// Check if the world as been successfully built
	if (not world.isEmpty())
	{
		m_FileEntryHash[modelId].setWorld(world);
		m_FileEntryHash[modelId].setAttachedFileNames(m_OpenFileThread.attachedFiles());

		// Update the foreground of the item Opened file
		int loadedItem= m_pAlbumManagerView->modelLoaded(modelId);

		// Test if the loaded model have to be set as current
		if (m_MakeFirstFileCurrent)
		{
			//Update window Title
			setWindowTitle(QCoreApplication::applicationName() + QString(" [") + QFileInfo(fileName).fileName() + QString("]"));
			// Set loading file as current item
			if (m_pAlbumManagerView->isCurrent(loadedItem))
			{
				m_pAlbumManagerView->setCurrent(loadedItem);
			}
			else	// force the display of loaded file
			{
				currentFileItemChanged(m_pAlbumManagerView->currentItem(), NULL);
			}
		}
		else if (m_pAlbumManagerView->item(loadedItem) == m_pAlbumManagerView->currentItem())
		{
			// force the display of loaded file
			currentFileItemChanged(m_pAlbumManagerView->currentItem(), NULL);
		}
		// Create the icon of the loaded file
		if (m_pAlbumManagerView->thumbnailsAreDisplay())
		{
			computeIconInBackBuffer(loadedItem);
		}

		addToRecentFiles(fileName);
		m_FileLoadingInProgress= false;
		m_MakeFirstFileCurrent= (m_pAlbumManagerView->numberOfUnloadedModels() == 0);
		m_FileEntryHash[modelId].setLoadingStatus(false);
	}
	// If there is a other file item, load it
	startLoading();
}
// load of File failed
void glc_player::loadFileFailed()
{
	ui.statusbar->showMessage(tr("File Not Loaded"));
	m_pProgressBar->hide();

	// Update the foreground of the item Opened file
	const GLC_uint modelId= m_OpenFileThread.getModelId();
	m_pAlbumManagerView->modelLoadFailed(modelId);

	// Update the entry
	m_FileEntryHash[modelId].setLoadingStatus(false);
	m_FileEntryHash[modelId].setError(m_OpenFileThread.getErrorMsg());
	m_MakeFirstFileCurrent= (m_pAlbumManagerView->numberOfUnloadedModels() == 0);
	m_FileLoadingInProgress= false;

}
// Current file Item Changed
void glc_player::currentFileItemChanged(QListWidgetItem * current, QListWidgetItem * previous )
{
	if ((previous != NULL) && (current != NULL))
	{
		if (NULL != m_pCurrentSelections)
		{
			unselectAll();
		}
		FileEntryHash::iterator iEntry= m_FileEntryHash.find(previous->data(Qt::UserRole).toUInt());
		if (iEntry.value().isLoaded() && (!(iEntry.value().getCamera() == m_OpenglView.getCamera())
											|| (iEntry.value().getPolygonMode() != m_OpenglView.getMode())
											|| iEntry.value().getViewAngle() != m_OpenglView.getViewAngle()))
		{
			iEntry.value().setCameraAndAngle(m_OpenglView.getCamera(), m_OpenglView.getViewAngle());
			iEntry.value().setPolygonMode(m_OpenglView.getMode());
			if (m_pAlbumManagerView->thumbnailsAreDisplay())
			{
				computeIconInBackBuffer(m_pAlbumManagerView->row(previous), true);
			}
		}
	}
	if (current != NULL)
	{
		setCurrentFileItem(current->data(Qt::UserRole).toUInt());
	}
	else
	{
		if (m_OpenglView.getVisibleState() == false)
		{
			m_OpenglView.setToVisibleState();
		}
		m_OpenglView.clear();
		m_OpenglView.updateGL();
	}
	// Update UI
	if (m_OpenglView.isEmpty() && ui.action_IsoView1->isEnabled())
	{
		ui.action_IsoView1->setEnabled(false);
		ui.action_IsoView2->setEnabled(false);
		ui.action_IsoView3->setEnabled(false);
		ui.action_IsoView4->setEnabled(false);
		ui.action_FrontView->setEnabled(false);
		ui.action_RightView->setEnabled(false);
		ui.action_TopView->setEnabled(false);
		ui.action_Reframe->setEnabled(false);
		ui.actionSelectAll->setEnabled(false);
		ui.action_ZoomIn->setEnabled(false);
		ui.action_ZoomOut->setEnabled(false);
		ui.action_RenderPoints->setEnabled(false);
		ui.action_RenderWireframe->setEnabled(false);
		ui.action_RenderShading->setEnabled(false);
		ui.action_SnapShot->setEnabled(false);
		ui.actionMultiShots->setEnabled(false);

		ui.actionShow_all->setEnabled(false);
		ui.actionSwap_visible_space->setEnabled(false);
	}
	else if (!m_OpenglView.isEmpty() && !ui.action_IsoView1->isEnabled())
	{
		ui.action_IsoView1->setEnabled(true);
		ui.action_IsoView2->setEnabled(true);
		ui.action_IsoView3->setEnabled(true);
		ui.action_IsoView4->setEnabled(true);
		ui.action_FrontView->setEnabled(true);
		ui.action_RightView->setEnabled(true);
		ui.action_TopView->setEnabled(true);
		ui.action_Reframe->setEnabled(true);
		ui.actionSelectAll->setEnabled(true);
		ui.action_ZoomIn->setEnabled(true);
		ui.action_ZoomOut->setEnabled(true);
		ui.action_RenderPoints->setEnabled(true);
		ui.action_RenderWireframe->setEnabled(true);
		ui.action_RenderShading->setEnabled(true);
		ui.action_SnapShot->setEnabled(true);
		ui.actionMultiShots->setEnabled(true);

		ui.actionShow_all->setEnabled(true);
		ui.actionSwap_visible_space->setEnabled(true);

	}
}
// Display a message in status bar
void glc_player::displayMessageInStatusBar(QString message)
{
	ui.statusbar->showMessage(message);
}

// Remove unload item
void glc_player::removeUnloadFileItem()
{
	// Remove unload file entry from the hash
	FileEntryHash::iterator i= m_FileEntryHash.begin();
	while (i != m_FileEntryHash.end())
	{
		if (i.value().isReadyToLoad())
		{
			m_modelName.remove(i.value().getFileName());
			i= m_FileEntryHash.erase(i);
		}
		else ++i;
	}

	if (m_pAlbumManagerView->numberOfUnloadedModels() == 0)
	{
		// Update Continu list loading flag
		m_ContinuListLoading= true;
		m_MakeFirstFileCurrent= true;
	}
	// Test the integrity between view and model
	Q_ASSERT(m_pAlbumManagerView->numberOfUnloadedModels() == 0);

}
// Remove item on error
void glc_player::removeItemOnError()
{
	// Remove on error file entry from the hash
	FileEntryHash::iterator i= m_FileEntryHash.begin();
	while (i != m_FileEntryHash.end())
	{
		if (i.value().errorOccurWhileLoading())
		{
			m_modelName.remove(i.value().getFileName());
			i= m_FileEntryHash.erase(i);
		}
		else ++i;
	}

	if (m_pAlbumManagerView->numberOfUnloadedModels() == 0)
	{
		// Update Continu list loading flag
		m_ContinuListLoading= true;
		m_MakeFirstFileCurrent= true;
	}

	// Test the integrity between view and model
	Q_ASSERT(m_pAlbumManagerView->numberOfErrorModels() == 0);

}

// Start Loading
void glc_player::startLoadingButton()
{
	m_ContinuListLoading= true;
	// Update UI
	ui.action_NewAlbum->setEnabled(false);
	ui.action_OpenAlbum->setEnabled(false);
	ui.menuRecent_Album->setEnabled(false);
	ui.action_SaveAlbumAs->setEnabled(false);
	ui.action_SaveAlbum->setEnabled(false);
	ui.actionExport_To_Folder->setEnabled(false);
	ui.actionExport_to_web->setEnabled(false);

	ui.action_Property->setEnabled(false);
	m_pSelectionProperty->editPropertySetEnabled(false);

	startLoading();
}

// Stop Loading
void glc_player::stopLoadingButton()
{
	m_ContinuListLoading= false;
	GLC_uint modelId= m_OpenFileThread.getModelId();
	QString loadingFileItemName(QFileInfo(m_FileEntryHash.value(modelId).getFileName()).fileName());
	QString msg(QString(tr("Loading will be interrupt after : ")) + loadingFileItemName);
	QMessageBox::information(this, tr("Loading interruption"), msg);
}

// Take a snapshoot
void glc_player::takeSnapShot()
{

	if (NULL == m_pScreenshotDialog)
	{
		m_pScreenshotDialog= new ScreenshotDialog(&m_OpenglView, this);
	}

	QString currentEntryName= QFileInfo(m_FileEntryHash.value(m_pAlbumManagerView->currentModelId()).getFileName()).baseName();
	m_pScreenshotDialog->initScreenshotDialog(currentEntryName);

	m_pScreenshotDialog->exec();

}

// Take multi snapshot
void glc_player::takeMultiShots()
{

	if (NULL == m_pMultiScreenshotsDialog)
	{
		m_pMultiScreenshotsDialog= new MultiScreenshotsDialog(&m_OpenglView, this);
	}

	m_pMultiScreenshotsDialog->initScreenshotDialog(m_FileEntryHash.value(m_pAlbumManagerView->currentModelId()));

	m_pMultiScreenshotsDialog->exec();


}

// Delete the specified item
void glc_player::deleteItem(const GLC_uint modelId)
{
	// Take the entry from hash table
	m_modelName.remove(m_FileEntryHash.value(modelId).getFileName());
	m_FileEntryHash.remove(modelId);

	if (m_pAlbumManagerView->numberOfUnloadedModels() == 0)
	{
		// Update Continu list loading flag
		m_ContinuListLoading= true;
		m_MakeFirstFileCurrent= true;
	}
}

// Album Management visibility toggle
void glc_player::albumManagementVisibilityToggle()
{
	ui.albumManagementWindow->blockSignals(true);
	if (ui.action_AlbumManagement->isChecked())
	{
		ui.albumManagementWindow->show();
	}
	else
	{
		ui.albumManagementWindow->hide();
	}
	ui.albumManagementWindow->blockSignals(false);
}

// Instance Property visibility toggle
void glc_player::instancePropertyVisibilityToggle()
{
	m_pInstanceProperty->blockSignals(true);
	if (m_pInstancePropertyVis->isChecked())
	{
		m_pInstanceProperty->show();
	}
	else
	{
		m_pInstanceProperty->hide();
	}
	m_pInstanceProperty->blockSignals(false);
}

// The album management visibility change
void glc_player::albumManagementVisibilityChanged(bool isVisible)
{
	ui.action_AlbumManagement->setChecked(isVisible);
}

// The instance property visibility change
void glc_player::instancePropertyVisibilityChanged(bool isVisible)
{
	m_pInstancePropertyVis->setChecked(isVisible);
}

// Camera property toggle
void glc_player::cameraPropertyVisibilityToggle()
{
	ui.cameraProperties->blockSignals(true);
	if (ui.action_CameraProperty->isChecked())
	{
		ui.cameraProperties->show();
	}
	else
	{
		ui.cameraProperties->hide();
	}
	ui.cameraProperties->blockSignals(false);

}
// The camera property visibility change
void glc_player::cameraPropertyVisibilityChanged(bool isVisible)
{
	ui.action_CameraProperty->setChecked(isVisible);
}
// Selection property toggle
void glc_player::selectionPropertyVisibilityToggle()
{
	ui.selectionDockWidget->blockSignals(true);
	if (ui.action_SelectionProperty->isChecked())
	{
		ui.selectionDockWidget->show();
	}
	else
	{
		ui.selectionDockWidget->hide();
	}
	ui.selectionDockWidget->blockSignals(false);

}
// The Selection property visibility change
void glc_player::selectionPropertyVisibilityChanged(bool isVisible)
{
	ui.action_SelectionProperty->setChecked(isVisible);
}

// Go to Point Rendering mode
void glc_player::pointsRenderingMode()
{
	if (!ui.action_RenderPoints->isChecked())
	{
		ui.action_RenderPoints->setChecked(true);
	}
	else
	{
		ui.action_RenderWireframe->setChecked(false);
		ui.action_RenderShading->setChecked(false);
		m_OpenglView.setMode(GL_POINT);
		m_OpenglView.updateGL();
	}
}
// Go to wireframe Rendering mode
void glc_player::wireframeRenderingMode()
{
	if (!ui.action_RenderWireframe->isChecked())
	{
		ui.action_RenderWireframe->setChecked(true);
	}
	else
	{
		ui.action_RenderPoints->setChecked(false);
		ui.action_RenderShading->setChecked(false);
		m_OpenglView.setMode(GL_LINE);
		m_OpenglView.updateGL();
	}
}
// Go to shading Rendering Mode
void glc_player::shadingRenderingMode()
{
	if (!ui.action_RenderShading->isChecked())
	{
		ui.action_RenderShading->setChecked(true);
	}
	else
	{
		ui.action_RenderWireframe->setChecked(false);
		ui.action_RenderPoints->setChecked(false);
		m_OpenglView.setMode(GL_FILL);
		m_OpenglView.updateGL();
	}
}
// Display about GLC_Player dialog box
void glc_player::aboutPlayer()
{
	m_OpenglView.doneCurrent();
	AboutPlayer about(this);
	about.exec();
}

// Go to fullScreen Mode
void glc_player::fullScreen()
{
	// Toggle fullScreen Mode
	if (Qt::WindowFullScreen != windowState())
	{
		//setUnifiedTitleAndToolBarOnMac(false);
		showFullScreen();
	}
	else
	{
		showNormal();
		//setUnifiedTitleAndToolBarOnMac(true);
		//updateGeometry();
	}
}
// Display GLC_Player help page
void glc_player::help()
{
	QSettings settings;
	if ("fr" == settings.value("currentLanguage").toString())
	{
		QDesktopServices::openUrl(QUrl("http://glc-player.net/fr/doc.php"));
	}
	else
	{
		QDesktopServices::openUrl(QUrl("http://glc-player.net/doc.php"));
	}
}

// Update the view
void glc_player::updateView()
{
	m_OpenglView.setDistMinAndMax();
	m_OpenglView.updateGL();
}

// Get information about failed loading
void glc_player::getErrorInformation()
{/*
	if (NULL != ui.loadingList->currentItem())
	{
		QString fileName(ui.loadingList->currentItem()->data(Qt::UserRole).toString());
		// Take the entry from hash table
		QString message(m_FileEntryHash.value(fileName).getError());
		QMessageBox::information(this, QCoreApplication::applicationName(), message);
	}*/
}

// Edit the light properties
void glc_player::editLightDialog()
{
	if (m_pEditLightDialog == NULL)
	{
		m_pEditLightDialog= new EditLightDialog(m_OpenglView.getLight(), this);
		connect(m_pEditLightDialog, SIGNAL(lightUpdated()), &m_OpenglView, SLOT(updateGL()));
	}
	m_pEditLightDialog->setWindowFlags(Qt::Tool);
	m_pEditLightDialog->show();
	m_pEditLightDialog->raise();
	m_pEditLightDialog->activateWindow();
}
// Two sided lightning
void glc_player::twoSidedLightning()
{
		m_OpenglView.getLight()->setTwoSided(ui.actionTwo_sided_Lightning->isChecked());
		m_OpenglView.updateGL();
}

// Compute the icon of a newly loaded file in back buffer
void glc_player::computeIconInBackBuffer(int itemRow, bool forceCurrent)
{
	QImage snapShoot(takeSnapShoot(itemRow, 1.0, forceCurrent));

	// Process event
	if (V_NORMAL != m_OpenglView.viewState())
	{
		QCoreApplication::processEvents();
	}

	m_pAlbumManagerView->setSnapShoot(itemRow, snapShoot);
}

// Return to normal mode
void glc_player::returnToNormalMode()
{

	// Enabled dockWindow visibility change
	ui.action_AlbumManagement->setEnabled(true);
	ui.action_CameraProperty->setEnabled(true);
	ui.action_SelectionProperty->setEnabled(true);
	// Menu edit
	ui.actionSelectAll->setEnabled(true);
	ui.actionUnselectAll->setEnabled(true);
	ui.action_Property->setEnabled(true);
	m_pSelectionProperty->editPropertySetEnabled(true);
	//Menu View
	ui.actionReframeOnSelection->setEnabled(true);
	ui.actionShow_Hide->setEnabled(true);
	ui.actionHide_unselected->setEnabled(true);
	ui.actionShow_all->setEnabled(true);
	ui.actionSwap_visible_space->setEnabled(true);
	//Menu File
	ui.action_NewAlbum->setEnabled(true);
	ui.action_OpenAlbum->setEnabled(true);
	ui.menuRecent_Album->setEnabled(true);
	ui.action_SaveAlbum->setEnabled(true);
	ui.action_SaveAlbumAs->setEnabled(true);
	ui.actionExport_To_Folder->setEnabled(true);
	ui.actionExport_to_web->setEnabled(true);
	ui.action_Open->setEnabled(true);
	ui.menuRecent_Models->setEnabled(true);

	m_pListOfMaterial= NULL;

	// Disconnect action
	disconnect(m_pInstancePropertyVis, SIGNAL(triggered()), this, SLOT(instancePropertyVisibilityToggle()));
	disconnect(m_pInstanceProperty, SIGNAL(visibilityChanged(bool)), this, SLOT(instancePropertyVisibilityChanged(bool)));
	disconnect(m_pInstanceProperty, SIGNAL(doneSignal()), this, SLOT(returnToNormalMode()));
	disconnect(m_pInstanceProperty, SIGNAL(updateView()), &m_OpenglView, SLOT(updateGL()));
	disconnect(m_pInstanceProperty, SIGNAL(viewSubMaterialList()), this, SLOT(viewListOfMaterial()));
	// Hide Instance property dock window
	m_pInstanceProperty->hide();

	// Remove instanceProperty visibility action
	ui.menu_Window->removeAction(m_pInstancePropertyVis);

	// Remove instance edit dock widget
	removeDockWidget(m_pInstanceProperty);

	if (NULL != m_pMaterialProperty)
	{
		removeDockWidget(m_pMaterialProperty);
		delete m_pMaterialProperty;
		m_pMaterialProperty= NULL;
	}

	// Restore dock windows
	UserInterfaceSate::SetGlobalState(NORMAL_STATE);
	ui.albumManagementWindow->setVisible(UserInterfaceSate::albumManagerIsVisible());
	ui.cameraProperties->setVisible(UserInterfaceSate::cameraPropertyIsVisible());
	ui.selectionDockWidget->setVisible(UserInterfaceSate::selectionPropertyIsVisible());

	// UnBlock selection
	m_OpenglView.blockSelection(false);
	// restore viewing state
	FileEntryHash::iterator iEntry= m_FileEntryHash.find(m_pAlbumManagerView->currentModelId());
	iEntry.value().setCameraAndAngle(m_OpenglView.getCamera(), m_OpenglView.getViewAngle());
	iEntry.value().setPolygonMode(m_OpenglView.getMode());

	setCurrentFileItem(m_pAlbumManagerView->currentModelId());
}

// Choose shader
void glc_player::chooseShader()
{
	if (NULL == m_pChooseShaderDialog)
	{
		m_pChooseShaderDialog= new ChooseShaderDialog(&m_OpenglView, this);
	}
	m_pChooseShaderDialog->UpdateThumbnailsList();

	if (m_pChooseShaderDialog->exec() == QDialog::Accepted)
	{
		m_OpenglView.setGlobalShaderId(m_pChooseShaderDialog->shaderId(), m_pChooseShaderDialog->shaderName());
	}

}

// reload model
void glc_player::reloadModel(GLC_uint modelId)
{

	if (m_FileEntryHash.contains(modelId))
	{
		unselectAll();
		if (m_pModelManagerView->isCurrentFileEntry(m_FileEntryHash[modelId]))
		{
			m_pModelManagerView->clear();
		}
		m_FileEntryHash[modelId].reload();
		m_OpenglView.clear();
		m_OpenglView.updateGL();
		startLoading();
	}
}

// Update current file Entry Material
void glc_player::updateCurrentEntryMaterial(GLC_Material* pMaterial)
{
	FileEntryHash::iterator iEntry= m_FileEntryHash.find(m_pAlbumManagerView->currentModelId());
	Q_ASSERT(iEntry != m_FileEntryHash.constEnd());
	iEntry.value().addModifiedMaterial(pMaterial);
}

//! The Opengl as been initialised
void glc_player::glInitialed()
{
	// Test if shader usage
	// Test for vbo Usage
	if (-1 == m_UseShader)
	{
		// First time
		GLC_State::setGlslUsage(GLC_State::vendorIsNvidia());
		m_UseShader = static_cast<int>(GLC_State::glslUsed());
	}
	else
	{
		GLC_State::setGlslUsage((1 == m_UseShader));
	}
	// Initialized shader list
	m_OpenglView.initShaderList();

	if (not GLC_State::glslUsed())
	{
		ui.actionSet_Shader->setEnabled(false);
	}
	else
	{
		GLC_State::setSelectionShaderUsage(m_UseSelectionShader);
	}
	// Test for vbo Usage
	if (-1 == m_UseVbo)
	{
		// First time
		GLC_State::setVboUsage(GLC_State::vendorIsNvidia());
		m_UseVbo = static_cast<int>(GLC_State::vboUsed());
	}
	else
	{
		GLC_State::setVboUsage((1 == m_UseVbo));
	}

}

// Change Current mover to track ball mover
void glc_player::changeCurrentMoverToTrackBall()
{
	ui.action_Rotate->setIcon(QIcon(":images/Orbit.png"));
	m_OpenglView.changeCurrentMoverToTrackBall();
}
// Change Current mover to turn table mover
void glc_player::changeCurrentMoverToTurnTable()
{
	ui.action_Rotate->setIcon(QIcon(":images/TurnTable.png"));
	m_OpenglView.changeCurrentMoverToTurnTable();
}

//////////////////////////////////////////////////////////////////////
// Protected event Functions
//////////////////////////////////////////////////////////////////////

// Key press event
void glc_player::keyPressEvent (QKeyEvent *pKeyEvent)
{
	const bool isInNormalSate= (UserInterfaceSate::globalState() == NORMAL_STATE);
	if ((pKeyEvent->key() == Qt::Key_Down) and isInNormalSate)
	{
		m_pAlbumManagerView->nextItem();
	}
	else if ((pKeyEvent->key() == Qt::Key_Up) and isInNormalSate)
	{
		m_pAlbumManagerView->previousItem();
	}
	else
	{
		QWidget::keyPressEvent(pKeyEvent);
	}
}

//////////////////////////////////////////////////////////////////////
// Private services Functions
//////////////////////////////////////////////////////////////////////

// Create RecentFileActionArray
void glc_player::createRecentFileActionsArray()
{
	for (int i= 0; i < MaxRecentFiles; ++i)
	{
		m_pRecentFileActionsArray[i]= new QAction(this);
		m_pRecentFileActionsArray[i]->setVisible(false);
		ui.menuRecent_Models->addAction(m_pRecentFileActionsArray[i]);
		connect(m_pRecentFileActionsArray[i], SIGNAL(triggered()), this, SLOT(openRecentFile()));
	}
}

// Create RecentFileActionArray
void glc_player::createRecentAlbumActionsArray()
{
	for (int i= 0; i < MaxRecentFiles; ++i)
	{
		m_pRecentAlbumActionsArray[i]= new QAction(this);
		m_pRecentAlbumActionsArray[i]->setVisible(false);
		ui.menuRecent_Album->addAction(m_pRecentAlbumActionsArray[i]);
		connect(m_pRecentAlbumActionsArray[i], SIGNAL(triggered()), this, SLOT(openRecentAlbum()));
	}
}

// Update Recent Files
void glc_player::updateRecentsFiles()
{
	QMutableStringListIterator i(m_RecentFilesList);
	while (i.hasNext())
	{
		if (!QFile::exists(i.next()))
		{
			i.remove();
		}
	}
	for (int j= 0; j < MaxRecentFiles;  ++j)
	{
		if (j < m_RecentFilesList.count())
		{
			QString text= tr("&%1 %2").arg(j + 1).arg(QFileInfo(m_RecentFilesList[j]).fileName());
			m_pRecentFileActionsArray[j]->setText(text);
			m_pRecentFileActionsArray[j]->setData(m_RecentFilesList[j]);
			m_pRecentFileActionsArray[j]->setVisible(true);
		}
		else
		{
			m_pRecentFileActionsArray[j]->setVisible(false);
		}
	}
}

// Update Recent Files
void glc_player::updateRecentsAlbums()
{
	QMutableStringListIterator i(m_RecentAlbumsList);
	while (i.hasNext())
	{
		if (!QFile::exists(i.next()))
		{
			i.remove();
		}
	}
	for (int j= 0; j < MaxRecentFiles;  ++j)
	{
		if (j < m_RecentAlbumsList.count())
		{
			QString text= tr("&%1 %2").arg(j + 1).arg(QFileInfo(m_RecentAlbumsList[j]).fileName());
			m_pRecentAlbumActionsArray[j]->setText(text);
			m_pRecentAlbumActionsArray[j]->setData(m_RecentAlbumsList[j]);
			m_pRecentAlbumActionsArray[j]->setVisible(true);
		}
		else
		{
			m_pRecentAlbumActionsArray[j]->setVisible(false);
		}
	}
}

// Open the file
void glc_player::openModel(const QString& fileName, const GLC_uint id)
{
	QFile file(fileName);
	m_FileLoadingInProgress= true;
	m_CurrentPath= QFileInfo(fileName).absolutePath();
	m_pProgressBar->setValue(0);
	m_pProgressBar->show();
	m_OpenFileThread.setOpenFile(id, &file);
	m_OpenFileThread.start(QThread::LowPriority);
	ui.action_NewAlbum->setEnabled(false);
	ui.action_OpenAlbum->setEnabled(false);
	ui.menuRecent_Album->setEnabled(false);
	ui.action_SaveAlbumAs->setEnabled(false);
	ui.action_SaveAlbum->setEnabled(false);
	ui.actionExport_To_Folder->setEnabled(false);
	ui.actionExport_to_web->setEnabled(false);

	ui.action_Property->setEnabled(false);
	m_pSelectionProperty->editPropertySetEnabled(false);

}

// Add current file to recent file
void glc_player::addToRecentFiles(const QString& fileName)
{
	m_CurrentFileName= fileName;
	if (!m_CurrentFileName.isEmpty())
	{
		m_RecentFilesList.removeAll(m_CurrentFileName);
		m_RecentFilesList.prepend(m_CurrentFileName);
		if (m_RecentFilesList.size() > MaxRecentFiles)
		{
			while (m_RecentFilesList.size() > MaxRecentFiles)
			{
				m_RecentFilesList.removeLast();
			}
		}
		updateRecentsFiles();
	}
}

// Add current Album to recent Albums
void glc_player::addToRecentAlbums(const QString& fileName)
{
	m_CurrentAlbumName= fileName;
	if (!m_CurrentAlbumName.isEmpty())
	{
		m_RecentAlbumsList.removeAll(m_CurrentAlbumName);
		m_RecentAlbumsList.prepend(m_CurrentAlbumName);
		if (m_RecentAlbumsList.size() > MaxRecentFiles)
		{
			while (m_RecentAlbumsList.size() > MaxRecentFiles)
			{
				m_RecentAlbumsList.removeLast();
			}
		}
		updateRecentsAlbums();
	}
}

// drag enter event
void glc_player::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasFormat("text/uri-list"))
	{
		event->acceptProposedAction();
	}
}

// drop event
void glc_player::dropEvent(QDropEvent* event)
{
	QList<QUrl> urls= event->mimeData()->urls();
	if (urls.isEmpty()) return;
	// Stransform the url List to string list
	QStringList localFiles;
	for (int i= 0; i < urls.size(); ++i)
	{
		localFiles << urls[i].toLocalFile();
	}
	const bool fileIsAlbum= (QFileInfo(localFiles[0]).suffix().toLower() == "album");
	if (fileIsAlbum and ui.action_OpenAlbum->isEnabled())
	{
		openAlbum(localFiles[0]);
	}
	else if (not fileIsAlbum)
	{
		addItems(localFiles);
		startLoading();
	}
}

// Load successful
void glc_player::loadSuccessful(QString fileName)
{
	addToRecentFiles(fileName);
	m_FileLoadingInProgress= false;
}

// Add items
void glc_player::addItems(QStringList nameList)
{
	QString currentString;
	for (int i= 0; i < nameList.size(); ++i)
	{
		// If the file is already in the list the file is skipped
		if (!m_modelName.contains(nameList[i]) &&
				((QFileInfo(nameList[i]).suffix().toLower() == "obj") || (QFileInfo(nameList[i]).suffix().toLower() == "stl")
						|| (QFileInfo(nameList[i]).suffix().toLower() == "off") || (QFileInfo(nameList[i]).suffix().toLower() == "3ds")
						|| (QFileInfo(nameList[i]).suffix().toLower() == "3dxml") || (QFileInfo(nameList[i]).suffix().toLower() == "dae")))
		{
			// add the File Entry
			m_modelName.insert(nameList[i]);
			FileEntry newEntry(nameList[i]);

			m_FileEntryHash.insert(newEntry.id(), newEntry);
			// Add the model to the view
			m_pAlbumManagerView->addModel(newEntry.id());
		}
	}
}
// Start Loading FileItem
void glc_player::startLoading()
{
	// Test if the application is ready to load a file
	if (m_FileLoadingInProgress) return;

	if (!m_ContinuListLoading)
	{
		// Update UI
		ui.action_NewAlbum->setEnabled(true);
		ui.action_OpenAlbum->setEnabled(true);
		ui.menuRecent_Album->setEnabled(true);
		ui.action_SaveAlbumAs->setEnabled(true);
		ui.action_SaveAlbum->setEnabled(true);
		ui.actionExport_To_Folder->setEnabled(true);
		ui.actionExport_to_web->setEnabled(true);
		m_ListLoadingInProgress= false;
		// enable the Instance property action if nesessary
		if ((NULL != m_pCurrentSelections) and (1 == m_pCurrentSelections->size()))
		{
			ui.action_Property->setEnabled(true);
			m_pSelectionProperty->editPropertySetEnabled(true);
		}
		return;
	}

	GLC_uint modelId= m_pAlbumManagerView->firstUnloadModelId();
	// Test if an unloaded model have been find
	if(0 == modelId)
	{
		m_ListLoadingInProgress= false;
		ui.action_NewAlbum->setEnabled(true);
		ui.action_OpenAlbum->setEnabled(true);
		ui.menuRecent_Album->setEnabled(true);
		ui.action_SaveAlbumAs->setEnabled(true);
		ui.action_SaveAlbum->setEnabled(true);
		ui.actionExport_To_Folder->setEnabled(true);
		ui.actionExport_to_web->setEnabled(true);

		// enable the Instance property action if nesessary
		if ((NULL != m_pCurrentSelections) and (1 == m_pCurrentSelections->size()))
		{
			ui.action_Property->setEnabled(true);
			m_pSelectionProperty->editPropertySetEnabled(true);
		}

	}
	else
	{
		FileEntryHash::iterator iEntry;
		iEntry= m_FileEntryHash.find(modelId);
		Q_ASSERT(iEntry != m_FileEntryHash.constEnd());
		iEntry.value().setLoadingStatus(true);
		openModel(iEntry.value().getFileName(), iEntry.value().id());
		m_ListLoadingInProgress= true;
	}
}

// Set the current File Item
void glc_player::setCurrentFileItem(const GLC_uint modelId)
{
	FileEntryHash::iterator iEntry= m_FileEntryHash.find(modelId);
	const QString fileName= m_FileEntryHash.value(modelId).getFileName();

	Q_ASSERT(iEntry != m_FileEntryHash.constEnd());

	m_pLeftSideDock->currentModelChanged();

	if (iEntry.value().isLoaded())
	{
		// Set polygon mode of the entry
		iEntry.value().setPolygonMode(m_OpenglView.getMode());
		GLC_World world(iEntry.value().getWorld());
		// Check the visible state of the view
		if (m_OpenglView.getVisibleState() != world.collection()->showState())
		{
			if(m_OpenglView.getVisibleState() == true) m_OpenglView.setToInVisibleState();
			else m_OpenglView.setToVisibleState();
		}
		m_OpenglView.clear();
		m_OpenglView.add(world);
		if (iEntry.value().cameraIsSet())
		{
			m_OpenglView.setCameraAndAngle(iEntry.value().getCamera(), iEntry.value().getViewAngle());
			m_OpenglView.updateGL();
		}
		else
		{
			GLC_Camera defaultCam;
			defaultCam.setDefaultUpVector(world.upVector());
			defaultCam.setIsoView();
			m_OpenglView.setCameraAndAngle(defaultCam, iEntry.value().getViewAngle());

			m_OpenglView.reframe(GLC_BoundingBox(), false);
		}
		setWindowTitle(QString(QCoreApplication::applicationName() +" [") + QFileInfo(fileName).fileName() + QString("]"));
		ui.statusbar->showMessage(fileName);
		//const int instances= iEntry.value().getNumberOfInstances();
		const int faces= iEntry.value().getNumberOfFaces();
		const int numberOfBody= iEntry.value().getNumberOfMeshes();
		m_pAlbumManagerView->updateCurrentModelInfo(numberOfBody, faces);

		// Manage selection
		if (!world.collection()->selection()->empty())
		{
			m_pCurrentSelections= world.collection()->selection();
			m_pSelectionProperty->setSelection(m_pCurrentSelections);
			ui.action_Property->setEnabled(true);
			m_pSelectionProperty->editPropertySetEnabled(true);
			ui.actionUnselectAll->setEnabled(true);
			if (GLC_State::glslUsed()) ui.actionAssign_Shader->setEnabled(true);
			ui.actionShow_Hide->setEnabled(true);
			ui.actionHide_unselected->setEnabled(true);
			ui.actionReframeOnSelection->setEnabled(true);

		}
		m_pAlbumManagerView->setEnabledStatus(true);
	}
	else // Entry not found or not loaded => clear the view
	{
		if (iEntry.value().isLoading())
		{
			m_pAlbumManagerView->setEnabledStatus(false);
		}
		else
		{
			m_pAlbumManagerView->setEnabledStatus(true);
		}
		ui.statusbar->showMessage(fileName);
		m_pAlbumManagerView->clearCurrentModelInfo();
		if (m_OpenglView.getVisibleState() == false)
		{
			m_OpenglView.setToVisibleState();
		}
		m_OpenglView.clear();
		m_OpenglView.updateGL();
	}
}
// Take a snapshot of the specifies item with the specifie ratio
// Return Snapshot as QImage
QImage glc_player::takeSnapShoot(int itemRow, double ratio, bool forceCurrent)
{
	GLC_Camera savCam;
	double savAngle;
	bool itemIsNotCurrent= true;
	const int currentRow= m_pAlbumManagerView->currentRow();
	const bool viewIsEmpty= m_OpenglView.isEmpty();
	const bool rowToComputeIsCurrent= itemRow == currentRow;
	m_OpenglView.setSnapShootMode(true);
	// Test if the current item is already in the current view and loaded
	if (not forceCurrent and ((not rowToComputeIsCurrent) or (rowToComputeIsCurrent and viewIsEmpty)))
	{
		/* The Current model is not in current view
		 * Or
		 * The Current model is in current view but not already in the view
		 */
		GLC_uint modelId= m_pAlbumManagerView->modelId(itemRow);

		FileEntryHash::iterator iEntry= m_FileEntryHash.find(modelId);
		// Test if the model is loaded
		if (iEntry.value().isLoaded())
		{
			// The model is Loaded
			savCam= m_OpenglView.getCamera();
			savAngle= m_OpenglView.getViewAngle();
			// Set polygon mode of the entry
			iEntry.value().setPolygonMode(m_OpenglView.getMode());
			GLC_World world(iEntry.value().getWorld());
			m_OpenglView.setAutoBufferSwap(false);
			if (m_OpenglView.getVisibleState() != world.collection()->showState())
			{
				if(m_OpenglView.getVisibleState() == true) m_OpenglView.setToInVisibleState();
				else m_OpenglView.setToVisibleState();
			}

			m_OpenglView.clear();
			m_OpenglView.add(world);
			// Test if the current model's camera is already set
			if (iEntry.value().cameraIsSet())
			{
				// The model's camera is set
				m_OpenglView.setCameraAndAngle(iEntry.value().getCamera(), iEntry.value().getViewAngle());
				// Change the view Aspect ratio
				m_OpenglView.getViewport()->forceAspectRatio(ratio);
				m_OpenglView.updateGL();
			}
			else
			{
				// The model's camera is not set
				GLC_BoundingBox emptyBoundingBox;
				m_OpenglView.initIsoView();
				m_OpenglView.reframe(emptyBoundingBox, false); // an updateGL is done while reframing
				// Change the view Aspect ratio
				m_OpenglView.getViewport()->forceAspectRatio(ratio);
				m_OpenglView.updateGL();
				m_OpenglView.getViewport()->updateProjectionMat();

				iEntry.value().setCameraAndAngle(m_OpenglView.getCamera(), m_OpenglView.getViewAngle());
			}
		}

	}
	else
	{
		/* The Current model is in current view
		 * And
		 * The Current model is already in the view
		 */
		m_OpenglView.setAutoBufferSwap(false);
		// Change the view Aspect ratio
		m_OpenglView.getViewport()->forceAspectRatio(ratio);
		m_OpenglView.updateGL();
		m_OpenglView.getViewport()->updateProjectionMat();
		itemIsNotCurrent= false;
	}

	// Make the screenShoot
	QImage snapShoot= m_OpenglView.grabFrameBuffer();

	// Test if this is the current model which has been grab
	if (itemIsNotCurrent)
	{
		/* This is not the current model which has been grab
		 * Load The current model in the view
		 */
		GLC_uint modelId= m_pAlbumManagerView->currentModelId();

		FileEntryHash::iterator iEntry= m_FileEntryHash.find(modelId);
		// Test if the current model has been loaded
		if (iEntry.value().isLoaded())
		{
			// The current model has been loaded
			// Set polygon mode of the entry
			iEntry.value().setPolygonMode(m_OpenglView.getMode());
			GLC_World world(iEntry.value().getWorld());
			if (m_OpenglView.getVisibleState() != world.collection()->showState())
			{
				if(m_OpenglView.getVisibleState() == true) m_OpenglView.setToInVisibleState();
				else m_OpenglView.setToVisibleState();
			}
			m_OpenglView.clear();
			m_OpenglView.add(world);
			m_OpenglView.setCameraAndAngle(savCam, savAngle);
		}
		else
		{
			// The current model has not been loaded
			m_OpenglView.clear();
		}
	}
	m_OpenglView.setAutoBufferSwap(true);
	m_OpenglView.setSnapShootMode(false);
	m_OpenglView.updateGL();

	return snapShoot;
}

// Read Application Settings
void glc_player::readSettings()
{
	QSettings settings;
	// Recent Model list
	m_RecentFilesList= settings.value("recentFiles").toStringList();
	// Recent Album list
	m_RecentAlbumsList= settings.value("recentAlbums").toStringList();
	// Quit confirmation
	m_QuitConfirmation= settings.value("quitConfirmation", true).toBool();

	// MainWindow geometry
	settings.beginGroup("MainWindow");
	resize(settings.value("size", QSize(1200, 730)).toSize());
	move(settings.value("pos", QPoint(50, 50)).toPoint());
	m_dislayInfoPanel= settings.value("displayInfoPanel", true).toBool();
	settings.endGroup();


	// Album Manager
	settings.beginGroup("AlbumManager");
	// Album manager visibility
	bool visibility= settings.value("visibility", true).toBool();
	ui.action_AlbumManagement->setChecked(visibility);
	ui.albumManagementWindow->setVisible(visibility);
	// IconSize
	m_pAlbumManagerView->setIconSize(settings.value("thumbnailSize", QSize(40, 40)).toSize());
	// Thumbnails display
	m_pAlbumManagerView->setThumbnailsDisplay(settings.value("displayThumbnail", true).toBool());
	settings.endGroup();

	// Camera properties
	settings.beginGroup("CameraProperties");
	// Camera properties visibility
	visibility= settings.value("visibility", true).toBool();
	ui.action_CameraProperty->setChecked(visibility);
	ui.cameraProperties->setVisible(visibility);
	settings.endGroup();

	// Selection properties
	settings.beginGroup("SelectionProperties");
	// Camera properties visibility
	visibility= settings.value("visibility", true).toBool();
	ui.action_SelectionProperty->setChecked(visibility);
	ui.selectionDockWidget->setVisible(visibility);
	settings.endGroup();

	// OpenGL setting
	settings.beginGroup("OpenGlSetting");
	// Vbo Usage
	m_UseVbo= settings.value("usevbo", -1).toInt();
	// Shader Usage
	m_UseShader= settings.value("useshader", -1).toInt();
	// Selection Shader
	m_UseSelectionShader= settings.value("useselectionshader", true).toBool();
	settings.endGroup();

	// Performance setting
	settings.beginGroup("PerformanceSetting");
	// Shader Usage
	m_DefaultLodValue= settings.value("defaultLODValue", 10).toInt();
	// Selection Shader
	m_UsePixelCulling= settings.value("usepixelCulling", true).toBool();
	settings.endGroup();


}

// Write Application Settings
void glc_player::writeSettings()
{
	QSettings settings;

	// Recent Model list
	settings.setValue("recentFiles", m_RecentFilesList);
	// Recent Albums list
	settings.setValue("recentAlbums", m_RecentAlbumsList);
	// Quit Confirmation
	settings.setValue("quitConfirmation", m_QuitConfirmation);

	// MainWindow geometry
	if (isFullScreen())
	{
		// Toggle fullScreen Mode
		setWindowState(windowState() ^ Qt::WindowFullScreen);
	}
	// MainWindow geometry
	settings.beginGroup("MainWindow");
	settings.setValue("size", size());
	settings.setValue("pos", pos());
	settings.setValue("displayInfoPanel", m_dislayInfoPanel);
	settings.endGroup();

	// Album Manager
	settings.beginGroup("AlbumManager");
	// Album manager visibility
	settings.setValue("visibility", ui.albumManagementWindow->isVisible());
	// IconSize
	settings.setValue("thumbnailSize", m_pAlbumManagerView->iconSize());
	// Thumbnails display
	settings.setValue("displayThumbnail", m_pAlbumManagerView->thumbnailsAreDisplay());
	settings.endGroup();

	// Camera properties
	settings.beginGroup("CameraProperties");
	// Camera properties visibility
	settings.setValue("visibility", ui.cameraProperties->isVisible());
	settings.endGroup();

	// Selection properties
	settings.beginGroup("SelectionProperties");
	// Camera properties visibility
	settings.setValue("visibility", ui.selectionDockWidget->isVisible());
	settings.endGroup();

	// OpenGL setting
	settings.beginGroup("OpenGlSetting");
	// Vbo Usage
	settings.setValue("usevbo", m_UseVbo);
	// Shader Usage
	settings.setValue("useshader", m_UseShader);
	// Selection Shader
	settings.setValue("useselectionshader", m_UseSelectionShader);
	settings.endGroup();

	// Performance setting
	settings.beginGroup("PerformanceSetting");
	// Shader Usage
	settings.setValue("defaultLODValue", m_DefaultLodValue);
	// Selection Shader
	settings.setValue("usepixelCulling", m_UsePixelCulling);
	settings.endGroup();

}

// Ask for open Album option
bool glc_player::getOpenAlbumOptionDlg()
{
	if (NULL == m_pOpenAlbumOption)
	{
		m_pOpenAlbumOption= new OpenAlbumOption(this);
	}
	return m_pOpenAlbumOption->exec();

}
// Apply album saving
void glc_player::applySavingAlbum(const QString& fileName)
{
	// Update the current FileEntry camera position
	FileEntryHash::iterator iEntry= m_FileEntryHash.find(m_pAlbumManagerView->currentModelId());
	if (iEntry.value().isLoaded() && (!(iEntry.value().getCamera() == m_OpenglView.getCamera())
						|| (iEntry.value().getPolygonMode() != m_OpenglView.getMode())
						|| iEntry.value().getViewAngle() != m_OpenglView.getViewAngle()))
	{
		iEntry.value().setCameraAndAngle(m_OpenglView.getCamera(), m_OpenglView.getViewAngle());
		iEntry.value().setPolygonMode(m_OpenglView.getMode());
	}

	// Save the album File
	QFile albumFile(fileName);
	AlbumFile albumFileWriter;
	albumFileWriter.saveAlbumFile(m_FileEntryHash.values(), &albumFile);
	m_CurrentAlbumPath= QFileInfo(fileName).absolutePath();
	m_CurrentAlbumName= fileName;
	addToRecentAlbums(fileName);
	QString message(QString(tr("Album (")) + fileName + QString(tr(") successfully saved")));
	ui.statusbar->showMessage(message);
	m_pAlbumManagerView->setAlbumName(QString("Album : ") + QFileInfo(fileName).baseName());

}

// Open specified album
void glc_player::openAlbum(const QString& fileName)
{
	// Open the album File
	// List of opened models
	QFile albumFile(fileName);
	if (!albumFile.exists())
	{
		qDebug() << "Album File doesn't exist" << fileName;
		return;
	}
	bool setFirstItemCurrent= true;
	// Test if there is some models in the current album
	if (m_FileEntryHash.size() > 0)
	{
		if (!getOpenAlbumOptionDlg()) return; // Do nothing on cancel
		if (m_pOpenAlbumOption->currentAlbumHaveToBeSaved())
		{
			// Save the current Album
			saveAlbum();
		}
		if (m_pOpenAlbumOption->currentAlbumHaveToBeReplaced())
		{
			newAlbum(false);
			m_CurrentAlbumPath= QFileInfo(fileName).absolutePath();
			m_CurrentAlbumName= fileName;
			m_pAlbumManagerView->setAlbumName(QString("Album : ") + QFileInfo(fileName).baseName());
		}
		else
		{
			setFirstItemCurrent= false;
			// Update Continu list loading flag
			m_ContinuListLoading= true;
			// Update Album Manager button
			m_pAlbumManagerView->resetButton();
		}
	}
	else
	{
		m_CurrentAlbumPath= QFileInfo(fileName).absolutePath();
		m_CurrentAlbumName= fileName;
		m_pAlbumManagerView->setAlbumName(QString("Album : ") + QFileInfo(fileName).baseName());
	}
	AlbumFile albumFileReader;
	QList<FileEntry> fileEntries;
	try
	{
		fileEntries= albumFileReader.loadAlbumFile(&albumFile);
	}
	catch (GLC_Exception &e)
	{
		QString message(tr("Wrong album file format"));
		QMessageBox::critical(this, QCoreApplication::applicationName(), message);
		return;
	}
	const int max= fileEntries.size();
	for (int i= 0; i < max; ++i)
	{
		// If the Entry is already in the list the Entry is skipped
		QString fileName(fileEntries[i].getFileName());
		if (!m_modelName.contains(fileName))
		{
			m_modelName.insert(fileName);
			m_FileEntryHash.insert(fileEntries[i].id(), fileEntries[i]);
			// Add the model to the view
			m_pAlbumManagerView->addModel(fileEntries[i].id());
		}
	}

	// Set the first item as current if the album is not addded to the current one
	if (setFirstItemCurrent) m_pAlbumManagerView->setCurrent(0);

	addToRecentAlbums(m_CurrentAlbumName);
	startLoading();

}
