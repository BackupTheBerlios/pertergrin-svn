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

---- Object type pc (personal character) - Last Update 18.04.2001

DROP TABLE pc;
DROP SEQUENCE pc_pcid_seq;
CREATE TABLE pc (
	pcid		serial PRIMARY KEY,
	name		TEXT NOT NULL,
	description	TEXT NOT NULL,
	complexattr	INT2[] NOT NULL,
	attributes	INT2[] NOT NULL,
	handler		INT2,
	FOREIGN KEY (attributes) REFERENCES attributes(attributesid)
	FOREIGN KEY (complexattr) REFERENCES complexattr(complexattrid)
);


---- Object type npc (non-personal character) - Last Update 18.04.2001

DROP TABLE npc;
DROP SEQUENCE npc_npcid_seq;
CREATE TABLE npc (
	npcid		serial PRIMARY KEY,
	name		TEXT NOT NULL,
	description	TEXT NOT NULL,
	complexattr	INT2[] NOT NULL,
	attributes	INT2[] NOT NULL,
	handler		INT2,
	FOREIGN KEY (attributes) REFERENCES attributes(attributesid)
	FOREIGN KEY (complexattr) REFERENCES complexattr(complexattrid)
);


---- Object type attributes - Last Update 18.04.2001

DROP TABLE attributes;
DROP SEQUENCE attributes_attributesid_seq;
CREATE TABLE attributes (
	attributesid	serial PRIMARY KEY,
	name		TEXT NOT NULL,
	description	TEXT NOT NULL,
	valueint	INT4 NOT NULL,
	valuechar	TEXT NOT NULL,
	valuefloat	FLOAT4 NOT NULL,
	format		TEXT NOT NULL
):


---- Object type complex / composed attribute - Last Update 18.04.2001

DROP TABLE complexattr;
DROP SEQUENCE complexattr_complexattrid_seq;
CREATE TABLE complexattr (
	complexattrid	serial PRIMARY KEY,
	name		TEXT NOT NULL,
	description	TEXT NOT NULL,
	attributes	INT2[] NOT NULL,
	handler		INT2,
	FOREIGN KEY (attributes) REFERENCES attributes(attributesid)
);

--- Foreign Keys which cannot be inserted (drop table will remove
--- them at once) are now inserted with ALTER TABLE
--- Last Update: 18.04.2001

--- ALTER TABLE ???
---	  ADD FOREIGN KEY (xyz) REFERENCES xyz(xyzid);
