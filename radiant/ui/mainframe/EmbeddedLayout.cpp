#include "EmbeddedLayout.h"

#include "itextstream.h"
#include "ieventmanager.h"
#include "iuimanager.h"
#include "igroupdialog.h"
#include "imainframe.h"
#include "ientityinspector.h"

#include "gtkutil/FramedWidget.h"
#include "gtkutil/Paned.h"
#include <gtk/gtkvbox.h>

#include "camera/GlobalCamera.h"
#include "ui/texturebrowser/TextureBrowser.h"
#include "xyview/GlobalXYWnd.h"

namespace ui {

std::string EmbeddedLayout::getName() {
	return EMBEDDED_LAYOUT_NAME;
}

void EmbeddedLayout::activate()
{
	// Get the toplevel window
	GtkWindow* parent = GlobalMainFrame().getTopLevelWindow();

	// Create a new camera window and parent it
	_camWnd = GlobalCamera().createCamWnd();
	 // greebo: The mainframe window acts as parent for the camwindow
	_camWnd->setContainer(parent);
	// Pack in the camera window
	GtkWidget* camWindow = gtkutil::FramedWidget(_camWnd->getWidget());

	// Allocate a new OrthoView and set its ViewType to XY
	XYWndPtr xyWnd = GlobalXYWnd().createEmbeddedOrthoView();
    xyWnd->setViewType(XY);
    // Create a framed window out of the view's internal widget
    GtkWidget* xyView = gtkutil::FramedWidget(xyWnd->getWidget());

	// Detach the notebook from the groupdialog to fit it into our pane
	GtkWidget* groupPane = createGroupPane();

	// Now pack those widgets into the paned widgets

	// First, pack the groupPane and the camera
	gtkutil::Paned groupCamPane(gtkutil::Paned::Vertical, camWindow, groupPane);
	_groupCamPane = groupCamPane.getWidget();
    
	gtkutil::Paned horizPane(gtkutil::Paned::Horizontal, _groupCamPane, xyView);
	_horizPane = horizPane.getWidget();
    
	// Retrieve the main container of the main window
	GtkWidget* mainContainer = GlobalMainFrame().getMainContainer();
	gtk_container_add(GTK_CONTAINER(mainContainer), GTK_WIDGET(_horizPane));

	// Set some default values for the width and height
	gtk_paned_set_position(GTK_PANED(_horizPane), 500);
	gtk_paned_set_position(GTK_PANED(_groupCamPane), 350);

	// Connect the pane position trackers
	_posHPane.connect(_horizPane);
	_posGroupCamPane.connect(_groupCamPane);
	
	// Now load the paned positions from the registry
	if (GlobalRegistry().keyExists("user/ui/mainFrame/embedded/pane[@name='horizontal']"))
	{
		_posHPane.loadFromPath("user/ui/mainFrame/embedded/pane[@name='horizontal']");
		_posHPane.applyPosition();
	}

	if (GlobalRegistry().keyExists("user/ui/mainFrame/embedded/pane[@name='texcam']"))
	{
		_posGroupCamPane.loadFromPath("user/ui/mainFrame/embedded/pane[@name='texcam']");
		_posGroupCamPane.applyPosition();
	}
	
	gtk_widget_show_all(mainContainer);

	// This is needed to fix a weirdness when re-parenting the entity inspector
	GlobalGroupDialog().showDialogWindow();

	// greebo: Now that the dialog is shown, tell the Entity Inspector to reload 
	// the position info from the Registry once again.
	GlobalEntityInspector().restoreSettings();

	// Reparent the notebook to our local pane (after the other widgets have been realised)
	GlobalGroupDialog().reparentNotebook(groupPane);

	// Hide the floating window again
	GlobalGroupDialog().hideDialogWindow();

	// Create the texture window
	GtkWidget* texWindow = gtkutil::FramedWidget(
		GlobalTextureBrowser().constructWindow(GlobalMainFrame().getTopLevelWindow())
	);

	// Add the Texture Browser page to the group dialog
	GlobalGroupDialog().addPage(
    	"textures",	// name
    	"Textures", // tab title
    	"icon_texture.png", // tab icon 
    	GTK_WIDGET(texWindow), // page widget
    	"Texture Browser"
    );

	// Hide the camera toggle option for non-floating views
    GlobalUIManager().getMenuManager().setVisibility("main/view/cameraview", false);
	// Hide the console/texture browser toggles for non-floating/non-split views
	GlobalUIManager().getMenuManager().setVisibility("main/view/textureBrowser", false);	
}

GtkWidget* EmbeddedLayout::createGroupPane() {
	GtkWidget* vbox = gtk_vbox_new(FALSE, 0);
	return vbox;
}

void EmbeddedLayout::deactivate() {
	// Show the camera toggle option again
    GlobalUIManager().getMenuManager().setVisibility("main/view/cameraview", true);
	GlobalUIManager().getMenuManager().setVisibility("main/view/textureBrowser", true);

	std::string path("user/ui/mainFrame/embedded");
		
	// Remove all previously stored pane information 
	GlobalRegistry().deleteXPath(path + "//pane");
	GlobalRegistry().createKeyWithName(path, "pane", "horizontal");

	_posHPane.saveToPath(path + "/pane[@name='horizontal']");
	
	GlobalRegistry().createKeyWithName(path, "pane", "texcam");
	_posGroupCamPane.saveToPath(path + "/pane[@name='texcam']");

	// Delete all active views
	GlobalXYWnd().destroyViews();

	// Delete the CamWnd
	_camWnd = CamWndPtr();

	// Give the notebook back to the GroupDialog
	GlobalGroupDialog().reparentNotebook(GlobalGroupDialog().getDialogWindow());

	// Hide the group dialog
	GlobalGroupDialog().hideDialogWindow();

	GlobalGroupDialog().removePage("textures");
	GlobalTextureBrowser().destroyWindow();

	// Destroy the widget, so it gets removed from the main container
	gtk_widget_destroy(GTK_WIDGET(_horizPane));
}

// The creation function, needed by the mainframe layout manager
EmbeddedLayoutPtr EmbeddedLayout::CreateInstance() {
	return EmbeddedLayoutPtr(new EmbeddedLayout);
}

} // namespace ui
