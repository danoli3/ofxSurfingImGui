ofxSurfingImGui
=============================

## Overview

An **ImGui** toolkit for **openFrameworks** projects.  

Setup and Layout Management, **ofParameter Helpers** and **ImGui Widgets** ported to **oF**, Docking Helpers with a Layout Presets Engine, and useful snippets.  

Some examples and testing projects are now moved to: [ofxSurfingImGuiExtra](https://github.com/moebiussurfing/ofxSurfingImGuiExtra)  

![image](/docs/1_Widgets_Sliders2.PNG?raw=true "image")  

## WORK IN PROGRESS!

```
NOT FINISHED. A BIT BUGGY YET.
API WILL CHANGE.  
SOME EXAMPLES CAN BE REDUNDANT OR INCOMPLETE  
OR NEEDING TO BE UPDATED TO API CHANGES.
```

<details>
  <summary>RELEVANT CHANGES vs official ofxImGui ofParameter/ImHelpers</summary>
  <p>

- Simplified and improved **oF Helpers** to use **ofParameters**. 
  * _ImHelpers.h_ has been rewritten to _ofxSurfing_ImGui_ofHelpers.h_.
  * Now the _ofParameter_ widgets, _Windows_ and _Group/Trees_ are more customizable. 
  * Removed all the old internal _Windows/Tree_, _WindowOpen/Settings_ and _GetUniqueName_ methods from **ofxImGui**. 
  * Currently using a _PushId()/PopID()_ approach on each widget.  
    
- **NEW: _Layout/Styles Management_.**
- **NEW: _Docking helpers with an Engine for Layout Presets_.**
    </p>
    </details>

## FEATURES

* Requires the _AWESOME_ @**Daandelange**'s [ofxImGui](https://github.com/Daandelange/ofxImGui/) fork: 
  - Why? 
    - Easy to update to future **NEW ImGui** versions.  
    - Multi context/instances: you can use several windows from different add-ons without colliding.  
* **ImGui Widgets**: 
  - Big Toggles and Buttons
  - Vertical and Horizontal Sliders
  - Range Sliders
  - Styled Knobs
  - Inactive, hidden, locked widgets
  - Show or hide labels and values
  - DearWidgets
  - Gradient Color Designer
  - Matrix Selector
  - Progress bars and waiting spinners
  - Files Browser
  - Curve Editors
  - Mouse Wheel control

* **Layout Manager** to speed up **windows creation** and responsive _auto-fit-widgets-width-to-window_ sizes.
* **Layout Presets Engine with Docking** and app sections handling.
* Included my **Dark Theme**.

## INDEX

### 1. [WIDGETS](#1-widgets-1)

### 2. [BASIC LAYOUT](#2-layout-1)

### 3. [TYPES ENGINE](#3-types-engine-1)

### 4. [DOCKING AND LAYOUT PRESETS ENGINE](#4-docking-and-layout-presets-engine-wip-1) [WIP]


<BR>

## 1. WIDGETS

Shows **ofParameter** helpers with full/half/third/quarter width Buttons, Toggles, DearWidgets, Range Sliders, Bezier Curve, Spinner/Progress Indicators, and **ofxImGui** standard parameters.  

<details>
  <summary>1_1_Widgets_DearWidgets</summary>
  <p>

Includes Range Sliders with **ofParameters**, responsive Button/Toggles and the AWESOME [DearWidgets](https://github.com/soufianekhiat/DearWidgets) from **@soufianekhiat**.  
![image](/docs/1_1_Widgets_DearWidgets.PNG?raw=true "image")  
  </p>
</details>

<details>
  <summary>1_2_Widgets_Curve</summary>
  <p>

Includes Bezier Curves and usable timed Tween/Easing functions.  
![image](/docs/1_2_Widgets_Curve.PNG?raw=true "image")  
  </p>
</details>

<details>
  <summary>1_3_Widgets_Spinner</summary>
  <p>

Includes waiting and progress spinners.  
![image](/docs/1_3_Widgets_Spinner.gif?raw=true "image")  
  </p>
</details>

<details>
  <summary>1_4_Widgets_Surfing</summary>
  <p>

Includes a **matrix button clicker selector** linked to an **ofParameter<int>** (aka preset index), small tooltips, spin clicker, and the awesome gradient engine from [@galloscript](https://twitter.com/galloscript) from his [Github Gist](https://gist.github.com/galloscript/8a5d179e432e062550972afcd1ecf112).  
![image](/docs/1_4_Widgets_Surfing.PNG?raw=true "image")  
  </p>
</details>

<details>
  <summary>1_5_FileBrowser</summary>
  <p>

![image](/docs/1_5_FileBrowser.PNG?raw=true "image")  
  </p>
</details>

<BR>

## 2. BASIC LAYOUT

Speed up **ofxImGui** instantiation (auto draw, viewport mode, fonts...), windows, and layouts.  
Includes **ofParameter** helpers, widget sizes, and extra widgets.  
_Notice that below at point **3. TYPES ENGINE**, there's a better NEW API!_ 

<details>
  <summary>Example Code</summary>
  <p>

  ![image](/docs/2_1_2_Layout_Basic.PNG?raw=true "image")  

ofApp.h

```.cpp
#include "ofxSurfingImGui.h"

ofxSurfing_ImGui_Manager guiManager;

ofParameter<bool> bGui{ "Show Gui", true };

ofParameter<bool> bEnable{ "Enable", true };
ofParameter<bool> b1{ "b1", false };
ofParameter<bool> b2{ "b2", false };
ofParameter<bool> b3{ "b3", false };
```

ofApp.cpp

```.c++
void ofApp::setup() 
{ 
    guiManager.setup(); 
    // Instantiates and configures all the required ofxImGui stuff inside:
    // Font, theme, autodraw, layout store/recall, multi context/instances, ofParams Helpers and other customizations.
}

void ofApp::draw() 
{ 
    guiManager.begin();
    if (bGui) // -> This param makes the close button functional
    {
        guiManager.beginWindow("Window", (bool *)&bGui.get(), ImGuiWindowFlags_None);
        {
            ofxImGuiSurfing::AddToggleRoundedButton(bEnable);
            if (bEnable)
            {
                // Precalculate common widgets sizes to fit current window, "to be responsive".
                float _w1 = ofxImGuiSurfing::getWidgetsWidth(1); // 1 widget full width
                float _w2 = ofxImGuiSurfing::getWidgetsWidth(2); // 2 widgets half width
                float _w3 = ofxImGuiSurfing::getWidgetsWidth(3); // 3 widgets third width
                float _w4 = ofxImGuiSurfing::getWidgetsWidth(4); // 4 widgets quarter width
                float _h = ofxImGuiSurfing::getWidgetsHeightRelative(); // one unit height relative to ImGui theme

                //-

                /* Draw RAW ImGui or SurfingWidgets with ofParameters */

                // One widget full with and theme height. The callback is handled by the param listeners.
                ofxImGuiSurfing::AddBigToggle(b1); 

                // Two widgets same line/row with the 50% of window panel width 
                if (ofxImGuiSurfing::AddBigButton(b2, _w2, _h)) {
                  // This acts as callback. 
                  // No parameter listener required.
                }
                ImGui::SameLine();
                if (ofxImGuiSurfing::AddBigButton(b3, _w2, _h)) {
                  // This acts as callback. 
                  // No parameter listener required.
                }

                // Or using raw ImGui
                // Three widgets and fit width in one line
                if (ImGui::Button("START", ImVec2(_w3, _h))) {}
                ImGui::SameLine();
                if (ImGui::Button("STOP", ImVec2(_w3, _h))) {}
                ImGui::SameLine();
                if (ImGui::Button("REPLAY", ImVec2(_w3, _h))) {}
            }
        }
        guiManager.endWindow();
    }
    guiManager.end();
}
```
</p>
</details>

<details>
  <summary>2_1_Layout_Basic</summary>
  <p>

![image](/docs/2_1_Layout_Basic.PNG?raw=true "image")  
  </p>
</details>

<details>
  <summary>2_0_Layout_ofParamaters</summary>
  <p>

Includes **ofParameter** and **ofParameterGroup** helpers and customize how groups are presented: collapsed/expanded, hidden header, **ImGui::Tree/ImGui::TreeEx** ...etc.  
[BROKEN]  
![image](/docs/2_0_Layout_ofParamaters.gif?raw=true "image")  
  </p>
</details>

<details>
  <summary>2_4_Layout_ThemeEditor</summary>
  <p>

This is a helper for tweaking your Themes: testings sizes, layout, and colors, and alternate fonts.  

Notice that you need to export the newly modified theme code through the clipboard and paste it to a new function/theme manually.  
There's not an automatic-fully-functional, save preset/load theme designer!  
![image](/docs/2_4_Layout_ThemeEditor.PNG?raw=true "image")  
  </p>
</details>

<BR>

## 3. TYPES ENGINE

One step forward for **ofHelpers**:  

* Fast **ofParameter**s widgets layout: responsive-auto_fit width, height, amount items per row, and different styles for the same types or even repeated parameters with different styles.  

* Also **ofParameterGroup** and their inside **ofParameters**, can be customized too with different **ImGui::Tree** settings, collapsed and nested.  

#### FEATURES

Instead of the **ofxImGui** behavior, where you can't change the param widget style, we will have:  
1. A more powerful **Layout Engine** to improve: "responsive" layouts and customized widgets.  
2. Draw each **ofParameter** type with different styles. 
3. Especially useful when **ofParameters** are into an **ofParameterGroup**.  
4. Queue settings for an **ofParameter** to define configurations to be applied when the widget is drawn when drawing a group. 
5. You can hide or exclude some params to be drawn, to disable the mouse interaction, or to draw his void spacing. 
6. You can customize how **ofParameterGroup** / **ImGui::Tree** are presented.

<details>
  <summary>USAGE CASES</summary>
  <p>

**CASE 1**:  
_Draw an **ofParameter<float>** as slider (default), drag number or/and +/- stepper box._  

**CASE 2**:  
_Draw an **ofParameter<bool>** as a check box (default), or as a big toggle button with custom dimensions._  

**CASE 3**:  
_You added an **ofParameter<bool>** inside an **ofParameterGroup**. Add a style for the type of widget. You want to customize how it will be drawn (instead of using the default style), but when the group is rendered._  
  </p>
</details>

<details>
  <summary>0_Intro_Examples/1_Widgets_Sliders</summary>
  <p>

![image](/docs/1_Widgets_Sliders.PNG?raw=true "image")  
  </p>
</details>

<details>
  <summary>0_Intro_Examples/1_Widgets_Sliders2</summary>
  <p>

![image](/docs/1_Widgets_Sliders2.PNG?raw=true "image")  
  </p>
</details>

<details>
  <summary>4_0_0_Layout_TypesEngine</summary>
  <p>

![image](/docs/4_0_0_Layout_TypesEngine.PNG?raw=true "image")  
  </p>
</details>

<details>
  <summary>4_0_1_Layout_TypesEngine</summary>
  <p>

![image](/docs/4_0_1_Layout_TypesEngine.PNG?raw=true "image")  
  </p>
</details>

<details>
  <summary>Code</summary>
  <p>

ofApp.h

```.cpp

```

ofApp.cpp

```.cpp

```
</p>
</details>

#### AVAILABLE CUSTOMIZATIONS:

    - Hide the parameter widget respecting the void space or not.
    - Make a parameter inactive disabling mouse interaction.  
    - Set an ImGui::SameLine() after the widget, to draw the next params at the same line.  
    - Add a final vertical spacing after the widget.  
    - Set the widget width to fit the panel width, passing the number of widgets per row/line.  
    - Queue styles for incoming parameters inside a group.  

#### API

```c++
void AddStyle(ofAbstractParameter& aparam, SurfingImGuiTypes type = OFX_IM_DEFAULT, bool bSameLine = false, int amtPerRow = 1, int spacing = -1);
bool Add(ofAbstractParameter& aparam, SurfingImGuiTypes type = OFX_IM_DEFAULT, bool bSameLine = false, int amtPerRow = 1, int spacing = -1);
void AddGroup(ofParameterGroup& group, ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None, SurfingImGuiTypesGroups typeGroup = OFX_IM_GROUP_DEFAULT);
```

<BR>

## 4. DOCKING AND LAYOUT PRESETS ENGINE [WIP]

Examples to improve the layout of Docking Spaces. 

* Learn to split docking spaces by code, or using mouse control by the user.
* Docking helpers. **Engine Manager** to create layout presets.
<details>
  <summary>3_0_Layout_Docking2</summary>
  <p>

#### LAYOUT PRESETS ENGINE

* Fast adding of windows to the **Gui Manager**.
* Auto populates **Control Panels** to handle layout presets.
* Cute workflow for Management.
* You can add extra parameters to the presets too. 

![image](/docs/3_0_Layout_Docking2.gif?raw=true "gif")  
  </p>
</details>

<details>
  <summary>3_0_Layout_Docking3</summary>
  <p>

This example shows how to populate many ImGui windows from different scopes on the same viewport.  
Uses different approaches: from ofApp, from an add-on/class, or with Surfing Layout tools as guiManager.  
Also useful to check viewport modes, docking merging windows or auto draw modes.  

![image](/docs/3_0_Layout_Docking3.PNG?raw=true "png")  
  </p>
</details>

<details>
  <summary>3_1_Layout_ImTools1</summary>
  <p>

Uses [ImTools](https://github.com/aiekick/ImTools) from **@aiekick**: "_ It's a class for manage docking panes in an easy way, display (panes, menu, pane dialog), load/save, auto layout, etc..._". **WIP** porting to use in my oF projects.  

An alternative to my Docking Layout Engine but without presets and with more development required.  
Nice to learn about ImGui Docking.  

![image](/docs/3_1_Layout_ImTools1.PNG?raw=true "image")  
  </p>
</details>

<BR>

## Dependencies
[ofxImGui](https://github.com/Daandelange/ofxImGui/) : AWESOME FORK from @**Daandelange**  
[ofxSurfingHelpers](https://github.com/moebiussurfing/ofxSurfingHelpers)  

[ofxWindowApp](https://github.com/moebiussurfing/ofxWindowApp) : For some examples only  

**/data/** folder contains some font files that we use in the examples.  

## Tested Systems
- **Windows10** / **VS2017** / **OF ~0.11**

## Author
An addon by **@moebiusSurfing**  
*( ManuMolina ) 2021-2022*  

### Thanks
_All source snippets from other authors are linked into header files. Thanks!_

### License
**MIT License**

<BR>

<details>
  <summary>Some Reference Links</summary>
  <p>

https://github.com/altschuler/imgui-knobs -> Cute Knobs  
https://github.com/HankiDesign/awesome-dear-imgui -> Collected widgets  
https://github.com/soufianekhiat/DearWidgets -> Cute widgets already included  
https://github.com/yumataesu/ImGui_Widgets -> oF ready widgets  
https://github.com/aiekick/ImTools/tree/main/LayoutManager -> Layout helpers  
https://github.com/Organic-Code/ImTerm -> Interesting terminal to look into  
https://github.com/d3cod3/ofxVisualProgramming -> oF node patched system for inspiration  
https://github.com/d3cod3/Mosaic -> oF node patched system for inspiration  
https://github.com/mnesarco/imgui_sugar/blob/main/imgui_sugar.hpp -> Useful ImGui style macros  
https://github.com/njazz/AutomationCurveEditor -> Curve editor for param automations  
https://github.com/leiradel/ImGuiAl -> Some widgets    

  </p>
</details>


<details>
  <summary>TODO</summary>
  <p>

* macOS testing and fixing. Any help on this is appreciated!  
* Split repo to simplify current examples. Move extra examples to new repo.
* Fix Bugs on all the ofParameter Helpers/Styles/Docking sections.
* Create some selected examples to simplify learning.
* Add more ImGui raw widgets/add-ons with examples.
* Convert some more widgets to ofParameters.
  </p>
</details>
