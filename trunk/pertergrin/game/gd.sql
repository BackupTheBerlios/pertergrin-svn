--- ------------------------------------------------------------------
--- PERTERGRIN MAIN SQL DATABASE Source Code
--- ------------------------------------------------------------------
---
--- Any Changes made here will not be automatically updated in the
--- Postgresql ptg database. You need to contact one of the database
--- administrators first!!
--- ------------------------------------------------------------------
---
--- Administrators as of 18.04.2001:
--- Reinhard Katzmann <reinhard@suamor.de>
---
--- ------------------------------------------------------------------


--- ------------------------------------------------------------------
--- Data Type / Data Function Definitions inside the database
--- ------------------------------------------------------------------

--- Warning: Be sure to have set the correct path in all the places!!

--- LOAD '/usr/local/games/pertergrin/modules/ptgfunc.so'


---
--- datatype / function definition of ????
---


--- function for setting something

--- DROP FUNCTION ptg_in(opaque);
--- CREATE FUNCTION ptg_in(opaque)
---        returns opaque
---        as '/usr/local/games/pertergrin/modules/ptgfunc.so'
--- language 'c';

--- function for getting something

--- DROP FUNCTION ptg_out(opaque);
--- CREATE FUNCTION ptg_out(opaque)
---        returns opaque
---        as '/usr/local/games/pertergrin/modules/ptgfunc.so'
--- language 'c';

--- type definition of something

--- DROP TYPE batbin;
--- CREATE TYPE batbin (
---        internallength = -1,
---        input = ptg_in,
---        output = ptg_out
--- );


--- ------------------------------------------------------------------
--- SQL Database definition
--- ------------------------------------------------------------------

---- Object type attributes - Last Update 16.05.2001
--
-- types: 0 = boolean
--        1 = integer
--        2 = double
--        3 = text

DROP TABLE attributes;
DROP SEQUENCE attributes_attributesid_seq;
CREATE TABLE attributes (
	attributesid	serial PRIMARY KEY,
	name		TEXT NOT NULL,
	description	TEXT NOT NULL,
	type		INT2 NOT NULL,
	value		TEXT NOT NULL,
	format		TEXT NOT NULL
):


---- Object type complex / composed attribute - Last Update 18.04.2001

DROP TABLE complexattr;
DROP SEQUENCE complexattr_complexattrid_seq;
CREATE TABLE complexattr (
	complexattrid	serial PRIMARY KEY,
	name		TEXT NOT NULL,
	description	TEXT NOT NULL,
	base		BOOLEAN NOT NULL,
	attributes	INT4[] NOT NULL,
	handler		INT2,
	FOREIGN KEY (attributes) REFERENCES attributes(attributesid)
);

---- Object type race - Last Update 16.05.2001

DROP TABLE race;
DROP SEQUENCE race_raceid_seq;
CREATE TABLE race (
	raceid		serial PRIMARY KEY,
	name		TEXT NOT NULL,
	description	TEXT NOT NULL,
	attributes	INT4[] NOT NULL,
	complexattr	INT4[],
	FOREIGN KEY (attributes) REFERENCES attributes(attributesid),
	FOREIGN KEY (complexattr) REFERENCES complexattr(complexattrid)
);

---- Object type herotype - Last Update 16.05.2001

DROP TABLE herotype;
DROP SEQUENCE herotype_herotypeid_seq;
CREATE TABLE herotype (
	herotypeid	serial PRIMARY KEY,
	name		TEXT NOT NULL,
	description	TEXT NOT NULL,
	attributes	INT4[] NOT NULL,
	complexattr	INT4[],
);
	
---- Object type pc (personal character, hero) - Last Update 16.05.2001

DROP TABLE pc;
DROP SEQUENCE pc_pcid_seq;
CREATE TABLE pc (
	pcid		serial PRIMARY KEY,
	name		TEXT NOT NULL,
	description	TEXT NOT NULL,
	complexattr	INT4[],
	attributes	INT4[] NOT NULL,
	race            INT2 NOT NULL,
	herotype        INT2 NOT NULL,
	handler		INT2,
	FOREIGN KEY (attributes) REFERENCES attributes(attributesid),
	FOREIGN KEY (complexattr) REFERENCES complexattr(complexattrid),
	FOREIGN KEY (race) REFERENCES race(raceid)
	FOREIGN KEY (herotype) REFERENCES herotype(herotypeid)
);

---- Object type npc (non-personal character) - Last Update 18.04.2001

DROP TABLE npc;
DROP SEQUENCE npc_npcid_seq;
CREATE TABLE npc (
	npcid		serial PRIMARY KEY,
	name		TEXT NOT NULL,
	description	TEXT NOT NULL,
	base		BOOLEAN NOT NULL,
	complexattr	INT4[],
	attributes	INT4[] NOT NULL,
	handler		INT2,
	FOREIGN KEY (attributes) REFERENCES attributes(attributesid),
	FOREIGN KEY (complexattr) REFERENCES complexattr(complexattrid)
);

---- Object type simpleobject - Last Update 16.05.2001

DROP TABLE simpleobject;
DROP SEQUENCE simpleobject_simpleobjectid_seq;
CREATE TABLE simpleobject (
	simpleobjectid	serial PRIMARY KEY,
	name		TEXT NOT NULL,
	description	TEXT NOT NULL,
	attributes	INT4[] NOT NULL,
	FOREIGN KEY (attributes) REFERENCES attributes(attributesid),
);

---- Object type item - Last Update 04.10.2002

DROP TABLE item;
DROP SEQUENCE item_itemid_seq;
CREATE TABLE item (
	itemid		serial PRIMARY KEY,
	name		TEXT NOT NULL,
	description	TEXT NOT NULL,
	type		INT2 NOT NULL,
	attributes	INT4[] NOT NULL,
	complexattr	INT4[],
	item		INT4[],
	FOREIGN KEY (item) REFERENCES item(itemid),
	FOREIGN KEY (attributes) REFERENCES attributes(attributesid),
	FOREIGN KEY (complexattr) REFERENCES complexattr(complexattrid)
);

---- Object type town - Last Update 04.10.2002

DROP TABLE town;
DROP SEQUENCE town_townid_seq;
CREATE TABLE town (
	townid		serial PRIMARY KEY,
	name		TEXT NOT NULL,
	description	TEXT NOT NULL,
	attributes	INT4[] NOT NULL,
	complexattr	INT4[],
	dungeon		INT4[],
	npc		INT4[],
	item		INT4[],
	simpleobject	INT4[],
	FOREIGN KEY (dungeon) REFERENCES dungeon(dungeonid),
	FOREIGN KEY (simpleobject) REFERENCES simpleobject(simpleobjectid),
	FOREIGN KEY (npc) REFERENCES npc(npcid),
	FOREIGN KEY (attributes) REFERENCES attributes(attributesid),
	FOREIGN KEY (complexattr) REFERENCES complexattr(complexattrid)
);

---- Object type dungeon - Last Update 04.10.2002

DROP TABLE dungeon;
DROP SEQUENCE dungeon_dungeonid_seq;
CREATE TABLE dungeon (
	dungeonid	serial PRIMARY KEY,
	name		TEXT NOT NULL,
	description	TEXT NOT NULL,
	attributes	INT4[] NOT NULL,
	complexattr	INT4[],
	town		INT4[],
	npc		INT4[],
	simpleobject	INT4[],
	item		INT4[],
	FOREIGN KEY (town) REFERENCES town(townid),
	FOREIGN KEY (simpleobject) REFERENCES simpleobject(simpleobjectid),
	FOREIGN KEY (npc) REFERENCES npc(npcid),
	FOREIGN KEY (attributes) REFERENCES attributes(attributesid),
	FOREIGN KEY (complexattr) REFERENCES complexattr(complexattrid)
);

---- Object type world - Last Update 04.10.2002

DROP TABLE world;
DROP SEQUENCE world_worldid_seq;
CREATE TABLE world (
	worldid		serial PRIMARY KEY,
	name		TEXT NOT NULL,
	description	TEXT NOT NULL,
	attributes	INT4[] NOT NULL,
	complexattr	INT4[],
	dungeon		INT4[],
	town		INT4[],
	npc		INT4[],
	pc		INT4 NOT NULL,
	FOREIGN KEY (dungeon) REFERENCES dungeon(dungeonid),
	FOREIGN KEY (town) REFERENCES town(townid),
	FOREIGN KEY (pc) REFERENCES pc(pcid),
	FOREIGN KEY (npc) REFERENCES npc(npcid),
	FOREIGN KEY (attributes) REFERENCES attributes(attributesid),
	FOREIGN KEY (complexattr) REFERENCES complexattr(complexattrid)
);

---- Object type universe - Last Update 04.10.2002

DROP TABLE universe;
DROP SEQUENCE universe_universeid_seq;
CREATE TABLE universe (
	universeid	serial PRIMARY KEY,
	name		TEXT NOT NULL,
	description	TEXT NOT NULL,
	attributes	INT4[],
	world		INT4[] NOT NULL,
	FOREIGN KEY (world) REFERENCES attributes(worldid),
	FOREIGN KEY (attributes) REFERENCES attributes(attributesid),
);

--- Foreign Keys which cannot be inserted (drop table will remove
--- them at once) are now inserted with ALTER TABLE
--- Last Update: 18.04.2001

--- ALTER TABLE ???
---	  ADD FOREIGN KEY (xyz) REFERENCES xyz(xyzid);
