#ifndef ENUMC_H
#define ENUMC_H

enum COMPONENT_TYPE
{
	TRANSFORMATION_CTYPE = 0,
	BASICMESH_CTYPE,
	FONT_CTYPE,
	CAMERA_CTYPE,
	DIRLIGHT_CTYPE,
	POINTLIGHT_CTYPE,
	SCRIPT_TYPE,
	LAST_CTYPE
};

// which components are allowed to add to entity
enum COMPONENT_ATYPE
{
	BASICMESH_ACTYPE = 0,
	FONT_ACTYPE,
	CAMERA_ACTYPE,
	DIRLIGHT_ACTYPE,
	POINTLIGHT_ACTYPE,
	SCRIPT_ACTYPE,
	LAST_ACTYPE
};

enum eMODELS
{
	QUAD_MODEL = 0,
	FONT_MODEL,
	BUNNY_MODEL,
	SUZANNA_MODEL,
	CUBE_MODEL,
	SKYBOX_MODEL,
	DRAGON_MODEL,
	SPHERE_MODEL,
	MAX_MODELS
};

enum eTEXTURES
{
	BASIC_2D_RGBA8 = 0,
	BLACK_2D_RGBA,
	EVAFONT_2D_BC3,
	YOKOHOMO_CUBEMAP_RGBA8,

	COLOR0_ATTACHMENT_RGBA32U,
	COLOR1_ATTACHMENT_RGBA32U,
	HDRTEX_ATTACHMENT_RGBA16F, // will be stored
	DEPTHSTENCIL_ATTACHMENT_32F,

	HPP_ATTACHMENT_RGBA16F,

	MAXTEX_NAME_TYPE_FORMAT
};

enum MESH_TEX
{
	BLACK_TEX = 0,
	BOX_TEX,
	MAX_MESH_TEX
};

enum eFONTS
{
	FONT_EVA = 0,
	MAX_FONT
};

enum LUNA_KEY
{
	LN_LEFT = 0x25,
	LN_UP = 0x26,
	LN_RIGHT = 0x27,
	LN_DOWN = 0x28,
	MAX_LN_KEY = 256
};

#endif
