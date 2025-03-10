---- Translated by Unovamata ----

  Please head to the original developer's page,
  this piece of software has been an amazing resource for
  the process of creating this randomizer and mirror randomizing.
  To this day this developer maintains and creates tools for
  romhacking other games so it would be appreciated to visit
  their website as a token of appreciation. 
  Thank you for all your hard work!
  
  I only translated and uploaded this to Github for archival purposes,
  ReflexViewer was almost impossible to find in the web as there are
  multiple links down that lead nowhere. You have to be very specific 
  in the search terms you feed google to even get to this developer's page.
  
  https://borokobo.web.fc2.com/-_x_tr_sl=auto&_x_tr_tl=en&_x_tr_hl=es&_x_tr_pto=wapp

---------------------------------

Reflex Viewer version 0.01

- Distributor's HP
	http://sldnoonmoon.hp.infoseek.co.jp/
	-iNeo Rags Studio)
	http://s2.muryo-de.etowns.net/~boro_kobo/
	(Neo Rags & Scraps Studio Annex)

- Disclaimer
  This software is freeware.
  You can use it freely,
  I am not responsible for any damages to your computer or operator system caused by this software.

- Outline
  This is a room viewer (for Kirby and the Amazing Mirror) as described on the distributor's page.

- Installation and Uninstallation
  To install, place the software in an appropriate folder.
  To uninstall, delete the folder.

- Instructions
  In advance, you need to put the ROM file of "Kirby and the Amazing Mirror" in the folder of this software and name it: "kagami.gba".
  Then, when you start the software, it will load the room image data and display the map.

- Usage
  F5/F6 Switch map
  A/S Move the map in X axis.
  W/Z Move the map in the Y axis.
  Press shift to move faster while navigating the maps using these keys.

  Tab: Toggle collision maps ON/OFF
  Ctrl+Tab: Toggle destroyable blocks ON/OFF

- About map numbers
  "Kirby and the Amazing Mirror" has a map indexing system difficult to understand.
  
  We will use 2 concepts to understand how ReflexViewer reads map data: Short Room (SRoom) and Long Room (LRoom).

  Maps are serialized with indexes in memory, these indexes are not organized based on its position in memory,
  if we were to reference these rooms starting from 0x00 incrementally, it would lead to "invalid" room reading
  because this index this is not the real ID of the room in memory.
  
  These incrementally counted rooms are denominated Short Rooms or "SRoom" in ReflexViewer.

  Apart from that, for map data stored in memory, we use its ROM room ID in memory,
  I decided to call these "Long Room" (LRoom) data.

  In the game, when linking doors, you have to use the Long Room number to link these mirrors together.
  When a mirror specifies a Long Room value, a certain location in the ROM is read and converted to a short room index,
  and then the game accesses the stored data.
  You don't need the Short Room numbers for any modification, the game does this at runtime.

  In this editor,
  The Long and Short room numbers are displayed in the title bar.
  
  Short Room #004 is an unused map left in the game.
  This room doesn't display in ReflexViewer but it does in the ROM as long as you have a Long Room ID to call this room.

- What you can see
	- Maps
		You can see the map displayed properly with its corresponding tileset.
		Background data is not loaded in, but that's intentional.
		You can change the background color in the Source Code, but it looks more natural this way.
		You can also press Tab to see the collision data.
		Ctrl+Tab will toggle between displaying destroyable objects in the map.
		(It is easier to visualize if you do these commands in a room with many destroyable blocks, etc.)
	
	- Enemy
		The position of an enemy is displayed as a yellow square.
		The value inside the square is the enemy "type".
		In this game, one enemy has an unbelievably large number of parameters.
		It would be difficult to detail them all in ReflexViewer, so I have omitted most of them.
		So, even though you may see repeated enemy types, these may behave in different ways based on its properties/behavioral data.
	
	- Door
		The position of a mirror is indicated by the blue square above it.
		The blue square holds the long room value of the mirror.
		(Sometimes it's a bit hard to see because it is a 3-digit value.)
		The coordinates of where should Kirby be placed in a room after entering a mirror are shown in the (x,y) parentheses,
		The value on the right side of the (x,y) parentheses indicate Kirby's facing direction when the room is loaded.

- Source
	The source of this software is located in the "src" folder.
	The coding is quite poor, but please forgive me.
	You are free to modify the source code, and there is no problem if you publish the modified version,
	You are free to modify the source code, and you are free to publish your modifications, but please solve any problems by yourself.
	However, if you use any part of this source, please release the source of the software.