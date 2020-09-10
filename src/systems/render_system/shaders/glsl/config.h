#ifndef GLSL_CONFIG_H
#define GLSL_CONFIG_H

/**
 * This was really stupid idea, should have used --auto-map-bindings and  --auto-map-bindings
 * --auto-map-locations options glslangvalidator to auto generate location and bindings config.
 * 
 * TODO: Do this properly
 */

// configuration file for vertex shaders
#ifdef FORWARD_VERTEX_SHADER
#define VERTEX_SHADER
// uniform locations
#define VERT_U_TRANSFORMATION_LOC 0
#endif

/**
 * Common - 0 start
 *
 * 0 - texCoord, 1 - worldPos, 3 - normal, 4 - camPos
 */
#if defined(FORWARD_VERTEX_SHADER) || defined(FORWARD_FRAGMENT_SHADER)
#define VERT_INTERFACE_BLOCK_LOC 0
#endif

#ifdef FORWARD_FRAGMENT_SHADER
#define FRAGMENT_SHADER
#define FRAG_U_MATERIAL_ALBEDO_LOC 10
#define FRAG_U_MATERIAL_METALLIC_LOC 11
#define FRAG_U_MATERIAL_ROUGHNESS_LOC 12
#define FRAG_U_MATERIAL_AO_LOC 13

#define MAX_POINT_LIGHTS 4
#define FRAG_U_POINT_LIGHT_SIZE 14
#define FRAG_U_POINT_LIGHT0_POS 15
#define FRAG_U_POINT_LIGHT0_COL 16
#define FRAG_U_POINT_LIGHT0_RAD 17
#define FRAG_U_POINT_LIGHT0_INT 18
#define FRAG_U_POINT_LIGHT1_POS 19
#define FRAG_U_POINT_LIGHT1_COL 20
#define FRAG_U_POINT_LIGHT1_RAD 21
#define FRAG_U_POINT_LIGHT1_INT 22
#define FRAG_U_POINT_LIGHT2_POS 23
#define FRAG_U_POINT_LIGHT2_COL 24
#define FRAG_U_POINT_LIGHT2_RAD 25
#define FRAG_U_POINT_LIGHT2_INT 26
#define FRAG_U_POINT_LIGHT3_POS 27
#define FRAG_U_POINT_LIGHT3_COL 28
#define FRAG_U_POINT_LIGHT3_RAD 29
#define FRAG_U_POINT_LIGHT3_INT 30

#define FRAG_U_IRRADIANCE_MAP_BND 1
#define FRAG_U_PREFILTERED_MAP_BND 2
#define FRAG_U_BRDF_INTEGRATION_MAP_BND 3
/* these are used by texture material shader */
#define FRAG_U_MATERIAL_ALBEDO_BND 4
#define FRAG_U_MATERIAL_METALLIC_ROUGHNESS_BND 5
#define FRAG_U_MATERIAL_AO_BND 6
#define FRAG_U_MATERIAL_NORMAL_BND 7
#define FRAG_U_MATERIAL_EMISSION_BND 8
#endif

#ifdef IBL_SPECULAR_CONVOLUTION_FRAGMENT_SHADER
#define CUBEMAP_FRAGMENT_SHADER
#define FRAG_U_ROUGHNESS_LOC 2
#endif

#ifdef CUBEMAP_VERTEX_SHADER
#define VERTEX_SHADER
#define VERT_U_PROJECTION_MAT_LOC 0
#define VERT_U_VIEW_MAT_LOC 1
#endif

#ifdef CUBEMAP_FRAGMENT_SHADER
#define FRAGMENT_SHADER
#define FRAG_U_ENVMAP_BND 0
#endif

#if defined(CUBEMAP_VERTEX_SHADER) || defined(CUBEMAP_FRAGMENT_SHADER)
#define VERT_V_DIRECTION_LOC 1
#endif

#ifdef SKYBOX_VERTEX_SHADER
#define VERTEX_SHADER
#define VERT_V_DIRECTION_LOC 1 // same as of cubemap
#endif

#ifdef SKYBOX_FRAGMENT_SHADER
#define FRAGMENT_SHADER
#define FRAG_U_ENVMAP_BND 0
#endif

#ifdef VISUAL_PREP_VERTEX_SHADER
#define VERTEX_SHADER
#endif

#ifdef VISUAL_PREP_FRAGMENT_SHADER
#define FRAGMENT_SHADER
#define FRAG_U_EXPOSURE_LOC 0
#define FRAG_U_GAMMA_LOC 1
#define FRAG_U_FRAME_TEXTURE_BND 5
#endif

#if defined(VISUAL_PREP_VERTEX_SHADER) || defined(VISUAL_PREP_FRAGMENT_SHADER)
#define VERT_V_TEX_COORDS_LOC 10
#endif

#if defined GUI_VERTEX_SHADER
#define VERT_A_POSITION_LOC 0
#define VERT_A_TEXCOORD0_LOC 1
#define VERT_A_COLOR_LOC 2
#define VERT_U_PROJECTION_MAT_LOC 0
#endif

#if defined GUI_FRAGMENT_SHADE
#define FRAGMENT_SHADER
#define FRAG_U_TEXTURE_BND 0
#endif

#if defined(GUI_VERTEX_SHADER) || defined(GUI_FRAGMENT_SHADER)
#define VERT_INTERFACE_BLOCK_LOC 0
#endif

#ifdef VERTEX_SHADER
// attribute locations
#define VERT_A_POSITION_LOC 0
#define VERT_A_NORMAL_LOC 1
#define VERT_A_TEXTCOORD0_LOC 2
// uniform block
#define VERT_UB_GENERAL_LOC 0
#endif

#ifdef FRAGMENT_SHADER
#define COLOR_ATTACHMENT0 0
#endif

#endif
