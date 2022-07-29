
#include "ofxSurfing_ImGui_LayoutManager.h"

//--------------------------------------------------------------
ofxSurfing_ImGui_Manager::ofxSurfing_ImGui_Manager()
{
	// Simplify namespaces!
	//TODO:
	namespace ofxSurfingImGui = ofxImGuiSurfing;

	//----

	ofAddListener(ofEvents().keyPressed, this, &ofxSurfing_ImGui_Manager::keyPressed);

	// Auto call draw
	ofAddListener(ofEvents().draw, this, &ofxSurfing_ImGui_Manager::draw, OF_EVENT_ORDER_AFTER_APP);
	//ofAddListener(ofEvents().draw, this, &ofxSurfing_ImGui_Manager::draw, OF_EVENT_ORDER_APP);

	// Callbacks
	ofAddListener(params_AppSettings.parameterChangedE(), this, &ofxSurfing_ImGui_Manager::Changed_Params);
	ofAddListener(params_WindowSpecials.parameterChangedE(), this, &ofxSurfing_ImGui_Manager::Changed_Params);

	//--

	params_Advanced.add(bLinkWindows);
	params_Advanced.add(bAutoResize);

	params_Advanced.add(bExtra);
	params_Advanced.add(bReset);
	params_Advanced.add(bReset_Window);
	params_Advanced.add(bLockMove);
	params_Advanced.add(bNoScroll);
	params_Advanced.add(bMinimize);
	params_Advanced.add(bMinimizePresets);
	params_Advanced.add(bAdvanced);
	params_Advanced.add(bKeys);
	params_Advanced.add(bMouseWheel);
	params_Advanced.add(bHelp);
	params_Advanced.add(bHelpInternal);
	params_Advanced.add(bDebug);
	params_Advanced.add(bLog);

	params_Advanced.add(windowsSpecialsOrganizer.pad);

	// Exclude from settings
	bLockMove.setSerializable(false);
	bNoScroll.setSerializable(false);
	bAdvanced.setSerializable(false);
	bExtra.setSerializable(false);
	bReset.setSerializable(false);
	bReset_Window.setSerializable(false);

	//-

	// -> TODO: BUG?: 
	// it seems than requires to be false when using multi-context/instances
	// if is setted to true, sometimes it hangs and gui do not refresh/freezes.
	bAutoDraw = false;
}

//--------------------------------------------------------------
ofxSurfing_ImGui_Manager::~ofxSurfing_ImGui_Manager() {

	ofRemoveListener(ofEvents().keyPressed, this, &ofxSurfing_ImGui_Manager::keyPressed);
	ofRemoveListener(ofEvents().draw, this, &ofxSurfing_ImGui_Manager::draw, OF_EVENT_ORDER_APP);

	ofRemoveListener(params_LayoutPresetsStates.parameterChangedE(), this, &ofxSurfing_ImGui_Manager::Changed_Params);
	ofRemoveListener(params_AppSettings.parameterChangedE(), this, &ofxSurfing_ImGui_Manager::Changed_Params);
	ofRemoveListener(params_WindowSpecials.parameterChangedE(), this, &ofxSurfing_ImGui_Manager::Changed_Params);

	saveAppSettings();
}

//--

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::setup(ofxImGuiSurfing::SurfingImGuiInstantiationMode mode) {
	surfingImGuiMode = mode;

	switch (surfingImGuiMode)
	{
	case ofxImGuiSurfing::IM_GUI_MODE_UNKNOWN:
		break;

	case ofxImGuiSurfing::IM_GUI_MODE_INSTANTIATED:
		setAutoSaveSettings(true);
		setImGuiAutodraw(true);
		setupInitiate(); // This instantiates and configures ofxImGui inside the class object.
		break;

	case ofxImGuiSurfing::IM_GUI_MODE_INSTANTIATED_DOCKING:
		numPresetsDefault = DEFAULT_AMOUNT_PRESETS;
		setAutoSaveSettings(true);
		setupDocking();
		setupInitiate();
		break;

	case ofxImGuiSurfing::IM_GUI_MODE_INSTANTIATED_SINGLE:
		setAutoSaveSettings(true);
		setImGuiAutodraw(true);
		setupInitiate(); // This instantiates and configures ofxImGui inside the class object.
		break;

		//	//TODO:
		//case ofxImGuiSurfing::IM_GUI_MODE_SPECIAL_WINDOWS:
		//	setWindowsMode(IM_GUI_MODE_WINDOWS_SPECIAL_ORGANIZER);
		//	setup();
		//	break;

	case ofxImGuiSurfing::IM_GUI_MODE_REFERENCED: //TODO:
		setAutoSaveSettings(false);
		break;

		// -> guiManager.begin(); it's bypassed internally then can remain uncommented.
	case ofxImGuiSurfing::IM_GUI_MODE_NOT_INSTANTIATED:
		setAutoSaveSettings(false);
		break;
	}

	//--

	// Clear

	params_Layouts.clear();
	params_LayoutsExtra.clear();
	params_LayoutsVisible.clear();
}

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::setupDocking()
{
	surfingImGuiMode = ofxImGuiSurfing::IM_GUI_MODE_INSTANTIATED_DOCKING;

	setAutoSaveSettings(true);
	setImGuiDocking(true);
	setImGuiDockingModeCentered(true);
	setImGuiAutodraw(true);
}

//--

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::setupInitiate()
{
	// For using internal instantiated GUI.
	// Called by all modes except when using the external scope modes aka not instantiated.
	// In that case we will only use the widgets helpers into a parent/external ImGui context!
	if (surfingImGuiMode == ofxImGuiSurfing::IM_GUI_MODE_NOT_INSTANTIATED) return;

	//--

	// MouseWheel link
	widgetsManager.bMouseWheel.makeReferenceTo(bMouseWheel);

	// MouseWheel link
	windowsSpecialsOrganizer.bDebug.makeReferenceTo(bDebug);
	
	// Minimizes link
	bMinimizePresets.makeReferenceTo(bMinimize);
	//bMinimizePanels.makeReferenceTo(bMinimize);

	//--

	//TODO:
	// When using docking/presets mode
	// we enable special windows by default
	if (surfingImGuiMode == IM_GUI_MODE_INSTANTIATED_DOCKING)
	{
		this->setWindowsMode(IM_GUI_MODE_WINDOWS_SPECIAL_ORGANIZER);

		// Add LINK to extra params
		// that allows that each presets could have his own link state enabled or disabled.
		// to allow linking or floating windows. 
		this->addExtraParamToLayoutPresets(this->getWindowsSpecialEnablerLinker());
	}

	//--

	// Main initialization for ImGui object!

	setupImGui();

	//--

	// Settings
	{
		// Default root path to contain all the file settings!
		path_Global = "Gui/";

		// Layout Presets
		path_ImLayouts = path_Global + "Presets/";

		// Create folders if required
		ofxImGuiSurfing::CheckFolder(path_Global);

		if (bUseLayoutPresetsManager) ofxImGuiSurfing::CheckFolder(path_ImLayouts);

		//--

		// Some internal settings
		path_AppSettings = path_Global + "guiManager_" + bGui_LayoutsPanels.getName() + path_SubPathLabel + ".json";
		// this allow multiple add-ons instances with non shared settings.

		// Add the basic param settings
		//TODO:
		//if (!bUseLayoutPresetsManager)
		{
			params_AppSettings.add(params_Advanced);
		}
	}
}

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::setup(ofxImGui::Gui& _gui) { // using external instantiated gui

	if (surfingImGuiMode == ofxImGuiSurfing::IM_GUI_MODE_NOT_INSTANTIATED) return;

	//-

	guiPtr = &_gui;

	setupImGui();
}

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::setupImGuiFonts()
{
	std::string _fontName;
	float _fontSizeParam;
	_fontName = FONT_DEFAULT_FILE; // WARNING: will not crash or notify you if the file font not present
	_fontSizeParam = FONT_DEFAULT_SIZE;

	std::string _path = "assets/fonts/"; // assets folder

	// We can set an alternative font like a legacy font
	ofFile fileToRead(_path + _fontName); // To check if font file exists
	if (!fileToRead.exists())
	{
		_fontName = FONT_DEFAULT_FILE_LEGACY;
		_fontSizeParam = FONT_DEFAULT_SIZE_LEGACY;
	}


	// Font default
	pushFont(_path + _fontName, _fontSizeParam); // queue default font too

	// Font big
	pushFont(_path + _fontName, _fontSizeParam * 1.5f); // queue big font too

	// Font huge
	pushFont(_path + _fontName, _fontSizeParam * 2.f); // queue huge font too

	//--

	// Set default
	addFont(_path + _fontName, _fontSizeParam);
}

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::setupImGui()
{
	if (surfingImGuiMode == ofxImGuiSurfing::IM_GUI_MODE_NOT_INSTANTIATED) return;

	//--

	ImGuiConfigFlags flags = ImGuiConfigFlags_None;

	// Hard coded settings

	bool bRestore = true;
	bool bMouse = false;

	if (bDockingLayoutPresetsEngine) flags += ImGuiConfigFlags_DockingEnable;
	if (bViewport) flags += ImGuiConfigFlags_ViewportsEnable;

	// Setup ImGui with the appropriate config flags

	if (guiPtr != nullptr) guiPtr->setup(nullptr, bAutoDraw, flags, bRestore, bMouse);
	else gui.setup(nullptr, bAutoDraw, flags, bRestore, bMouse);

	// Uncomment below to perform docking with SHIFT key
	// Gives a better user experience, matter of opinion.

	if (bDockingLayoutPresetsEngine) ImGui::GetIO().ConfigDockingWithShift = true;

	// Uncomment below to "force" all ImGui windows to be standalone
	//ImGui::GetIO().ConfigViewportsNoAutoMerge=true;

	//--

	// Fonts

	setupImGuiFonts();

	//--

	// Theme

	// Colors and widgets/spacing sizes
	ofxImGuiSurfing::ImGui_ThemeMoebiusSurfingV2();
}

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::startup()
{
	bStartupCalled = true;

	//--

	// Last setup step

	if (bDockingLayoutPresetsEngine)
	{
		setupLayout((int)DEFAULT_AMOUNT_PRESETS); // Init Default Layout with 4 presets.

		//--

		appLayoutIndex = appLayoutIndex;

		//--

		////TODO:
		//// enable special windows
		//this->setWindowsMode(IM_GUI_MODE_WINDOWS_SPECIAL_ORGANIZER);
	}

	//--

	// Special Windows Organizer

	if (specialsWindowsMode == IM_GUI_MODE_WINDOWS_SPECIAL_ORGANIZER)
	{
		// Cascade / Organizer Mode
		// Special windows manager

		initiatieSpecialWindowsOrganizer();

		//// Customize names
		//// for file settings
		//windowsSpecialsOrganizer.setName("Show Global");

		//windowsSpecialsOrganizer.setNameWindowsSpecialsEnableGlobal("Show Global");
		//setNameWindowsSpecialsOrganizer("Organizer");

		if (surfingImGuiMode == IM_GUI_MODE_INSTANTIATED_DOCKING)
		{
			// workflow
			windowsSpecialsOrganizer.setHideWindows(true);
		}

		// Docking mode has the GUI toggles in other panels..
		else
		{
			//// workflow
			//// force disable to avoid collide settings layout!
			//windowsSpecialsOrganizer.bGui_WindowsSpecials = false;
		}
	}

	//--

	//if (surfingImGuiMode != IM_GUI_MODE_INSTANTIATED_DOCKING)
	//{
	//}

	//--

	// Aligners

	windowsSpecialsOrganizer.bGui_WindowsAlignHelpers.makeReferenceTo(bGui_WindowsAlignHelpers);

	//--

	// Help
	{
		// Help Text Box internal

		textBoxWidgetInternal.setPath(path_Global + "HelpBox_Internal/");
		textBoxWidgetInternal.setup();
		buildHelpInfo();

		//--

		// Help Text Box app

		textBoxWidgetApp.setPath(path_Global + "HelpBox_App/");
		textBoxWidgetApp.setup();

		//--

		setEnableHelpInfoInternal(true);
	}

	//--

	// Startup

	//-

	// Load some internal settings
	bool bNoSettingsFound;
	bNoSettingsFound = !loadAppSettings();
	// Will return false if settings file do not exist. 
	// That happens when started for first time or after OF_APP/bin cleaning

	if (bNoSettingsFound)
	{
		setShowAllPanels(false);//none

		bHelp = true;
		bHelpInternal = false;

		bMinimize = false;
		//bMinimizePanels = false;
		//bMinimizePresets = false;

		//rect1_Panels.set(ofRectangle(ofGetWidth() / 2, 10, 100, 100));
		//rect0_Presets.set(ofRectangle(10, 10, 100, 100));
		////rect2_Manager.set(ofRectangle(10, 10, 100, 100));

		textBoxWidgetApp.setPosition(400,10);
		textBoxWidgetInternal.setPosition(800,10);
	}
}

//----

// Help

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::buildHelpInfo()
{
	helpInfo = "";

	//helpInfo += "GUI MANAGER \n";
	helpInfo += "HELP INTERNAL \n\n";

	//if (bHelp)
	{
		helpInfo += "Double click to Edit/Lock \n\n";
	}

	helpInfo += "\n";
	helpInfo += "KEY COMMANDS \n\n";

	if (bHelpInternal) helpInfo += "I      HELP INTERNAL ON \n";
	else helpInfo += "I      HELP INTERNAL OFF \n";

	if (bHelp) helpInfo += "H      HELP APP ON \n";
	else helpInfo += "H      HELP APP OFF \n";

	if (bMinimize) helpInfo += "`      MINIMIZE ON \n";
	else helpInfo += "`      MINIMIZE OFF \n";

	if (bExtra) helpInfo += "E      EXTRA ON \n";
	else helpInfo += "E      EXTRA OFF \n";

	if (bLog) helpInfo += "L      LOG ON \n";
	else helpInfo += "L      LOG OFF \n";

	if (bDebug) helpInfo += "D      DEBUG ON \n";
	else helpInfo += "D      DEBUG OFF \n";

	helpInfo += "\n\n";

	if (surfingImGuiMode == ofxImGuiSurfing::IM_GUI_MODE_INSTANTIATED_DOCKING)
		if (bDockingLayoutPresetsEngine)
		{
			helpInfo += "LAYOUTS PRESETS ENGINE \n";
			helpInfo += "\n";

			helpInfo += "F1-F2-F3-F4      PRESETS \n";
			helpInfo += "P1-P2-P3-P4 \n";
			helpInfo += "\n";
			helpInfo += "F5      LAYOUTS \n";
			helpInfo += "F6      PANELS \n";
			helpInfo += "F7      MANAGER \n";
			helpInfo += "F8      EXTRA \n";
			helpInfo += "F9      MINIMIZE \n";
			helpInfo += "\n\n";

			helpInfo += "HOW TO \n";
			helpInfo += "\n";
			helpInfo += "1. CLICK ON P1-P2-P3-P4 \nTO SET A LAYOUT PRESET. \n";
			helpInfo += "\n";
			helpInfo += "2. ENABLE OR HIDE THE PANELS \nYOU WANT VISIBLE \nFOR CURRENT PRESET. \n";
			helpInfo += "\n";
			helpInfo += "3. MOVE AND RESIZE THE PANELS. \n";
			helpInfo += "\n";
			helpInfo += "4. SET ANOTHER LAYOUT PRESET. \n";
			helpInfo += "\n\n";

			helpInfo += "MORE TIPS \n";
			helpInfo += "\n";
			helpInfo += "- DISABLE MINIMIZE TOGGLES \nTO LOOK HIDDEN CONTROLS. \n";
			helpInfo += "\n";
			helpInfo += "- EXPLORE MORE DEEP INTO \n'LAYOUT', 'PANELS' \nAND 'MANAGER' WINDOWS. \n";
			helpInfo += "\n";
			helpInfo += "- EACH LAYOUT PRESET CAN BE THINKED \nAS AN APP MODE OR ACTIVED SECTION. \n";
			helpInfo += "\n";
			helpInfo += "- WHEN NO PRESET IS ENABLED \nALL PANELS WILL BE HIDDEN. \n";
			helpInfo += "\n";
			helpInfo += "- WHEN NO MINIMIZED, ON EXTRA ZONE, \nYOU CAN ENABLE MENU, \nLOG ON EACH PRESET. \n";
			helpInfo += "\n\n";
		}

	//setHelpInfoInternal(helpInfo);
	textBoxWidgetInternal.setText(helpInfo);
}

//----

// Fonts

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::setDefaultFontIndex(int index)
{
	if (customFonts.size() == 0) return;

	currFont = ofClamp(index, 0, customFonts.size() - 1);
	customFont = customFonts[currFont];
}

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::setDefaultFont()//will apply the first added font file
{
	setDefaultFontIndex(0);
}

//TODO:
//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::clearFonts()
{
	customFonts.clear();

	auto& io = ImGui::GetIO();
	io.Fonts->Clear();
}

//--------------------------------------------------------------
bool ofxSurfing_ImGui_Manager::pushFont(std::string path, int size)
{
	//TODO:
	// should be a vector with several customFont to allow hot reloading..
	// if not, last added font will be used
	ofLogNotice(__FUNCTION__) << path << " : " << size;

	auto& io = ImGui::GetIO();
	auto normalCharRanges = io.Fonts->GetGlyphRangesDefault();

	ofFile fileToRead(path); // a file that exists
	bool b = fileToRead.exists();
	if (b)
	{
		ImFont* _customFont = nullptr;
		if (guiPtr != nullptr) {
			_customFont = guiPtr->addFont(path, size, nullptr, normalCharRanges);
		}
		else
		{
			_customFont = gui.addFont(path, size, nullptr, normalCharRanges);
		}

		if (_customFont != nullptr)
		{
			customFonts.push_back(_customFont);
			customFont = _customFont;
			currFont = customFonts.size() - 1;
		}
	}
	else {
		ofLogError(__FUNCTION__) << path << " NOT FOUND!";
	}
	if (customFont != nullptr) io.FontDefault = customFont;

	return b;
}

//--------------------------------------------------------------
bool ofxSurfing_ImGui_Manager::addFont(std::string path, int size)
{
	//TODO:
	// should be a vector with several customFont to allow hot reloading..
	// if not, last added font will be used

	auto& io = ImGui::GetIO();
	auto normalCharRanges = io.Fonts->GetGlyphRangesDefault();

	ofFile fileToRead(path); // a file that exists
	bool b = fileToRead.exists();
	if (b)
	{
		if (guiPtr != nullptr) customFont = guiPtr->addFont(path, size, nullptr, normalCharRanges);
		else customFont = gui.addFont(path, size, nullptr, normalCharRanges);
	}

	if (customFont != nullptr) io.FontDefault = customFont;

	return b;
}

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::pushStyleFont(int index)
{
	if (index < customFonts.size())
	{
		if (customFonts[index] != nullptr)
			ImGui::PushFont(customFonts[index]);
	}
	else
	{
		bIgnoreNextPopFont = true; // workaround to avoid crashes
	}
}

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::popStyleFont()
{
	//TODO: will crash if not pushed..
	//workaround to avoid crashes
	if (bIgnoreNextPopFont)
	{
		bIgnoreNextPopFont = false;

		return;
	}

	ImGui::PopFont();
}

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::processOpenFileSelection(ofFileDialogResult openFileResult, int size = 10) {

	std::string path = openFileResult.getPath();

	ofLogNotice(__FUNCTION__) << "getName(): " << openFileResult.getName();
	ofLogNotice(__FUNCTION__) << "getPath(): " << path;

	ofFile file(path);

	if (file.exists())
	{
		ofLogNotice(__FUNCTION__) << ("The file exists - now checking the type via file extension");
		std::string fileExtension = ofToUpper(file.getExtension());

		// We only want ttf/otf
		if (fileExtension == "TTF" || fileExtension == "OTF") {

			ofLogNotice(__FUNCTION__) << ("TTF or OTF found!");

			pushFont(path, size);
		}
		else ofLogError(__FUNCTION__) << ("TTF or OTF not found!");
	}
}

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::openFontFileDialog(int size)
{
	// Open the Open File Dialog
	ofFileDialogResult openFileResult = ofSystemLoadDialog("Select a font file, TTF or OTF to add to ImGui", false, ofToDataPath(""));

	// Check if the user picked a file
	if (openFileResult.bSuccess) {

		ofLogNotice(__FUNCTION__) << ("User selected a file");

		// We have a file, check it and process it
		processOpenFileSelection(openFileResult, size);
	}
	else {
		ofLogNotice(__FUNCTION__) << ("User hit cancel");
	}
}

//----

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::update() { // -> Not being used by default
	//if (ofGetFrameNum() == 1)
	//{
	//	appLayoutIndex = appLayoutIndex;
	//}
}

//--------------------------------------------------------------
//void ofxSurfing_ImGui_Manager::draw() // -> Not being used by default
void ofxSurfing_ImGui_Manager::draw(ofEventArgs& args)
{
	if (!bAutoDraw) if (customFont == nullptr) gui.draw();

	//--

	// Draw Help boxes

	// Internal
	if (bHelpInternal)
	{
		if (bUseHelpInfoInternal) textBoxWidgetInternal.draw();
	}

	// App
	if (bHelp)
	{
		if (bUseHelpInfoApp) textBoxWidgetApp.draw();
	}
}

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::updateLayout() {

	// Layouts

	if (ini_to_load)
	{
		ofLogNotice(__FUNCTION__) << "LOAD! " << ini_to_load;

		loadLayoutImGuiIni(ini_to_load);

		ini_to_load = NULL;
	}

	if (ini_to_save)
	{
		ofLogNotice(__FUNCTION__) << "SAVE! " << ini_to_save;

		if (ini_to_save != "-1")
		{
			saveLayoutPreset(ini_to_save);
		}

		ini_to_save = NULL;
	}
}

//----

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::drawLayoutsManager()
{
	ImGuiWindowFlags flagsMng = ImGuiWindowFlags_None;

	// Exclude from ImGui ini settings
	flagsMng |= ImGuiWindowFlags_NoSavedSettings;

	if (bAutoResize) flagsMng |= ImGuiWindowFlags_AlwaysAutoResize;

	//--

	ImGuiCond mngCond;
	mngCond = ImGuiCond_Appearing;

	// Right to the Presets Window (tittled as Layouts)
	/**/
	bool blocked = true;
	if (blocked)
	{
		int _pad = windowsSpecialsOrganizer.pad;
		//int _pad = PADDING_PANELS;

		glm::vec2 pos = rectangles_Windows[0].get().getTopRight();
		ofRectangle r = rectangles_Windows[2];
		r.setPosition(pos.x + _pad, pos.y);
		r.setWidth(rectangles_Windows[0].get().getWidth());

		//TODO:
		//set same height than Layouts panel if visible
		if (bGui_LayoutsPanels) r.setHeight(rectangles_Windows[0].get().getHeight());

		//if (bGui_LayoutsPanels) r.setHeight(100);
		//if (bGui_LayoutsPanels) r.setHeight(rectangles_Windows[2].get().getHeight());

		rectangles_Windows[2] = r;
		mngCond = ImGuiCond_Always;
	}

	const int i = 2;
	ImGui::SetNextWindowPos(ofVec2f(rectangles_Windows[i].get().getX(), rectangles_Windows[i].get().getY()), mngCond);

	//TODO: disable height
	//ImGui::SetNextWindowSize(ofVec2f(rectangles_Windows[i].get().getWidth(), rectangles_Windows[i].get().getHeight()), mngCond);

	//-

	IMGUI_SUGAR__WINDOWS_CONSTRAINTSW_SMALL;
	if (beginWindow(bGui_LayoutsManager, flagsMng))
	{
		const int i = 2;
		rectangles_Windows[i].setWithoutEventNotifications(ofRectangle(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight()));

		//-

		float _w = ofxImGuiSurfing::getWidgetsWidth();
		float _h = 2 * ofxImGuiSurfing::getWidgetsHeightRelative();

		//-

		// Minimize
		//ofxImGuiSurfing::AddToggleRounded(bMinimizePresets);

		// Panels & Presets
		AddBigToggle(bGui_LayoutsPanels, _w, _h, false);
		AddBigToggle(bGui_LayoutsPresets, _w, _h, false);

		if (!bMinimizePresets && !bGui_LayoutsPanels)
		{
			this->AddSpacingSeparated();
		}

		//--

		this->AddSpacingSeparated();

		//--

		// Extra Params

		if (!bMinimizePresets)
		{
			this->AddGroup(params_LayoutsExtra);
		}

		//--

		if (!bGui_LayoutsPanels)
		{
			static bool bOpen = false;
			ImGuiColorEditFlags _flagw = (bOpen ? ImGuiWindowFlags_NoCollapse : ImGuiWindowFlags_None);
			if (ImGui::CollapsingHeader("Panels", _flagw))
			{
				this->AddSpacing();

				for (int i = 0; i < windowsSpecialsLayouts.size(); i++)
				{
					AddToggleRoundedButton(windowsSpecialsLayouts[i].bGui);
				}

				this->AddSpacing();

				float _w2 = ofxImGuiSurfing::getWidgetsWidth(2);
				if (ImGui::Button("All", ImVec2(_w2, _h / 2)))
				{
					setShowAllPanels(true);
				}
				ImGui::SameLine();
				if (ImGui::Button("None", ImVec2(_w2, _h / 2)))
				{
					setShowAllPanels(false);
				}
			}
		}

		//if (!bMinimizePresets)
		//{
		//	this->Add(bAutoResizePanels, OFX_IM_TOGGLE_BUTTON_ROUNDED_MINI);
		//	this->Add(bResetWindowPanels, OFX_IM_TOGGLE_BUTTON_ROUNDED_MINI);
		//}

		//--

		if (!bMinimizePresets)
		{
			drawAdvancedBundle();
		}

		this->endWindow();
	}
}

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::drawLayouts() {

	//TODO:
	// How to make all windows dockeable in the same space ?
	if (bGui_LayoutsPresets)
	{
		drawLayoutsPresets(); // main presets clicker

		if (!bMinimizePresets) if (bGui_LayoutsExtra) drawLayoutsExtra();
	}

	if (bGui_LayoutsPanels) drawLayoutsPanels();
	// Draws all sections except drawLayoutsManager();

	// Log
	if (appLayoutIndex != -1) if (bLog) log.ImGui(bLog);
	//if (bLog) log.ImGui("Log");
}

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::drawLayoutPresetsEngine() {

	if (bUseLayoutPresetsManager && bGui_LayoutsManager && !bMinimizePresets)
		drawLayoutsManager();

	//----

	if (bUseLayoutPresetsManager)
	{
		updateLayout(); // to attend save load flags

		//----

		if (bDockingLayoutPresetsEngine)
		{
			ImGuiID dockNodeID;
			ImGuiDockNode* dockNode;
			//ImGuiDockNode* centralNode;
			//ImGuiDockNodeFlags dockingFlags;

			//----

			// a. Define the ofWindow as a docking space

			//ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0, 0, 0, 0)); // Fixes imgui to expected behaviour. Otherwise add in ImGui::DockSpace() [�line 14505] : if (flags & ImGuiDockNodeFlags_PassthruCentralNode) window_flags |= ImGuiWindowFlags_NoBackground;
			//ImGuiID dockNodeID = ImGui::DockSpaceOverViewport(NULL, ImGuiDockNodeFlags_PassthruCentralNode);
			//ImGui::PopStyleColor();

			//-

			// b. Lockable settings 

			// Fixes imgui to expected behaviour. Otherwise add in ImGui::DockSpace() [�line 14505] : if (flags & ImGuiDockNodeFlags_PassthruCentralNode) window_flags |= ImGuiWindowFlags_NoBackground;
			//ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0, 0, 0, 0));
			//ImGuiDockNodeFlags flagsDock;
			//flagsDock = ImGuiDockNodeFlags_PassthruCentralNode;
			//if (bModeLockControls)
			//{
			//	flagsDock |= ImGuiDockNodeFlags_NoResize;
			//	flagsDock |= ImGuiDockNodeFlags_NoCloseButton;
			//	//flagsDock |= ImGuiDockNodeFlags_NoTabBar;
			//	//flagsDock |= ImGuiDockNodeFlags_NoWindowMenuButton;
			//	//flagsDock |= ImGuiDockNodeFlags_NoMove__;
			//}

			//TODO:
			//dockNodeID = ImGui::DockSpaceOverViewport(NULL, flagsDock);
			//dockNodeID = ImGui::GetID("MyDockSpace");
			//dockNodeID = ImGui::GetID("DockSpace");

			//ImGui::PopStyleColor();

			//----

			// Get check free space
			// central inter docks rectangle

			ImGuiDockNodeFlags flagsDock = ImGuiDockNodeFlags_None;
			//flagsDock += ImGuiDockNodeFlags_DockSpace;
			flagsDock += ImGuiDockNodeFlags_PassthruCentralNode;

			// A
			dockNodeID = ImGui::DockSpaceOverViewport(NULL, flagsDock);
			dockNode = ImGui::DockBuilderGetNode(dockNodeID);

			// B
			//ImGuiDockNode* dockNode = ImGui::DockBuilderGetNode(dockNodeID);

			if (dockNode)
			{
				ImGuiDockNode* centralNode = ImGui::DockBuilderGetCentralNode(dockNodeID);

				// Verifies if the central node is empty (visible empty space for oF)
				if (centralNode && centralNode->IsEmpty())
				{
					ImRect availableSpace = centralNode->Rect();
					//availableSpace.Max = availableSpace.Min + ImGui::GetContentRegionAvail();
					//ImGui::GetForegroundDrawList()->AddRect(availableSpace.GetTL() + ImVec2(8, 8), availableSpace.GetBR() - ImVec2(8, 8), IM_COL32(255, 50, 50, 255));

					ImVec2 viewCenter = availableSpace.GetCenter();
					// Depending on the viewports flag, the XY is either absolute or relative to the oF window.
					//if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) viewCenter = viewCenter - ImVec2(ofGetWindowPositionX(), ofGetWindowPositionY());

					float ww = availableSpace.GetSize().x;
					float hh = availableSpace.GetSize().y;
					rectangle_Central_MAX = ofRectangle(viewCenter.x, viewCenter.y, ww, hh);

					bool bDebug = bDebugRectCentral.get();
					if (bDebug)
					{
						int _wl = 2;
						int pad = 10;

						ofPushStyle();
						ofSetRectMode(OF_RECTMODE_CENTER);

						int g = 0;
						ofColor cl = ofColor::white;
						//ofColor cl = ofColor::orange;

						//int g = 255 * ofxImGuiSurfing::Bounce(0.5);
						int a = 255.f * ofMap(ofxImGuiSurfing::Bounce(1), 0.0f, 1.0f, 0.2f, 1.0f, true);
						ofColor c = ofColor(cl.r, cl.g, cl.b, a);
						//ofColor c = ofColor(g, a);
						ofSetColor(c);

						ofNoFill();
						ofSetLineWidth(_wl);

						float ww = availableSpace.GetSize().x - pad;
						float hh = availableSpace.GetSize().y - pad;

						ofRectangle rDebug;
						rDebug = ofRectangle(viewCenter.x, viewCenter.y, ww, hh);
						ofDrawRectangle(rDebug);

						//ofDrawRectangle(rectangle_Central_MAX);
						ofSetRectMode(OF_RECTMODE_CORNER);
						ofPopStyle();
					}
					// move to left corner mode
					rectangle_Central_MAX.translate(-ww / 2, -hh / 2);

					//-

					static ofRectangle rectangle_Central_MAX_PRE;
					if (rectangle_Central_MAX_PRE != rectangle_Central_MAX) { // updates when layout changes..
						rectangle_Central_MAX_PRE = rectangle_Central_MAX;

						// fit exact rectangle to borders and scaled to fit
						//rectangle_Central = DEMO3_Svg.getRect();
						//if (rectangle_Central_MAX.getWidth() != 0 && rectangle_Central_MAX.getHeight() != 0) // avoid crash
						rectangle_Central.scaleTo(rectangle_Central_MAX, OF_ASPECT_RATIO_KEEP, OF_ALIGN_HORZ_CENTER, OF_ALIGN_VERT_CENTER);

						//// rescaled rectangle a bit
						//float _scale = 0.7f;
						//rectangle_Central_Transposed = rectangle_Central;
						//rectangle_Central_Transposed.scaleFromCenter(_scale, _scale);//scale down to fit layout spacing better
						//rectangle_Central_Transposed.translateY(rectangle_Central.getHeight() * 0.07);//move down a bit
						//DEMO3_Svg.setRect(rectangle_Central_Transposed);
					}
				}
			}
		}

		//----

		//// b. Lockable settings 

		//// Fixes imgui to expected behaviour. Otherwise add in ImGui::DockSpace() [�line 14505] : if (flags & ImGuiDockNodeFlags_PassthruCentralNode) window_flags |= ImGuiWindowFlags_NoBackground;
		////ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0, 0, 0, 0));
		//ImGuiDockNodeFlags flagsDock;
		//flagsDock = ImGuiDockNodeFlags_PassthruCentralNode;
		//if (bModeLockControls)
		//{
		//	flagsDock |= ImGuiDockNodeFlags_NoResize;
		//	flagsDock |= ImGuiDockNodeFlags_NoCloseButton;
		//	//flagsDock |= ImGuiDockNodeFlags_NoTabBar;
		//	//flagsDock |= ImGuiDockNodeFlags_NoWindowMenuButton;
		//	//flagsDock |= ImGuiDockNodeFlags_NoMove__;
		//}
		//ImGuiID dockNodeID = ImGui::DockSpaceOverViewport(NULL, flagsDock);
		////ImGui::PopStyleColor();
	}
}

#ifdef FIXING_DRAW_VIEWPORT
//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::drawOFnative() {

	//TODO: debug viewport. freew space for OF drawing

	ImGuiDockNodeFlags __dockingFlags;
	__dockingFlags = ImGuiDockNodeFlags_PassthruCentralNode;

	//ImGuiViewport* viewport = ImGui::GetMainViewport();

	//auto dockNodeID = ImGui::DockSpaceOverViewport(NULL, __dockingFlags);
	//auto dockNodeID = ImGui::GetID("DockSpace");
	auto dockNodeID = ImGui::GetID("MyDockSpace");

	ImGuiDockNode* dockNode = ImGui::DockBuilderGetNode(dockNodeID);
	if (dockNode)
	{
		ImGuiDockNode* centralNode = ImGui::DockBuilderGetCentralNode(dockNodeID);
		if (centralNode)
			//if (centralNode && centralNode->IsEmpty()) 
		{
			ImRect availableSpace = centralNode->Rect();
			//availableSpace.Max = availableSpace.Min + ImGui::GetContentRegionAvail();
			//ImGui::GetForegroundDrawList()->AddRect(availableSpace.GetTL() + ImVec2(8, 8), availableSpace.GetBR() - ImVec2(8, 8), IM_COL32(255, 50, 50, 255));

			ImVec2 viewCenter = availableSpace.GetCenter();

			// Depending on the viewports flag, the XY is either absolute or relative to the oF window.
			if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) viewCenter = viewCenter - ImVec2(ofGetWindowPositionX(), ofGetWindowPositionY());

			// create rectangle
			rectangle_Central = ofRectangle(viewCenter.x, viewCenter.y, availableSpace.GetWidth(), availableSpace.GetHeight());
			float v = ofxImGuiSurfing::Bounce(1);
			rectangle_Central.setSize(availableSpace.GetWidth() * v, availableSpace.GetHeight() * v);
		}
	}
	else // get the OF viewport
	{
		auto view = ofGetCurrentViewport();
		auto viewCenter = view.getCenter();
		rectangle_Central = ofRectangle(viewCenter.x, viewCenter.y, view.getWidth(), view.getHeight());
		float v = ofxImGuiSurfing::Bounce(1);
		rectangle_Central.setSize(view.getWidth() * v, view.getHeight() * v);
	}

	ofPushStyle();
	{
		ofSetRectMode(OF_RECTMODE_CENTER);
		ofSetLineWidth(4);
		ofColor cl = ofColor::yellow;
		int a = 255.f * ofMap(ofxImGuiSurfing::Bounce(1), 0.0f, 1.0f, 0.2f, 1.0f, true);
		ofColor c = ofColor(cl.r, cl.g, cl.b, a);
		ofSetColor(c);
		ofFill();
		ofDrawCircle(rectangle_Central.getCenter().x, rectangle_Central.getCenter().y, 3);
		ofNoFill();
		ofDrawRectangle(rectangle_Central);

		ofSetRectMode(OF_RECTMODE_CORNER);
	}
	ofPopStyle();
}
#endif

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::startupFirstFrame() {
	//TODO:
	{
		appLayoutIndex = appLayoutIndex;
		if (rectangles_Windows.size() > 0) rectangles_Windows[1] = rectangles_Windows[1];
	}

	// Force call startup(). 
	// Maybe user forgets to do it or to speed up the API setup in some scenarios.
	// i.e. when not using special windows or layout engine
	if (!bStartupCalled) startup();
}

//----

// Global ImGui being/end like ofxImGui
// 
// All the ImGui Stuff goes in between here,
// The RAW ImGui widgets and the API / Engine handled stuff too!
// 
//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::begin() {

	// Check that it's property initialized!
	if (surfingImGuiMode == ofxImGuiSurfing::IM_GUI_MODE_NOT_INSTANTIATED) return;

	//--

	//TODO:
	if (ofGetFrameNum() == 1)
	{
		startupFirstFrame();
	}

	//--

	//TODO:
	// This is used to auto handle indexes and speed up API.. 
	_indexWindowsSpecials = -1;

	//--

	resetUniqueNames(); // Reset unique names

	// This handles the name to Push/Pop widgets IDs
	// Then we can use several times the same ofParameter with many styles,
	// into the same window without colliding!
	// That collide happens when using the original legacy ofxImGui ofParam Helpers!

	//--

	//TODO:
	// Sometimes we could use an ofxImGui external or from a parent scope..
	if (guiPtr != nullptr) guiPtr->begin();
	else gui.begin();

	//--

	if (customFont != nullptr) ImGui::PushFont(customFont);

	// Reset font to default.
	// this clear all the push/pop queue.
	setDefaultFont();

	//--

	//TODO. fix
	//if (!bDockingLayoutPresetsEngine)
	//	if (bMenu) drawMenu();

	//----

	// Extra Tools / Engines

	// 1. Layout Presets Engine

	if (bDockingLayoutPresetsEngine) drawLayoutPresetsEngine();

	//----

	// 2. Special Windows Engine Window Panel

	// Organizer

	//TODO: To draw always, also when not initiated as IM_GUI_MODE_WINDOWS_SPECIAL_ORGANIZER 
	//if (windowsSpecialsOrganizer.bGui_WindowsSpecials) drawWindowsSpecialsOrganizer();

	//TODO:
	// Draw only when initiated as special windows mode

	if (specialsWindowsMode == IM_GUI_MODE_WINDOWS_SPECIAL_ORGANIZER)
	{
		// Main Panels Controller
		if (windowsSpecialsOrganizer.isUsing())
		{
			//TODO:
			// Docking mode has the GUI toggles in other panels..
			//if (surfingImGuiMode != IM_GUI_MODE_INSTANTIATED_DOCKING)
			{
				if (windowsSpecialsOrganizer.bGui_WindowsSpecials) drawWindowsSpecialsOrganizer();
			}
		}
	}


	//----

	// 3. Align Helpers Window Panel

	// Aligners

	if (bGui_WindowsAlignHelpers) drawWindowsAlignHelpers();
}

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::end()
{
	// Check that it's property initialized!
	if (surfingImGuiMode == ofxImGuiSurfing::IM_GUI_MODE_NOT_INSTANTIATED) return;

	//--

	drawLogPanel();

	//--

#ifdef FIXING_DRAW_VIEWPORT
	if (bPreviewSceneViewport) drawOFnative();
#endif

	//--

	if (customFont != nullptr) ImGui::PopFont();

	// Mouse lockers helpers
	// Here we check if mouse is over gui to disable other external stuff
	// e.g. easyCam draggable moving, text input boxes, key commands...

	bMouseOverGui = false;
	bMouseOverGui |= ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
	bMouseOverGui |= ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);
	bMouseOverGui |= ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

	//--

	//TODO:
	// Sometimes we could use an ofxImGui external or from a parent scope..
	if (guiPtr != nullptr) guiPtr->end();
	else gui.end();
}

//--

//--------------------------------------------------------------
bool ofxSurfing_ImGui_Manager::beginWindow(char* name)
{
	ImGuiWindowFlags fg = ImGuiWindowFlags_None;
	if (bAutoResize) fg |= ImGuiWindowFlags_AlwaysAutoResize;

	return beginWindow((string)name, NULL, fg);
}

//--------------------------------------------------------------
bool ofxSurfing_ImGui_Manager::beginWindow(std::string name)
{
	ImGuiWindowFlags fg = ImGuiWindowFlags_None;
	if (bAutoResize) fg |= ImGuiWindowFlags_AlwaysAutoResize;

	return beginWindow(name, NULL, fg);
}

//--------------------------------------------------------------
bool ofxSurfing_ImGui_Manager::beginWindow(std::string name, bool* p_open)
{
	if (!&p_open) return false;

	ImGuiWindowFlags fg = ImGuiWindowFlags_None;
	if (bAutoResize) fg |= ImGuiWindowFlags_AlwaysAutoResize;

	return beginWindow(name, p_open, fg);
}

//--------------------------------------------------------------
bool ofxSurfing_ImGui_Manager::beginWindow(ofParameter<bool>& p)
{
	if (!p.get()) return false;

	ImGuiWindowFlags fg = ImGuiWindowFlags_None;
	if (bAutoResize) fg |= ImGuiWindowFlags_AlwaysAutoResize;

	return beginWindow(p.getName().c_str(), (bool*)&p.get(), fg);
}

//--------------------------------------------------------------
bool ofxSurfing_ImGui_Manager::beginWindow(std::string name, ofParameter<bool>& p)
{
	if (!p.get()) return false;

	ImGuiWindowFlags fg = ImGuiWindowFlags_None;
	if (bAutoResize) fg |= ImGuiWindowFlags_AlwaysAutoResize;

	return beginWindow(name.c_str(), (bool*)&p.get(), fg);
}

//--------------------------------------------------------------
bool ofxSurfing_ImGui_Manager::beginWindow(std::string name, ofParameter<bool>& p, ImGuiWindowFlags window_flags)
{
	if (!p.get()) return false;

	return beginWindow(name.c_str(), (bool*)&p.get(), window_flags);
}

//--------------------------------------------------------------
bool ofxSurfing_ImGui_Manager::beginWindow(ofParameter<bool>& p, ImGuiWindowFlags window_flags)
{
	if (!p.get()) return false;

	return beginWindow(p.getName().c_str(), (bool*)&p.get(), window_flags);
}

// This is the main beginWindow. all above methods call this one!
//--------------------------------------------------------------
bool ofxSurfing_ImGui_Manager::beginWindow(std::string name = "Window", bool* p_open = NULL, ImGuiWindowFlags window_flags = ImGuiWindowFlags_None)
{
	//TODO: 
	//if (bLockMove) window_flags |= ImGuiWindowFlags_NoMove;

	//TODO: 
	//if (bReset_Window) {
	//	bReset_Window = false;
	//	resetWindowImGui(false, true);
	//}

	//--

	// Reset unique names
	// This is to handle the widgets ID to avoid repeat an used name, avoiding collides between them.

	resetUniqueNames();

	//--

	//IMGUI_SUGAR__WINDOWS_CONSTRAINTS;
	//IMGUI_SUGAR__WINDOWS_CONSTRAINTS_SMALL;

	//--

	bool b = ImGui::Begin(name.c_str(), p_open, window_flags);

	//TODO:
	// Early out if the window is collapsed, as an optimization.
	if (!b)
	{
		ImGui::End();

		return false;
	}

	// Refresh layout
	widgetsManager.refreshLayout(); // calculate sizes related to window shape/size

	// When we are instantiating ImGui externally, not inside this addon,
	// we don't handle the font and theme.
	if (surfingImGuiMode != ofxImGuiSurfing::IM_GUI_MODE_NOT_INSTANTIATED)
	{
		// Set default font
		setDefaultFont();
	}

	//TODO: workflow for API / special windows engine too
	//currWindow++;

	return b;
}

//TODO: a faster mode to avoid use indexes..
// that workflow mode could be problematic, bc you must draw the special windows sorted... 
// in the same order than when adding special windows on setup.
//--------------------------------------------------------------
bool ofxSurfing_ImGui_Manager::beginWindowSpecial() {
	_indexWindowsSpecials++;
	bool b = beginWindowSpecial(_indexWindowsSpecials);

	return b;
}

//--------------------------------------------------------------
bool ofxSurfing_ImGui_Manager::beginWindowSpecial(string name)
{
	int _index = getWindowSpecialIndexForName(name);

	if (_index != -1) {
		return beginWindowSpecial(_index);
	}
	else
	{
		ofLogError(__FUNCTION__) << "Special Window with name '" << name << "' not found!";

		return false;
	}
}

//--------------------------------------------------------------
bool ofxSurfing_ImGui_Manager::beginWindowSpecial(int index)
{
	//TODO:
	_indexWindowsSpecials = index; // workflow

	//--

	// Skip window if hidden

	if (index > windowsSpecialsLayouts.size() - 1 || index == -1)
	{
		ofLogError(__FUNCTION__) << "Out of range index for queued windows, " << index;
		return false;
	}

	if (!windowsSpecialsLayouts[index].bGui.get()) return false;

	//--

	ImGuiWindowFlags flags = ImGuiWindowFlags_None;

	//--

	if (specialsWindowsMode == IM_GUI_MODE_WINDOWS_SPECIAL_ORGANIZER)
	{
		if (!windowsSpecialsOrganizer.bGui_ShowAll.get()) return false;

		//--

		if (windowsSpecialsOrganizer.bLinkedWindowsSpecial)
		{
			//TODO: make refresh faster
			// can be moved to global begin() to reduce calls?
			windowsSpecialsOrganizer.update();

			windowsSpecialsOrganizer.runShapeState(index);
		}

		if (!windowsSpecialsOrganizer.bHeaders) flags += ImGuiWindowFlags_NoDecoration;
	}

	//--

	if (bAutoResize) flags += ImGuiWindowFlags_AlwaysAutoResize; // global

	bool b = beginWindow(windowsSpecialsLayouts[index].bGui, flags);

	//--

	//bool b = beginWindow(windowsSpecialsLayouts[index].bGui);
	//if (windowsSpecialsLayouts[index].bMasterAnchor.get()) // window
	//{
	//	if (windowsSpecialsLayouts[index].bAutoResize.get()) {
	//		flags |= ImGuiWindowFlags_AlwaysAutoResize;
	//	}
	//}
	//bool b = beginWindow(windowsSpecialsLayouts[index].bGui.getName().c_str(), (bool*)&windowsSpecialsLayouts[index].bGui.get(), flags);

	//--

	return b;
}

//--------------------------------------------------------------
bool ofxSurfing_ImGui_Manager::beginWindowSpecial(ofParameter<bool>& _bGui)
{
	//TODO:
	if (!_bGui) return false;

	int i = getWindowSpecialIndexForToggle(_bGui);

	if (i != -1)
	{
		return beginWindowSpecial(i);
	}
	else
	{
		ofLogError(__FUNCTION__) << "Special Window toggle not found! " << _bGui.getName();

		return false;
	}
}

//--------------------------------------------------------------
int ofxSurfing_ImGui_Manager::getWindowSpecialIndexForName(string name)
{
	for (size_t i = 0; i < windowsSpecialsLayouts.size(); i++)
	{
		string _name = windowsSpecialsLayouts[i].bGui.getName();

		if (name == _name)
		{
			return  i;
		}
	}

	ofLogError(__FUNCTION__) << "Special Window with name '" << name << "' not found!";

	return -1;
}

//--------------------------------------------------------------
int ofxSurfing_ImGui_Manager::getWindowSpecialIndexForToggle(ofParameter<bool>& _bGui)
{
	string name = _bGui.getName();

	for (size_t i = 0; i < windowsSpecialsLayouts.size(); i++)
	{
		string _name = windowsSpecialsLayouts[i].bGui.getName();

		if (name == _name)
		{
			return  i;
		}
	}

	ofLogError(__FUNCTION__) << "Special Window toggle not found! " << _bGui.getName();

	return -1;
}

//TODO:
//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::endWindowSpecial(ofParameter<bool>& _bGui)
{
	string name = _bGui.getName();
	int i = getWindowSpecialIndexForName(name);

	if (i == -1) {
		ofLogError(__FUNCTION__) << "Special Window with bool param with name '" << name << "' not found!";

		return;
	}

	endWindowSpecial(i);

	return;
}

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::endWindowSpecial(int index)
{
	if (index == -1) index = _indexWindowsSpecials; // workaround

	//--

	if (index > windowsSpecialsLayouts.size() - 1)
	{
		ofLogError(__FUNCTION__) << "Out of range index for queued windows, " << index;

		return;
	}

	//--

	// Skip window if hidden

	if (!windowsSpecialsLayouts[index].bGui.get()) return;

	if (specialsWindowsMode == IM_GUI_MODE_WINDOWS_SPECIAL_ORGANIZER)
	{
		if (!windowsSpecialsOrganizer.bGui_ShowAll.get()) return;
	}

	//--

	//if (windowsSpecialsLayouts[_indexWindowsSpecials].bMasterAnchor.get())
	//{
	//	drawAdvancedControls();
	//}

	//--

	if (specialsWindowsMode == IM_GUI_MODE_WINDOWS_SPECIAL_ORGANIZER)
	{
		if (windowsSpecialsOrganizer.bLinkedWindowsSpecial)
		{
			windowsSpecialsOrganizer.getShapeState(index);

			//TODO: make refresh faster
			//windowsSpecialsOrganizer.update();
		}
	}

	//--

	// workflow: to avoid use the index. but requires sequencial calling
	//_indexWindowsSpecials++;

	ImGui::End();
}

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::endWindow()
{
	ImGui::End();
}

//--

// Docking Helpers

#ifdef FIXING_DOCKING
//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::beginDocking()
{
	// Make windows transparent, to demonstrate drawing behind them.
	//ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(200, 200, 200, 128)); // This styles the docked windows

	ImGuiDockNodeFlags dockingFlags = ImGuiDockNodeFlags_PassthruCentralNode; // Make the docking space transparent
	// Fixes imgui to expected behaviour, having a transparent central node in passthru mode.
	// Alternative: Otherwise add in ImGui::DockSpace() [�line 14505] : if (flags & ImGuiDockNodeFlags_PassthruCentralNode) window_flags |= ImGuiWindowFlags_NoBackground;
	//ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0, 0, 0, 0));

	//dockingFlags |= ImGuiDockNodeFlags_NoDockingInCentralNode; // Uncomment to always keep an empty "central node" (a visible oF space)
	//dockingFlags |= ImGuiDockNodeFlags_NoTabBar; // Uncomment to disable creating tabs in the main view

	// Define the ofWindow as a docking space
	ImGuiID dockNodeID = ImGui::DockSpaceOverViewport(NULL, dockingFlags); // Also draws the docked windows
	//ImGui::PopStyleColor(2);
}
#endif

#ifndef FIXING_DOCKING
//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::beginDocking()
{
	//dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	//ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
	if (bMenu) window_flags |= ImGuiWindowFlags_MenuBar;

	ImGuiViewport* viewport = ImGui::GetMainViewport();

	// fit full viewport
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

	window_flags
		|= ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoCollapse
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove;

	window_flags
		|= ImGuiWindowFlags_NoBringToFrontOnFocus
		| ImGuiWindowFlags_NoNavFocus;

	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace", nullptr, window_flags);
	//ImGui::Begin("MyDockSpace", nullptr, window_flags);
	ImGui::PopStyleVar();
	ImGui::PopStyleVar(2);

	//----

	// DockSpace
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");

		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}
	else
	{
		// Docking is DISABLED - Show a warning message
		//ShowDockingDisabledMessage();
	}

	//----

	// All windows goes here before endDocking()

	drawLayouts();
	}
#endif

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::endDocking()
{
#ifdef FIXING_DOCKING
	return;
#endif

	if (bMenu) drawMenuDocked();

	//-

	//ImGuiIO& io = ImGui::GetIO();
	//if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	//{
	//}

	 // End the parent window that contains the Dockspace:
	ImGui::End(); // ?
}

//----

// Layouts presets management
//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::setupLayout(int numPresets) //-> must call manually after adding windows and layout presets
{
	numPresetsDefault = numPresets; // default is 4 presets with names P0, P1, P2, P3

	//-

	//// Clear
	//params_Layouts.clear();
	//params_LayoutsExtra.clear();
	//params_LayoutsVisible.clear();

	//--

	// 1.1 Store all the window panels show toggles
	// we will remember, on each layout preset, if a window is visible or not!

	for (int i = 0; i < windowsSpecialsLayouts.size(); i++)
	{
		params_LayoutsVisible.add(windowsSpecialsLayouts[i].bGui);
	}

	//--

	//// 1.2 Add other settings that we want to store into each presets

	//-

	// Extra params that will be included into each preset.
	// Then can be different and memorized in different states too,
	// like the common panels. 

	params_LayoutsExtra.add(bMenu);
	params_LayoutsExtra.add(bLog);
	//TODO: should be removed if handled by preset engine..


	//params_LayoutsExtraInternal.clear();
	//params_LayoutsExtraInternal.add(bMenu);
	//params_LayoutsExtraInternal.add(bLog);
	//params_LayoutsExtra.add(params_LayoutsExtraInternal);

	//--

	// 1.2.2 Special Windows Helpers

	if (specialsWindowsMode == IM_GUI_MODE_WINDOWS_SPECIAL_ORGANIZER)
	{
		this->addExtraParamToLayoutPresets(this->getWindowsSpecialEnablerLinker());
		//params_LayoutsExtra.add(windowsSpecialsOrganizer.getParamsUser());
	}

	//--

	// 1.3 Applied to control windows

	//params_LayoutsExtra.add(bModeFree);
	//params_LayoutsExtra.add(bModeForced);
	//params_LayoutsExtra.add(bModeLock1);
	//params_LayoutsExtra.add(bModeLockControls);

	//--

	// 1.4 Pack both groups

	params_Layouts.add(params_LayoutsVisible);
	params_Layouts.add(params_LayoutsExtra);

	//--

	// 2. Initiate

	bLayoutPresets.clear();
	params_LayoutPresetsStates.clear();
	appLayoutIndex.setMax(0);

	//--

	// 3. Populate some presets

	for (int i = 0; i < numPresetsDefault; i++)
	{
		if (namesPresets.size() == 0) {//if names are not defined will be setted by default P0-P1-P2-P3
			createLayoutPreset();
		}
		else {
			if (i < namesPresets.size()) createLayoutPreset(namesPresets[i]);
			else createLayoutPreset();
		}
	}

	//--

	// 4. App states for the next session

	// The main control windows

	params_AppSettings.add(bGui_LayoutsManager);

	//params_AppSettingsLayout.add(bModeFree);
	//params_AppSettingsLayout.add(bModeForced);
	//params_AppSettingsLayout.add(bModeLock1);
	//params_AppSettingsLayout.add(bModeLockControls);
	//params_AppSettingsLayout.add(bModeLockPreset);

	params_AppSettingsLayout.add(bGui_LayoutsPresets);
	params_AppSettingsLayout.add(bGui_LayoutsExtra);
	params_AppSettingsLayout.add(bGui_LayoutsPanels);

	params_AppSettingsLayout.add(bAutoSave_Layout);
	params_AppSettingsLayout.add(bPreviewSceneViewport);
	params_AppSettingsLayout.add(appLayoutIndex);
	params_AppSettingsLayout.add(bSolo);

	params_AppSettings.add(params_AppSettingsLayout);

	//--

	// Exclude from settings but to use the grouped callback
	//bSolo.setSerializable(false);

	//---------------

	// Engine Windows

	// Initiate the 3 control windows
	// We store the shapes using ofRectangles to "split" them from ImGui .ini store manager...

	float x, y, w, h, pad;
	x = ofGetWidth() * 0.6;
	y = 30;
	//y = ofGetHeight() * 0.3;
	w = 200;
	h = 1;
	//pad = 2;
	pad = windowsSpecialsOrganizer.pad;
	
	//--

	rect1_Panels.set(ofRectangle(x, y, w, h));
	//rect1_Panels.set(ofRectangle(x + (pad + w), y, w, h));

	rect0_Presets.set(ofRectangle(10, y, w, h));
	//rect0_Presets.set(ofRectangle(x, y, w, h));
	
	rect2_Manager.set(ofRectangle(x + 2 * (pad + w), y, w, h));

	//--

	rectangles_Windows.clear();
	rectangles_Windows.emplace_back(rect0_Presets);
	rectangles_Windows.emplace_back(rect1_Panels);
	rectangles_Windows.emplace_back(rect2_Manager);

	// to store settings to disk
	params_RectPanels.clear();
	params_RectPanels.add(rect0_Presets);
	params_RectPanels.add(rect1_Panels);
	params_RectPanels.add(rect2_Manager);

	params_WindowsEngine.clear();
	params_WindowsEngine.add(params_RectPanels);

	//--

	// Presets and Panels Windows

	params_WindowPresets.add(bResetWindowPresets);
	params_WindowPresets.add(bAutoResizePresets);
	params_WindowPresets.add(bMinimizePresets);

	params_WindowPanels.add(bResetWindowPanels);
	params_WindowPanels.add(bAutoResizePanels);

	//params_WindowPresets.add(bMinimizePanels);

	params_WindowsEngine.add(params_WindowPresets);
	params_WindowsEngine.add(params_WindowPanels);

	params_AppSettings.add(params_WindowsEngine);

	//----

	//TODO: simplify calls merging to one group only...

	// Callbacks
	ofAddListener(params_LayoutPresetsStates.parameterChangedE(), this, &ofxSurfing_ImGui_Manager::Changed_Params);

	//--

	//// Gui - > which panels enabled but overwritten by Layout Presets Engine
	//params_AppSettings.add(params_WindowSpecials);

	//--

	setImGuiLayoutPresets(true);
}

//--------------------------------------------------------------
bool ofxSurfing_ImGui_Manager::loadAppSettings()
{
	bool b = false;
	if (bAutoSaveSettings) b = ofxImGuiSurfing::loadGroup(params_AppSettings, path_AppSettings, true);

	return b;

	// Will return false if settings file do not exist. That happens when started for first time or after OF_APP/bin cleaning
}

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::saveAppSettings()
{
	if (bAutoSaveSettings) ofxImGuiSurfing::saveGroup(params_AppSettings, path_AppSettings, true);
}

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::saveAppLayout(int _index)
{
	if (_index == -1) return;

	ini_to_save_Str = getLayoutName(_index);

	ofLogNotice(__FUNCTION__) << ini_to_save_Str;

	if (ini_to_save_Str == "-1") return; // skip

	// Flag to save .ini on update
	ini_to_save = ini_to_save_Str.c_str();

	// Save group
	saveLayoutPresetGroup(ini_to_save);
}

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::loadAppLayout(int _index)
{
	if (_index == -1) return;

	//if (appLayoutIndex == _index) return; // skip

	appLayoutIndex = ofClamp(_index, appLayoutIndex.getMin(), appLayoutIndex.getMax());

	std::string _name = getLayoutName(appLayoutIndex.get());
	ofLogNotice(__FUNCTION__) << appLayoutIndex << ":" << _name;

	//std::string _label = APP_RELEASE_NAME;
	std::string _label = "";
	_label += "App Layout ";
	_label += " "; // spacing

	if (!bLayoutPresets[appLayoutIndex.get()])
	{
		for (int i = 0; i < bLayoutPresets.size(); i++) {
			bLayoutPresets[i].set(false);
		}
		bLayoutPresets[_index].set(true);
	}

	// Window title
	titleWindowLabel = _label + " " + _name;
	//titleWindowLabel = _label + " " + ofToString(appLayoutIndex.get()) + " " + _name;

	//// Apply window title
	//ofSetWindowTitle(titleWindowLabel);

	// ini
	ini_to_load_Str = _name;
	ini_to_load = ini_to_load_Str.c_str();

	// Group
	loadLayoutPresetGroup(ini_to_load_Str);

	ofLogNotice(__FUNCTION__) << "------------------------------------";
	ofLogNotice(__FUNCTION__) << "ini_to_load    : " << ini_to_load;
}

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::drawLayoutsPresets() // That's the window tittled as "Layout"
{
	flags_wPr = ImGuiWindowFlags_None;
	flags_wPr += ImGuiWindowFlags_NoSavedSettings;

	if (bAutoResizePresets) flags_wPr += ImGuiWindowFlags_AlwaysAutoResize;

	//--

	/*
	//// Viewport Center
	//// is excluded from .ini
	//float xw, yw, ww, hw;
	//{
	//	if (bModeForced)
	//	{
	//		// Forced inside a free viewport
	//		//// Upper left
	//		//glm::vec2 p = rectangle_Central_MAX.getTopLeft() + glm::vec2(-1, -1);
	//		// Center upper left
	//		int _pad = 10;
	//		int _xx = rectangle_Central_MAX.getTopLeft().x + _pad;
	//		int _yy = rectangle_Central_MAX.getTopLeft().y + rectangle_Central_MAX.getHeight() / 2 - hw / 2;
	//		glm::vec2 p = glm::vec2(_xx, _yy);
	//		// Shape
	//		xw = p.x;
	//		yw = p.y;
	//		flagCond = ImGuiCond_Always;
	//		ImGui::SetNextWindowPos(ofVec2f(xw, yw), flagCond);
	//		//ImGui::SetNextWindowSize(ofVec2f(ww, hw), flagCond); // exclude shape
	//	}
	//}
	*/

	ImGuiCond prCond = ImGuiCond_None;
	prCond += ImGuiCond_Appearing;

	if (bResetWindowPresets)
	{
		bResetWindowPresets = false;

		const int i = 0;
		ofRectangle r = rectangles_Windows[i];
		r.setWidth(100);
		r.setHeight(100);
		rectangles_Windows[i].set(r);
		prCond = ImGuiCond_Always;

		// workflow
		bAutoResizePresets = true;
		bMinimizePresets = true;
		bGui_LayoutsManager = false;
	}

	//----

	const int i = 0;
	ImGui::SetNextWindowPos(ImVec2(rectangles_Windows[i].get().getX(), rectangles_Windows[i].get().getY()), prCond);
	ImGui::SetNextWindowSize(ImVec2(rectangles_Windows[i].get().getWidth(), rectangles_Windows[i].get().getHeight()), prCond);

	//-

	if (beginWindow(bGui_LayoutsPresets, flags_wPr))
	{
		float _h = 2 * ofxImGuiSurfing::getWidgetsHeightUnit();
		float _w1 = ofxImGuiSurfing::getWidgetsWidth(1);
		float _w2 = ofxImGuiSurfing::getWidgetsWidth(2);
		float _w = _w2;

		//--

		const int i = 0;
		rectangles_Windows[i].setWithoutEventNotifications(ofRectangle(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight()));

		//--

		// 1. Minimize (global)

		//ofxImGuiSurfing::AddToggleRounded(bMinimizePresets);
		this->Add(bMinimizePresets, OFX_IM_TOGGLE_BUTTON_ROUNDED_SMALL);

		this->AddSpacingSeparated();

		//--

		// 2. The 4 Preset Toggles

		this->AddLabelBig("Presets");

		const int NUM_WIDGETS = bLayoutPresets.size() + 4;
		for (int i = 0; i < bLayoutPresets.size(); i++)
		{
			ofxImGuiSurfing::AddBigToggle(bLayoutPresets[i], _w2, _h);
			if (i % 2 == 0) ImGui::SameLine();//two toggles per row
		}

		//// Autosave/Save when minimized
		//if (bMinimizePresets)
		//{
		//	// Autosave
		//	ofxImGuiSurfing::AddBigToggle(bAutoSave_Layout, _w1, 0.75 * _h, true);
		//	//ofxImGuiSurfing::AddBigToggle(bAutoSave_Layout, _w1, 0.75 * _h);
		//	// Save Button
		//	if (!bAutoSave_Layout.get())
		//	{
		//		ImGui::PushID("##SaveLayout0");
		//		if (ImGui::Button("Save", ImVec2(_w1, 0.5 * _h)))
		//		{
		//			saveAppLayout(appLayoutIndex.get());
		//		}
		//		ImGui::PopID();
		//	}
		//}

		this->AddSpacingSeparated();

		//--

		if (bLayoutPresets.size() % 2 == 0) _w = _w1;//full wdith when using only one preset

		//--

		// Panels & Manager

		ofxImGuiSurfing::AddBigToggle(bGui_LayoutsPanels, _w1, (bMinimizePresets ? _h * 0.75f : _h));
		//ofxImGuiSurfing::AddBigToggle(bGui_LayoutsPanels, _w1, (bMinimizePresets ? _h / 2 : _h));
		if (!bMinimizePresets) ofxImGuiSurfing::AddBigToggle(bGui_LayoutsManager, _w1, (bMinimizePresets ? _h / 2 : _h));

		//--

		if (!bMinimizePresets)
		{
			this->AddSpacingSeparated();

			// Autosave

			ofxImGuiSurfing::AddBigToggle(bAutoSave_Layout, _w1, 0.75 * _h, true);

			// Save Button

			if (!bAutoSave_Layout.get())
			{
				ImGui::PushID("##SaveLayout");
				if (ImGui::Button("Save", ImVec2(_w1, 0.5 * _h)))
				{
					saveAppLayout(appLayoutIndex.get());
				}
				ImGui::PopID();
			}

			this->AddSpacingSeparated();
			this->AddSpacing();
		}

		//-

		if (!bMinimizePresets)
		{
			// Organizer panel visible toggle

			//this->Add(bGui_WindowsAlignHelpers, OFX_IM_TOGGLE_ROUNDED); // Aligners
			this->Add(this->getWindowsSpecialsGuiToggle(), OFX_IM_TOGGLE_ROUNDED); // Organizer
			this->Add(this->getWindowsSpecialEnablerLinker(), OFX_IM_TOGGLE_ROUNDED); // Organizer

			this->AddSpacingSeparated();

			//--

			this->Add(bKeys, OFX_IM_TOGGLE_ROUNDED);

			//--

			// Help

			this->Add(bHelp, OFX_IM_TOGGLE_BUTTON_ROUNDED_SMALL);
			this->Add(bHelpInternal, OFX_IM_TOGGLE_BUTTON_ROUNDED_SMALL);

			//--

			// Reset Window//TODO: not fully implemented
			//if (ofxImGuiSurfing::AddButtonMini(bResetWindowPresets))
			if (this->Add(bResetWindowPresets, OFX_IM_TOGGLE_BUTTON_ROUNDED_SMALL))
			{
				bResetWindowPresets = true;
			}

			//--

			// Auto resize 

			this->Add(bAutoResizePresets, OFX_IM_TOGGLE_BUTTON_ROUNDED_SMALL);

			// Extra

			this->AddSpacingSeparated();
			this->Add(bGui_LayoutsExtra, OFX_IM_TOGGLE_BUTTON_ROUNDED);

			//ofxImGuiSurfing::AddToggleRoundedButton(bAutoResizePresets);
			//ofxImGuiSurfing::AddToggleRoundedButton(bGui_LayoutsExtra);
		}

		this->endWindow();
	}
}

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::drawLayoutsExtra()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
	window_flags |= ImGuiWindowFlags_NoTitleBar;//hide header

	if (bAutoResize) window_flags |= ImGuiWindowFlags_AlwaysAutoResize;

	//--

	bool bMin = false; // hide load buttons to simplify
	float max = (bMin ? 150 : 175);

	//----

	int _pad = PADDING_PANELS;

	ImGuiCond _flagc;
	_flagc = ImGuiCond_Always;

	const int i = 0;
	ImGui::SetNextWindowPos(ofVec2f(rectangles_Windows[i].get().getX(), rectangles_Windows[i].get().getY() + rectangles_Windows[i].get().getHeight() + _pad), _flagc);
	ImGui::SetNextWindowSize(ofVec2f(rectangles_Windows[i].get().getWidth(), -1), _flagc);

	//----

	if (beginWindow(bGui_LayoutsExtra, window_flags))
	{
		this->AddLabelBig("Extra Params");

		float _h = ofxImGuiSurfing::getWidgetsHeightUnit();
		float _w1 = ofxImGuiSurfing::getWidgetsWidth(1);
		float _w2 = ofxImGuiSurfing::getWidgetsWidth(2);

		if (!bMinimizePresets)
		{
			ofxImGuiSurfing::AddToggleRounded(bMenu);
			ofxImGuiSurfing::AddToggleRounded(bLog);
			//ofxImGuiSurfing::AddToggleRounded(bKeys);
		}

		//--

		if (!bMinimizePresets)
		{
			this->AddSpacingSeparated();
			this->AddLabelBig("Layout Presets");

			if (ImGui::CollapsingHeader("Manager", ImGuiWindowFlags_None))
			{
				ImVec2 bb{ (bMin ? _w1 : _w2), _h };

				//--

				int _id = 0;
				for (int i = 0; i < bLayoutPresets.size(); i++)
				{
					std::string _name = (bLayoutPresets[i].getName());

					ImGui::Text(_name.c_str());
					if (!bMin)
					{
						ImGui::PushID(_id++);
						if (ImGui::Button("Load", bb))
						{
							appLayoutIndex = i;
						}
						ImGui::PopID();

						ImGui::SameLine();
					}

					ImGui::PushID(_id++);

					if (ImGui::Button("Save", bb))
					{
						ini_to_save_Str = _name;
						ini_to_save = ini_to_save_Str.c_str();
					}

					ImGui::PopID();
				}

				//--

				this->AddSpacingSeparated();

				if (ImGui::Button("Reset", ImVec2(_w1, _h)))
				{
					//TODO: to trig an external function..
					if (bResetPtr != nullptr) {
						*bResetPtr = true;
					}

					// Toggle panels to true
					for (int i = 0; i < windowsSpecialsLayouts.size(); i++) {
						windowsSpecialsLayouts[i].bGui.set(true);
					}

					saveAppLayout((appLayoutIndex.get()));
				}
				this->AddTooltip("Reset all the Presets");

				if (ImGui::Button("Clear", ImVec2(_w1, _h)))
				{
					doRemoveDataFiles();
				}
				this->AddTooltip("Clear all the Presets");
			}
		}

		//-

		//if (!bMinimizePresets)
		if (specialsWindowsMode == IM_GUI_MODE_WINDOWS_SPECIAL_ORGANIZER)
		{
			if (ImGui::CollapsingHeader(nameWindowSpecialsPanel.c_str(), ImGuiWindowFlags_None))
			{
				windowsSpecialsOrganizer.drawWidgetsOrganizer(bMinimizePresets);
			}
		}

		//--

		/*
		//TODO: not storing new presets yet!
		//presets amounts can be defined on setupLayout(4);
		// create a new layout preset
		if (ImGui::Button("Create", ImVec2(_w100, _h)))
		{
			createLayoutPreset();
		}
		*/

		//--

		this->endWindow();
	}
}

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::Changed_Params(ofAbstractParameter& e)
{
	std::string name = e.getName();

	if (name != "position" &&
		name != "rect_Manager")
	{
		ofLogNotice(__FUNCTION__) << name << " : " << e;
	}

	if (0) {}

	//--

	// Update Help Info

	// Help internal
	else if (name == bHelpInternal.getName() && bHelpInternal)
	{
		buildHelpInfo();//recreate info
	}

	// Help App / global. To be handled externally
	else if (name == bHelp.getName())
	{
		buildHelpInfo();
	}

	// Debug
	else if (name == bDebug.getName())
	{
		buildHelpInfo();
	}

	// Extra
	else if (name == bExtra.getName())
	{
		buildHelpInfo();
	}

	// Log
	else if (name == bLog.getName())
	{
		buildHelpInfo();
	}

	// Minimize
	else if (name == bMinimize.getName())
	{
		buildHelpInfo();
	}
	else if (name == bMinimizePresets.getName())
	{
		buildHelpInfo();
	}

	//----

	// Skip callbacks when not using the Layout Presets Engine!
	if (!surfingImGuiMode == ofxImGuiSurfing::IM_GUI_MODE_INSTANTIATED_DOCKING)
		if (!bDockingLayoutPresetsEngine)
			return;

	//--

	// Gui layout
		else if (name == bGui_LayoutsPresets.getName())
		{
			// workflow
			if (!bGui_LayoutsPresets)
			{
				bGui_LayoutsExtra = false;
			}
		}

	//--

	// Reset 
	// This toggle/flag is "sended" to the parent scope (ofApp), to resets something in our apps.
	// Example: to resets the layout.
		else if (name == bReset.getName() && bReset.get())
		{
			bReset = false;

			if (bResetPtr != nullptr) {
				*bResetPtr = true;
			}
		}

		else if (name == bReset_Window.getName() && bReset_Window.get())
		{
			bReset_Window = false;
		}

	//--

	// Solo Panel

		else if (name == bSolo.getName() && bSolo.get())
		{
			// workflow
			appLayoutIndex = -1;

			// disable preset
			for (int i = 0; i < bLayoutPresets.size(); i++)
			{
				bLayoutPresets[i].setWithoutEventNotifications(false);
			}
		}

	//--

	// Layout preset index

		else if (name == appLayoutIndex.getName())
		{
			//appLayoutIndex = ofClamp(appLayoutIndex.get(), appLayoutIndex.getMin(), appLayoutIndex.getMax());

			if (appLayoutIndex != appLayoutIndex_PRE /*&& appLayoutIndex_PRE != -1*/) //changed
			{
				ofLogNotice(__FUNCTION__) << "Changed: " << appLayoutIndex;

				//-

				// 1. Auto save

				if (bAutoSave_Layout)
				{
					// workaround:
					// must save here bc usually we use the fallged on update save...
					// only once per cycle allowed this way.
					//force to ensure save bc update chain load and save below
					//saveAppLayout(AppLayouts(appLayoutIndex_PRE));
					std::string __ini_to_save_Str = getLayoutName(appLayoutIndex_PRE);

					if (__ini_to_save_Str != "-1")
					{
						const char* _iniSave = NULL;
						_iniSave = __ini_to_save_Str.c_str(); // flags to save on update

						if (_iniSave != "-1")
						{
							saveLayoutPreset(_iniSave);
						}
					}
				}

				appLayoutIndex_PRE = appLayoutIndex.get();
			}

			//-

			// Hide all modules / GUI toggles

			if (appLayoutIndex == -1)
			{
				for (int i = 0; i < windowsSpecialsLayouts.size(); i++)
				{
					windowsSpecialsLayouts[i].bGui.set(false);
				}
				return; // not required bc loadAppLayout will be skipped when -1
			}

			//-

			// 2. Load layout
			loadAppLayout(appLayoutIndex.get());
		}

	//-

	// Presets Selector exclusive toggles

	{
		bool bSomeTrue = false;
		for (int i = 0; i < bLayoutPresets.size(); i++)
		{
			if (name == bLayoutPresets[i].getName())
			{
				if (bLayoutPresets[i].get()) // true
				{
					// workflow
					if (bSolo.get()) bSolo = false;

					appLayoutIndex = i;
					bSomeTrue = true;
				}
				else { // false
					// avoid all false
					bool bAllFalse = true;
					for (int i = 0; i < bLayoutPresets.size(); i++)
					{
						if (bLayoutPresets[i].get()) {
							bAllFalse = false;
						}
					}
					if (bAllFalse)
					{
						// workflow A
						////force back to true if it's there's no other enabled..
						//bLayoutPresets[appLayoutIndex].set(true);

						// workflow B
						//set to -1
						appLayoutIndex = -1;
					}
				}
			}
		}
		if (bSomeTrue) {
			for (int i = 0; i < bLayoutPresets.size(); i++)
			{
				if (i != appLayoutIndex.get())
					bLayoutPresets[i].set(false);
			}
			return;
		}
	}

	//-

	// Solo Panels Selectors
	{
		bool bSomeTrue = false;
		for (int i = 0; i < windowsSpecialsLayouts.size(); i++)
		{
			int iTrue = -1;
			if (name == windowsSpecialsLayouts[i].bGui.getName() && windowsSpecialsLayouts[i].bGui.get()) {
				iTrue = i;
				if (bSolo.get())
				{
					for (int i = 0; i < windowsSpecialsLayouts.size(); i++)
					{
						if (iTrue != i && iTrue != -1) windowsSpecialsLayouts[i].bGui.set(false);
					}
					return;
				}
			}
		}
	}

	//--

	//// Rectangles
	//{
	//	for (int i = 0; i < rectangles_Windows.size(); i++)
	//	{
	//		if (name == rectangles_Windows[i].getName())
	//		{
	//		}
	//	}
	//}
}

//----

// Layout Preset Loaders / Savers

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::saveLayoutPreset(std::string path)
{
	saveLayoutImGuiIni(path);
	saveLayoutPresetGroup(path);
}

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::loadLayoutPreset(std::string path)
{
	loadLayoutImGuiIni(ini_to_load);
	loadLayoutPresetGroup(path);
}

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::saveLayoutImGuiIni(std::string path)
{
	ImGui::SaveIniSettingsToDisk(ofToDataPath(path_ImLayouts + path + ".ini", true).c_str());
}

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::loadLayoutImGuiIni(std::string path)
{
	ImGui::LoadIniSettingsFromDisk(ofToDataPath(path_ImLayouts + path + ".ini", true).c_str());
}

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::saveLayoutPresetGroup(std::string path)
{
	ofxImGuiSurfing::saveGroup(params_Layouts, path_ImLayouts + path + ".json");
}

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::loadLayoutPresetGroup(std::string path)
{
	ofxImGuiSurfing::loadGroup(params_Layouts, path_ImLayouts + path + ".json");
}

//----

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::createLayoutPreset(std::string namePreset)
{
	std::string n;
	//int i = bLayoutPresets.size();
	int i = bLayoutPresets.size() + 1;

	if (namePreset == "-1") n = "P" + ofToString(i);
	else n = namePreset;

	ofParameter<bool> _b = ofParameter<bool>{ n, false };
	bLayoutPresets.push_back(_b);
	appLayoutIndex.setMax(bLayoutPresets.size() - 1);
	params_LayoutPresetsStates.add(bLayoutPresets[i - 1]);
	//params_LayoutPresetsStates.add(bLayoutPresets[i]);
}

//----

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::drawLayoutsPanels()
{
	flags_wPanels = ImGuiWindowFlags_None;
	flags_wPanels += ImGuiWindowFlags_NoSavedSettings;
	if (bAutoResizePanels) flags_wPanels += ImGuiWindowFlags_AlwaysAutoResize;

	//--

	static bool bLandscape;
	// Docking mode ignores these constraints...
#ifdef OFX_IMGUI_CONSTRAIT_WINDOW_SHAPE
	// Landscape
	if (bLandscape) ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(700, 150)); // ?
	// Portrait
	else ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(105, 300));
#endif

	//--

	ImGuiCond pnCond = ImGuiCond_None;
	pnCond += ImGuiCond_Appearing;

	if (bResetWindowPanels)
	{
		bResetWindowPanels = false;

		const int i = 1;
		ofRectangle r = rectangles_Windows[i];
		r.setWidth(150);
		r.setHeight(150);
		rectangles_Windows[i].set(r);
		pnCond = ImGuiCond_Always;
	}

	//-

	const int i = 1;
	ImGui::SetNextWindowPos(ofVec2f(rectangles_Windows[i].get().getX(), rectangles_Windows[i].get().getY()), pnCond);
	ImGui::SetNextWindowSize(ofVec2f(rectangles_Windows[i].get().getWidth(), rectangles_Windows[i].get().getHeight()), pnCond);

	if (beginWindow(bGui_LayoutsPanels, flags_wPanels))
	{
		const int i = 1;
		rectangles_Windows[i].setWithoutEventNotifications(ofRectangle(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight()));

		bLandscape = false;
		float __w = ImGui::GetWindowWidth();
		float __h = ImGui::GetWindowHeight();
		if (__w > __h) bLandscape = true;

		//-

		const int NUM_WIDGETS = windowsSpecialsLayouts.size(); // expected num widgets
		const int NUM_WIDGETS_EXTRA_LANDSCAPE = 5;
		//const int NUM_WIDGETS_EXTRA_LANDSCAPE = 6; // with autoresize

		float _spcx = ImGui::GetStyle().ItemSpacing.x;
		float _spcy = ImGui::GetStyle().ItemSpacing.y;
		float _h100 = ImGui::GetContentRegionAvail().y;

		float _w;
		float _h;
		float _hWid;

		// A. Landscape

		if (bLandscape)
		{
			//_w = ofxImGuiSurfing::getWidgetsWidth(NUM_WIDGETS + 2);
			const int amntColumns = NUM_WIDGETS + 2;
			float __w100 = ImGui::GetContentRegionAvail().x - (3 * _spcx); // remove extra columns x spacing added!
			_w = (__w100 - _spcx * (amntColumns - 1)) / amntColumns;

			_h = _h100 - _spcy;
			_hWid = (_h - _spcy) / 2;
			//_hWid = (_h - _spcy) / 3; // with autoResize
		}

		// B. Portrait

		else
		{
			_hWid = ofxImGuiSurfing::getWidgetsHeightRelative();
			float _hTotal = _h100 - (_hWid * NUM_WIDGETS_EXTRA_LANDSCAPE + (NUM_WIDGETS_EXTRA_LANDSCAPE + 1) * _spcy);
			_w = ofxImGuiSurfing::getWidgetsWidth();
			_h = _hTotal / NUM_WIDGETS - _spcy;
		}

		// Landscape

		if (bLandscape)
		{
			ImGui::Columns(3);
			ImGui::SetColumnWidth(0, _spcx + (_w + _spcx) * NUM_WIDGETS);
		}

		//-

		// 1. Populate all toggles

		for (int i = 0; i < windowsSpecialsLayouts.size(); i++)
		{
			ofxImGuiSurfing::AddBigToggle(windowsSpecialsLayouts[i].bGui, _w, _h);
			if (bLandscape) ImGui::SameLine();
		}

		//-

		float _w100;
		float _w50;

		// Landscape

		if (bLandscape)
		{
			ImGui::SameLine();
			_w100 = _w;
			_w50 = (_w - _spcx) / 2.0f;
		}

		// Portrait

		else
		{
			this->AddSpacing();
			ImGui::Separator();
			this->AddSpacing();
			_w100 = ofxImGuiSurfing::getWidgetsWidth(1);
			_w50 = ofxImGuiSurfing::getWidgetsWidth(2);
		}

		// Landscape

		if (bLandscape) {
			ImGui::NextColumn();
			ImGui::SetColumnWidth(1, _w100 + 2 * _spcx);
		}

		//-

		// 2. Extra widgets

		if (ImGui::Button("All", ImVec2(_w50, _hWid)))
		{
			// workflow
			if (bSolo) bSolo.set(false);

			bool b = true;
			setShowAllPanels(b);

			//for (int i = 0; i < windowsSpecialsLayouts.size(); i++)
			//{
			//	windowsSpecialsLayouts[i].bGui.set(b);
			//}
		}
		ImGui::SameLine();
		if (ImGui::Button("None", ImVec2(_w50, _hWid)))
		{
			bool b = false;
			setShowAllPanels(b);

			//for (int i = 0; i < windowsSpecialsLayouts.size(); i++)
			//{
			//	windowsSpecialsLayouts[i].bGui.set(b);
			//}
		}

		ofxImGuiSurfing::AddBigToggle(bSolo, _w100, _hWid, true);

		//-

		// 3. Panels Toggles

		// Landscape
		if (bLandscape)
		{
			ImGui::NextColumn();
			ImGui::SetColumnWidth(2, _w100 + _spcx + _spcx);
		}
		// Portrait
		else
		{
			this->AddSpacing();
			ImGui::Separator();
			this->AddSpacing();
		}

		// Layout
		float _hUnit = ofxImGuiSurfing::getWidgetsHeightRelative();
		//ofxImGuiSurfing::AddBigToggle(bGui_LayoutsPresets, _w100, _hWid, false);
		ofxImGuiSurfing::AddBigToggle(bGui_LayoutsPresets, _w100, (bMinimizePresets ? ofxImGuiSurfing::getPanelHeight() : _hWid), false);

		if (!bMinimizePresets) ofxImGuiSurfing::AddBigToggle(bGui_LayoutsManager, _w100, _hWid, false);

		//ofxImGuiSurfing::AddToggleRoundedButton(bAutoResizePanels);

		// Landscape
		if (bLandscape)
		{
			ImGui::Columns();
		}

		this->endWindow();
	}

	//-

#ifdef OFX_IMGUI_CONSTRAIT_WINDOW_SHAPE
	ImGui::PopStyleVar();
#endif
}

//--

// Keys
//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::keyPressed(ofKeyEventArgs& eventArgs)
{
	if (!bKeys) return;

	const int& key = eventArgs.key;
	ofLogNotice(__FUNCTION__) << (char)key << " [" << key << "]";

	// Modifiers
	bool mod_COMMAND = eventArgs.hasModifier(OF_KEY_COMMAND);
	bool mod_CONTROL = eventArgs.hasModifier(OF_KEY_CONTROL);
	bool mod_ALT = eventArgs.hasModifier(OF_KEY_ALT);
	bool mod_SHIFT = eventArgs.hasModifier(OF_KEY_SHIFT);

	// Log
	if (key != OF_KEY_SHIFT && !mod_COMMAND && !mod_CONTROL && !mod_ALT && !mod_SHIFT)
	{
		std::string ss = ofToString((char)key);
		log.AddText(ss);
	}

	//-

	if (0)
	{
		ofLogNotice(__FUNCTION__) << "mod_COMMAND : " << (mod_COMMAND ? "ON" : "OFF");
		ofLogNotice(__FUNCTION__) << "mod_CONTROL : " << (mod_CONTROL ? "ON" : "OFF");
		ofLogNotice(__FUNCTION__) << "mod_ALT     : " << (mod_ALT ? "ON" : "OFF");
		ofLogNotice(__FUNCTION__) << "mod_SHIFT   : " << (mod_SHIFT ? "ON" : "OFF");
	}

	//----

	// Help 
	if (key == 'H')
	{
		bHelp = !bHelp;
	}
	// Help Internal
	else if (key == 'I')
	{
		bHelpInternal = !bHelpInternal;
	}

	// Minimize
	else if (key == '`')
	{
		bMinimize = !bMinimize;
	}

	// Extra 
	else if (key == 'E' && !mod_CONTROL)
	{
		bExtra = !bExtra;
	}

	// Debug 
	else if (key == 'D' && !mod_CONTROL)
	{
		bDebug = !bDebug;
	}

	// Log 
	else if (key == 'L' && !mod_CONTROL)
	{
		bLog = !bLog;
	}

	//--

	// Layout Presets Engine
	{
		if (!bDockingLayoutPresetsEngine && !bUseLayoutPresetsManager) return;//skip is not enabled!

		//--

		switch (key)
		{
		case OF_KEY_F1: bLayoutPresets[0] = !bLayoutPresets[0]; break;
		case OF_KEY_F2: bLayoutPresets[1] = !bLayoutPresets[1]; break;
		case OF_KEY_F3: bLayoutPresets[2] = !bLayoutPresets[2]; break;
		case OF_KEY_F4: bLayoutPresets[3] = !bLayoutPresets[3]; break;

		default: break;
		}

		if (key == OF_KEY_F5 /*|| key == 'p'*/) // Presets
		{
			bGui_LayoutsPresets = !bGui_LayoutsPresets;
		}

		else if (key == OF_KEY_F6) // Panels
		{
			bGui_LayoutsPanels = !bGui_LayoutsPanels;
		}

		else if (key == OF_KEY_F7) // Manager
		{
			bGui_LayoutsManager = !bGui_LayoutsManager;
		}

		else if (key == OF_KEY_F8) // Extra
		{
			bGui_LayoutsExtra = !bGui_LayoutsExtra;
		}

		else if (key == OF_KEY_F9) // Minimize
		{
			bMinimizePresets = !bMinimizePresets;
		}

		//--

		// Solo
		else if (key == 's' && mod_CONTROL)
		{
			bSolo = !bSolo;
		}

		//// Unlock Dock 
		//else if (key == 'l')
		//{
		//	bModeLock1 = !bModeLock1;
		//}

		//else if (key == 'L')
		//{
		//	bModeLockControls = !bModeLockControls;
		//}

		//----

		//// Layout Modes

		//else if (key == OF_KEY_TAB && !mod_CONTROL)
		//{
		//	if (appLayoutIndex > appLayoutIndex.getMin()) appLayoutIndex++;
		//	else if (appLayoutIndex == appLayoutIndex.getMin()) appLayoutIndex = appLayoutIndex.getMax();
		//	//if (appLayoutIndex < 3) loadAppLayout(AppLayouts(appLayoutIndex + 1));
		//	//else if (appLayoutIndex == 3) loadAppLayout(AppLayouts(0));
		//}
	}

	//--

	//// Key Enabler
	//if (key == 'k')
	//{
	//	bKeys = !bKeys;
	//	ofLogNotice(__FUNCTION__) << "KEYS: " << (bKeys ? "ON" : "OFF");
	//	if (!bKeys)
	//	{
	//		ofLogNotice(__FUNCTION__) << "ALL KEYS DISABLED. PRESS 'k' TO ENABLE GAIN!";
	//	}
	//	else
	//	{
	//		ofLogNotice(__FUNCTION__) << "KEYS ENABLED BACK";
	//	}
	//}
}

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::keyReleased(ofKeyEventArgs& eventArgs)
{
	const int& key = eventArgs.key;
	ofLogNotice(__FUNCTION__) << (char)key << " [" << key << "]";

	bool mod_COMMAND = eventArgs.hasModifier(OF_KEY_COMMAND);
	bool mod_CONTROL = eventArgs.hasModifier(OF_KEY_CONTROL);
	bool mod_ALT = eventArgs.hasModifier(OF_KEY_ALT);
	bool mod_SHIFT = eventArgs.hasModifier(OF_KEY_SHIFT);
}

//--

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::drawMenuDocked()
{
	static bool opt_fullscreen = true;
	static bool* p_open = NULL;
	static bool opt_exit = false;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	//-

	// Menu bar

	// This is not operative. just for testing menus!

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			// Disabling fullscreen would allow the window to be moved to the front of other windows,
			// which we can't undo at the moment without finer window depth/z control.
			ofWindowMode windowMode = ofGetCurrentWindow()->getWindowMode();
			if (windowMode == OF_WINDOW) opt_fullscreen = false;
			else if (windowMode == OF_FULLSCREEN) opt_fullscreen = true;
			else if (windowMode == OF_GAME_MODE) opt_fullscreen = false;

			if (ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen))
			{
				ofSetFullscreen(opt_fullscreen);
			}

			this->AddSpacingSeparated();

			if (ImGui::MenuItem("Exit", NULL, &opt_exit))
			{
				ofExit();
				//*opt_exit = false;
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Copy", NULL)) {}
			if (ImGui::MenuItem("Paste", NULL)) {}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Layouts"))
		{
			for (int i = 0; i < bLayoutPresets.size(); i++)
			{
				if (ImGui::MenuItem(bLayoutPresets[i].getName().c_str(), "", (bool*)&bLayoutPresets[i].get()))
				{
					bLayoutPresets[i] = bLayoutPresets[i]; // to trig
				}
			}
			//this->AddSpacingSeparated();
			//if (ImGui::MenuItem("All", NULL)) { setShowAllPanels(true); }
			//if (ImGui::MenuItem("None", NULL)) { setShowAllPanels(false); }
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Docking"))
		{
			if (ImGui::MenuItem("WARNING", "")) {};
			ofxImGuiSurfing::AddTooltip2("Don't pay attention for this! \nThis is not operative here. \nJust for testing menus!\nPotential CRASH!");

			dockspace_flags = ImGui::GetIO().ConfigFlags;

			if (ImGui::MenuItem("Flag: NoSplit", "", (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoSplit; }
			if (ImGui::MenuItem("Flag: NoResize", "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoResize; }
			if (ImGui::MenuItem("Flag: NoDockingInCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingInCentralNode) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingInCentralNode; }
			if (ImGui::MenuItem("Flag: AutoHideTabBar", "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar; }
			if (ImGui::MenuItem("Flag: PassthruCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, opt_fullscreen)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }
			//if (ImGui::MenuItem("Flag: ConfigDockingWithShift", "", (dockspace_flags & ImGuiDockNodeFlags_) != 0, opt_fullscreen)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }
			//this->AddSpacingSeparated();

			ImGui::GetIO().ConfigFlags = dockspace_flags;
			//ImGui::GetIO().ConfigDockingWithShift = true;

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("About"))
		{
			ofxImGuiSurfing::AddTooltipHelp(
				"WARNING\nDon't pay attention for this text! \nThis is not operative here. \nJust for testing menus!" "\n\n"
				"When docking is enabled, you can ALWAYS dock MOST window into another! Try it now!" "\n"
				"- Drag from window title bar or their tab to dock/undock." "\n"
				"- Drag from window menu button (upper-left button) to undock an entire node (all windows)." "\n"
				"- Hold SHIFT to enable dragging docking." "\n"
				"This demo app has nothing to do with it!" "\n\n"
				"This demo app only demonstrate the use of ImGui::DockSpace() which allows you to manually create a docking node _within_ another window. This is useful so you can decorate your main application window (e.g. with a menu bar)." "\n\n"
				"ImGui::DockSpace() comes with one hard constraint: it needs to be submitted _before_ any window which may be docked into it. Therefore, if you use a dock spot as the central point of your application, you'll probably want it to be part of the very first window you are submitting to imgui every frame." "\n\n"
				"(NB: because of this constraint, the implicit \"Debug\" window can not be docked into an explicit DockSpace() node, because that window is submitted as part of the NewFrame() call. An easy workaround is that you can create your own implicit \"Debug##2\" window after calling DockSpace() and leave it in the window stack for anyone to use.)"
			);

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}
}

//--

//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::drawMenu()
{
	static bool opt_fullscreen = true;
	static bool* p_open = NULL;
	static bool opt_exit = false;

	//-

	// Menu bar

	// This is not operative. just for testing menus!

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			// Disabling fullscreen would allow the window to be moved to the front of other windows,
			// which we can't undo at the moment without finer window depth/z control.
			ofWindowMode windowMode = ofGetCurrentWindow()->getWindowMode();
			if (windowMode == OF_WINDOW) opt_fullscreen = false;
			else if (windowMode == OF_FULLSCREEN) opt_fullscreen = true;
			else if (windowMode == OF_GAME_MODE) opt_fullscreen = false;

			if (ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen))
			{
				ofSetFullscreen(opt_fullscreen);
			}

			this->AddSpacingSeparated();

			if (ImGui::MenuItem("Exit", NULL, &opt_exit))
			{
				ofExit();
				//*opt_exit = false;
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Copy", NULL)) {}
			if (ImGui::MenuItem("Paste", NULL)) {}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("About"))
		{
			ofxImGuiSurfing::AddTooltipHelp(
				"This is not operative here. Just for testing menus!" "\n\n"
				"When docking is enabled, you can ALWAYS dock MOST window into another! Try it now!" "\n"
				"- Drag from window title bar or their tab to dock/undock." "\n"
				"- Drag from window menu button (upper-left button) to undock an entire node (all windows)." "\n"
				"- Hold SHIFT to enable dragging docking." "\n"
				"This demo app has nothing to do with it!" "\n\n"
				"This demo app only demonstrate the use of ImGui::DockSpace() which allows you to manually create a docking node _within_ another window. This is useful so you can decorate your main application window (e.g. with a menu bar)." "\n\n"
				"ImGui::DockSpace() comes with one hard constraint: it needs to be submitted _before_ any window which may be docked into it. Therefore, if you use a dock spot as the central point of your application, you'll probably want it to be part of the very first window you are submitting to imgui every frame." "\n\n"
				"(NB: because of this constraint, the implicit \"Debug\" window can not be docked into an explicit DockSpace() node, because that window is submitted as part of the NewFrame() call. An easy workaround is that you can create your own implicit \"Debug##2\" window after calling DockSpace() and leave it in the window stack for anyone to use.)"
			);

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}
}


//----

/*
//TODO:
//--------------------------------------------------------------
void ofxSurfing_ImGui_Manager::drawSpecialWindowsPanel()
{
	if (ImGui::TreeNode("Special Windows"))
	{
		//ImGui::Text("Special Windows");

		//ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
		//if (bAutoResizePanels) window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
		//if (beginWindow("Specials", NULL, window_flags))
		{
			const int NUM_WIDGETS = windowsSpecialsLayouts.size(); // expected num widgets

			float _spcx = ImGui::GetStyle().ItemSpacing.x;
			float _spcy = ImGui::GetStyle().ItemSpacing.y;
			//float _h100 = ImGui::GetContentRegionAvail().y;

			// 1. populate all toggles

			const int _amnt = 1;

			float _w = ofxImGuiSurfing::getWidgetsWidth(_amnt);
			float _h = 1 * ofxImGuiSurfing::getWidgetsHeightRelative();
			//float _w = ofxImGuiSurfing::getWidgetsWidth(windowsSpecialsLayouts.size());

			for (int i = 0; i < NUM_WIDGETS; i++)
			{
				if (i > windowsSpecialsLayouts.size() - 1) continue;

				ofxImGuiSurfing::AddBigToggle(windowsSpecialsLayouts[i].bGui, _w, _h);

				//if ((i + 1) % _amnt != 0 && i < NUM_WIDGETS - 1) ImGui::SameLine();
			}
		}

		ImGui::TreePop();
	}
}*/

//--

// Special behaviour to control windows

//// Free layout
//else if (name == bModeFree.getName())
//{
//	if (bModeFree.get())
//	{
//		flagsWindowsModeFreeStore = ImGuiWindowFlags_NoSavedSettings;
//	}
//	else
//	{
//		flagsWindowsModeFreeStore = ImGuiWindowFlags_None;
//	}
//}

//-

//// Lock layout
//else if (name == bModeLock1.getName())
//{
//	if (!bModeLock1)
//	{
//		flagsWindowsLocked1 = ImGuiWindowFlags_None;
//	}
//	else
//	{
//		flagsWindowsLocked1 = ImGuiWindowFlags_NoMove;
//		flagsWindowsLocked1 |= ImGuiWindowFlags_NoSavedSettings;
//		//flagsWindowsLocked1 |= ImGuiWindowFlags_NoResize;
//		//flagsWindowsLocked1 |= ImGuiWindowFlags_NoTitleBar;
//		//flagsWindowsLocked1 |= ImGuiWindowFlags_NoCollapse;
//		//flagsWindowsLocked1 |= ImGuiWindowFlags_NoDecoration;
//		//flagsWindowsLocked1 |= ImGuiWindowFlags_NoBackground;
//		//flagsWindowsLocked1 |= ImGuiDockNodeFlags_AutoHideTabBar;
//		//flagsWindowsLocked1 |= ImGuiDockNodeFlags_NoTabBar;
//		//flagsWindowsLocked1 |= ImGuiDockNodeFlags_NoCloseButton;
//	}
//}
//else if (name == bModeLockControls.getName())
//{
//	if (!bModeLockControls)
//	{
//		flagsWindowsLocked2 = ImGuiWindowFlags_None;
//	}
//	else
//	{
//		flagsWindowsLocked2 = ImGuiWindowFlags_NoMove;
//		//flagsWindowsLocked2 |= ImGuiWindowFlags_NoResize;
//		//flagsWindowsLocked2 |= ImGuiWindowFlags_NoTitleBar;
//		//flagsWindowsLocked2 |= ImGuiWindowFlags_NoCollapse;
//		//flagsWindowsLocked2 |= ImGuiWindowFlags_NoDecoration;
//		//flagsWindowsLocked2 |= ImGuiWindowFlags_NoBackground;
//		//flagsWindowsLocked2 |= ImGuiDockNodeFlags_AutoHideTabBar;
//		//flagsWindowsLocked2 |= ImGuiDockNodeFlags_NoTabBar;
//		//flagsWindowsLocked2 |= ImGuiDockNodeFlags_NoCloseButton;
//	}
//}