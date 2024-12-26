Reflex Viewer version 0.01

? Distributor's HP
	http://sldnoonmoon.hp.infoseek.co.jp/
	（Neo Rags Studio)
	http://s2.muryo-de.etowns.net/~boro_kobo/
	(Neo Rags & Scraps Studio Annex)
	https://borokobo.web.fc2.com/?_x_tr_sl=auto&_x_tr_tl=en&_x_tr_hl=es&_x_tr_pto=wapp

? Disclaimer
　This software is freeware.
　You can use it freely,
　(e.g., any damage or injury to the computer or operator caused by its use).
　(For example, any damage or injury to the computer or operator caused by the use of this software)
　I am not responsible for any damage or injury to the computer or operator caused by the existence of this software.

? Outline
　This is a viewer (for Kirby and the Amazing Mirror) as described on the distributor's page.

? Installation and Uninstallation
　To install, place the software in an appropriate folder.
　To uninstall, delete the folder.

? Instructions
　In advance, you need to put the ROM file of “Kirby and the Amazing Mirror” in the folder of this software.
  Place this ROM file in the root folder of this program and name it "kagami.gba".
　Then, when you start the software, it will load the image and display the map.

? Usage
  F5/F6 Switch map
  A/S Move the map in X axis.
  W/Z Move the map in the Y axis.
  Press shift to move faster while navigating the maps using these keys.

  Tab: Toggle collision maps ON/OFF
  Ctrl+Tab: Toggle destroyable blocks ON/OFF

  It is difficult to understand, but there are four display modes (2 x 2).

? About map numbers
  This game has a map numbering system that is difficult to understand.
  
  We will use 2 concepts to understand how ReflexViewer reads map data: Short Room (SRoom) and Long Room (LRoom).

  Maps are serialized with indexes in memory, these indexes are not organized based on its position in memory,
  if we were to reference these rooms starting from 0x00 incrementally, it would lead to "invalid" room reading
  because this index this is not the real ID of the room in memory.
  
  These incrementally counted rooms are denominated Short Rooms or "SRoom" in ReflexViewer.

  Apart from that, for map data stored in memory, we use its ROM room ID in memory,
  I decided to call these "Long Room" (LRoom).

  In the game when linking doors you have to use the Long Room number to link these mirrors together.
  When a mirror specifies a Long Room, a certain location in the ROM is read and converted to a short room,
  and then accesses the stored data.
  You don't need the Short Room numbers for any modification the game does this at runtime.

  In this editor,
  The Long and Short room numbers are displayed in the title bar.
  Short Room #004 is an unused map left in the game.
  This room doesn't display in ReflexViewer but it does in the ROM as long as you have a Long Room ID to call this room.



? What you can see
	? Maps
		You can see the map displayed properly with its corresponding tileset.
		Background data is not loaded in but that's intentional.
		You can change the background color in the Source Code but it looks more natural this way.
		You can also press Tab to see the collision data.
		Ctrl+Tab will toggle between displaying destroyable objects in the map.
		(It is easier to understand if you do it in a place with many destroyable blocks, etc.)
	
	? Enemy
		The position of the enemy is displayed as a yellow square.
		The value inside the square is the enemy "type".
		In this game, one enemy has an unbelievably large number of parameters.
		It would be difficult to see them all, so we have omitted most of them.
		So, even though they are the same enemy type these may behave in different 
		ways based on its properties/behavioral data.
	
	? Door
		The position of the door is indicated by a blue square.
		It is linked to the “long room” of the value written on the square itself.
		(It is a little hard to see because it is a 3-digit value.)
		The coordinates of the appearance are shown in (,),
		The value on the right side indicates the direction of appearance.

? Source
	The source of this software is enclosed in the src folder.
	The coding is quite poor, but please forgive me.
	You are free to modify the source code, and there is no problem if you publish the modified version,
	You are free to modify the source code, and you are free to publish your modifications, but please solve any problems by yourself.
	However, if you use any part of this source, please release the source of the software.