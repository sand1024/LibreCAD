### Other UI Improvements

Several small improvements for UI are included. 

#### "All Tools" CAD panel (or tools matrix). 

This panel is enabled Application Properties dialog, and if enabled, is replaces all CAD panels by one that includes all tools. 

The main reason for this is better support for CAD panels on small screen resolutions (it simply tookl less screen space). 

Widget Options dialog was also adjusted to support settings for this panel. 

#### Separators in Custom Toolbars and Custom Menus 

Custom Toolbar Creator and Custom Menu Creator now supports adding separators.  

#### Hideable Main menu. 

Top level menu of the application now may be hidden/shown - with appropriate action (default shortcut is **F12**). 

#### Workspace minor improvements
Workspace state now includes the state of main menu and fullscreen. 


#### Refactorings 
1) Unified workflow for document modification via LC_UndoSection. Consistent modification of the document and undo state, simplification of actions.
2) Selection changes - via RS_Document as primary interface for this, selected objects stored in container so no need to iterate over the entire document content. 
3) Selection modification listener
4) Selection mode (additive or exclusive, like AutoCAD PICKADD System variable)
5) Copy-paste fixes

### Minor changes
- Undoable changes of entities in layers via operations performed in Layers Tree widget.
- Ability to perform entities selection without pressed mouse button (similar to AutoCAD rect selection)
- LibreCAD#2375 - added property to specify selection overlay linetype
